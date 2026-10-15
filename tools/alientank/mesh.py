"""Small mesh builder for the specimen tank: lathed solids, boxes, tubes along paths, MLOD export.

Faces are kept outward counter-clockwise (right-hand cross product points out of the solid) with one normal
and one UV per corner; `lod()` turns them into an MLOD LOD the way Object Builder stores it (corner order
reversed so +cross points inward, normals pointing inward). UVs are in texture tiles: v grows down the image.
DayZ's model space is left-handed (x right, y up, z forward), so seen in the game everything is the mirror
image of the same numbers read right-handed: texture directions here are chosen so text reads correctly in game.
"""
import struct

import numpy as np


class Mesh:
    def __init__(self):
        self.P = []                 # points
        self.F = []                 # (point idx tuple, normal tuple of 3-vectors, uv tuple, material key)

    def point(self, p):
        self.P.append(np.asarray(p, float))
        return len(self.P) - 1

    def face(self, pts, normals, uvs, mat):
        """A tri or quad from explicit corners (outward CCW)."""
        idx = tuple(self.point(p) for p in pts)
        self.F.append((idx, tuple(np.asarray(n, float) for n in normals), tuple(uvs), mat))

    def poly(self, pts, normal, uvs, mat):
        """A flat tri or quad facing `normal`; corners are put in outward CCW order whatever order they come in."""
        pts = [np.asarray(p, float) for p in pts]
        n = np.asarray(normal, float)
        if np.cross(pts[1] - pts[0], pts[2] - pts[0]) @ n < 0:
            pts, uvs = pts[::-1], list(uvs)[::-1]
        self.face(pts, [n] * len(pts), uvs, mat)

    def extend(self, other, M=None, t=None):
        """Copy another mesh in, optionally rotated by M and moved by t."""
        M = np.eye(3) if M is None else np.asarray(M, float)
        t = np.zeros(3) if t is None else np.asarray(t, float)
        base = len(self.P)
        self.P += [M @ p + t for p in other.P]
        flip = np.linalg.det(M) < 0
        for idx, ns, uvs, mat in other.F:
            idx = tuple(i + base for i in idx)
            ns = tuple(M @ n for n in ns)
            if flip:
                idx, ns, uvs = idx[::-1], ns[::-1], uvs[::-1]
            self.F.append((idx, ns, uvs, mat))
        return self

    def faces_of(self, mat):
        return sum(1 for f in self.F if f[3] == mat)


def _unit(v):
    v = np.asarray(v, float)
    n = np.linalg.norm(v)
    return v / n if n > 1e-12 else v


def rot_y(a):
    c, s = np.cos(a), np.sin(a)
    return np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]])


def rot_x(a):
    c, s = np.cos(a), np.sin(a)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]])


def rot_z(a):
    c, s = np.cos(a), np.sin(a)
    return np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]])


def frame_to(axis):
    """Rotation taking +Y onto `axis`."""
    a = _unit(axis)
    y = np.array([0.0, 1.0, 0.0])
    v = np.cross(y, a)
    c = float(y @ a)
    if np.linalg.norm(v) < 1e-9:
        return np.eye(3) if c > 0 else np.diag([1.0, -1.0, -1.0])
    K = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    return np.eye(3) + K + K @ K / (1 + c)


# --------------------------------------------------------------------------------------------------------------
# primitives
# --------------------------------------------------------------------------------------------------------------

def _tiles(tile):
    return (float(tile), float(tile)) if np.isscalar(tile) else (float(tile[0]), float(tile[1]))


def lathe(profile, seg, mat, tile=1.0, a0=0.0, a1=2 * np.pi, smooth=(), v0=0.0, inward=False, planar_caps=True):
    """Solid of revolution about +Y.

    profile: [(r, y), ...] walked so the surface's outside is on the right of the walking direction in the
    (r, y) plane (bottom centre -> out -> up -> in -> top centre for a closed solid). Each profile segment is a
    band with its own normals (a hard edge between bands) unless its index is in `smooth`, where it shares
    averaged normals with its neighbours. UVs (tile = metres per texture repeat, one number or (u, v)): u runs
    along the circumference so text reads left to right from outside in game, v runs up the profile with the image's
    top higher up (flat bands use the plan view instead when planar_caps). inward=True builds the inside.
    """
    tu, tv = _tiles(tile)
    m = Mesh()
    profile = [np.asarray(p, float) for p in profile]
    angles = np.linspace(a0, a1, seg + 1)
    seg_n = []
    for p, q in zip(profile[:-1], profile[1:]):
        d = q - p
        seg_n.append(_unit([d[1], -d[0]]))          # (radial, up) outward normal of the segment
    vacc = v0
    for i, (p, q) in enumerate(zip(profile[:-1], profile[1:])):
        length = np.linalg.norm(q - p)
        if length < 1e-9:
            continue

        def band_normal(j, end):
            n = seg_n[j]
            if j in smooth:
                k = j - 1 if end == 0 else j + 1
                if 0 <= k < len(seg_n):
                    n = _unit(n + seg_n[k])
            return n
        n0, n1 = band_normal(i, 0), band_normal(i, 1)
        flat = abs(q[1] - p[1]) < 1e-9 and planar_caps
        for k in range(seg):
            A, B = angles[k], angles[k + 1]
            corners = []
            for (r, y, n, a, vv) in ((p[0], p[1], n0, A, vacc), (p[0], p[1], n0, B, vacc),
                                     (q[0], q[1], n1, B, vacc + length), (q[0], q[1], n1, A, vacc + length)):
                pos = np.array([r * np.cos(a), y, r * np.sin(a)])
                nor = np.array([n[0] * np.cos(a), n[1], n[0] * np.sin(a)])
                if flat:
                    uv = (pos[0] / tu, pos[2] / tv)
                else:
                    arc = r * a if r > 1e-9 else ((p[0] + q[0]) / 2) * (A + B) / 2
                    uv = (arc / tu, -vv / tv)
                corners.append((pos, nor, uv))
            # walking p->q with the outside on the right, angle increasing: this order is outward CCW
            order = [0, 3, 2, 1]
            cs = [corners[o] for o in order]
            if p[0] < 1e-9:
                cs = [cs[0], cs[1], cs[2]]          # p at the axis: drop the doubled corner
            elif q[0] < 1e-9:
                cs = [cs[0], cs[1], cs[3]]
            if inward:
                cs = [(c[0], -c[1], c[2]) for c in cs[::-1]]
            m.face([c[0] for c in cs], [c[1] for c in cs], [c[2] for c in cs], mat)
        vacc += length
    return m


def cylinder(r, y0, y1, seg, mat, tile=1.0, caps=True, a0=0.0, a1=2 * np.pi):
    prof = [(0, y0), (r, y0), (r, y1), (0, y1)] if caps else [(r, y0), (r, y1)]
    return lathe(prof, seg, mat, tile, a0, a1)


def box(size, mat, tile=1.0, centre=(0, 0, 0), M=None):
    """Axis box (size x, y, z) centred on `centre`, rotated by M about its centre; planar UVs per side."""
    sx, sy, sz = np.asarray(size, float) / 2
    tu, tv = _tiles(tile)
    m = Mesh()
    sides = [  # normal, u axis, v axis (v grows down the image, so it runs along -up)
        ((1, 0, 0), (0, 0, -1), (0, -1, 0)), ((-1, 0, 0), (0, 0, 1), (0, -1, 0)),
        ((0, 0, 1), (1, 0, 0), (0, -1, 0)), ((0, 0, -1), (-1, 0, 0), (0, -1, 0)),
        ((0, 1, 0), (1, 0, 0), (0, 0, 1)), ((0, -1, 0), (1, 0, 0), (0, 0, -1)),
    ]
    half = np.array([sx, sy, sz])
    for n, u, v in sides:
        n, u, v = map(np.array, (n, u, v))
        c = n * half
        hu = np.abs(u) @ half
        hv = np.abs(v) @ half
        pts = [c - u * hu + v * hv, c + u * hu + v * hv, c + u * hu - v * hv, c - u * hu - v * hv]
        # outward CCW seen from outside: u right, -v up
        uvs = [((p @ u) / tu, (p @ v) / tv) for p in pts]
        m.face(pts, [n] * 4, uvs, mat)
    return Mesh().extend(m, M, centre)


def tube(path, radius, sides, mat, tile=1.0, caps=False, twist=0.0):
    """A round tube swept along a polyline (points (N, 3)); parallel-transport frames, smooth radial normals."""
    path = np.asarray(path, float)
    T = np.array([_unit(path[min(i + 1, len(path) - 1)] - path[max(i - 1, 0)]) for i in range(len(path))])
    ref = np.array([0.0, 1.0, 0.0]) if abs(T[0][1]) < 0.9 else np.array([1.0, 0.0, 0.0])
    Nf = [_unit(np.cross(T[0], ref))]
    for i in range(1, len(path)):
        b = np.cross(T[i - 1], T[i])
        if np.linalg.norm(b) < 1e-9:
            Nf.append(Nf[-1])
            continue
        b = _unit(b)
        ang = np.arccos(np.clip(T[i - 1] @ T[i], -1, 1))
        K = np.array([[0, -b[2], b[1]], [b[2], 0, -b[0]], [-b[1], b[0], 0]])
        R = np.eye(3) + np.sin(ang) * K + (1 - np.cos(ang)) * K @ K
        Nf.append(_unit(R @ Nf[-1]))
    s = np.concatenate([[0], np.cumsum(np.linalg.norm(np.diff(path, axis=0), axis=1))])
    tu, tv = _tiles(tile)
    m = Mesh()
    circ = 2 * np.pi * radius
    for i in range(len(path) - 1):
        for k in range(sides):
            corners = []
            for (j, kk) in ((i, k), (i, k + 1), (i + 1, k + 1), (i + 1, k)):
                a = 2 * np.pi * kk / sides + twist
                B = np.cross(T[j], Nf[j])
                dirn = np.cos(a) * Nf[j] + np.sin(a) * B
                corners.append((path[j] + radius * dirn, dirn, (circ * kk / sides / tu, s[j] / tv)))
            cs = corners[::-1] if _orient(corners, T[i]) else corners
            m.face([c[0] for c in cs], [c[1] for c in cs], [c[2] for c in cs], mat)
    if caps:
        for j, sgn in ((0, -1), (len(path) - 1, 1)):
            B = np.cross(T[j], Nf[j])
            ring = [path[j] + radius * (np.cos(2 * np.pi * k / sides) * Nf[j] + np.sin(2 * np.pi * k / sides) * B)
                    for k in range(sides)]
            for k in range(sides):
                tri = [path[j], ring[k], ring[(k + 1) % sides]]
                n = T[j] * sgn
                if np.cross(tri[1] - tri[0], tri[2] - tri[0]) @ n < 0:
                    tri = tri[::-1]
                m.face(tri, [n] * 3, [(0, 0)] * 3, mat)
    return m


def _orient(corners, tangent):
    """True when the quad's cross product points inward (needs reversing)."""
    p = [c[0] for c in corners]
    cr = np.cross(p[1] - p[0], p[2] - p[0])
    return cr @ corners[0][1] < 0


def smooth_path(points, radius=0.08, steps=6):
    """Polyline with its corners rounded (quadratic Bezier between the corner's neighbours)."""
    P = [np.asarray(p, float) for p in points]
    out = [P[0]]
    for i in range(1, len(P) - 1):
        a, b, c = P[i - 1], P[i], P[i + 1]
        da, dc = _unit(a - b), _unit(c - b)
        r = min(radius, np.linalg.norm(a - b) / 2, np.linalg.norm(c - b) / 2)
        s, e = b + da * r, b + dc * r
        for t in np.linspace(0, 1, steps + 1):
            out.append((1 - t) ** 2 * s + 2 * (1 - t) * t * b + t ** 2 * e)
    out.append(P[-1])
    return np.array(out)


def bezier(p0, p1, p2, p3, n):
    t = np.linspace(0, 1, n)[:, None]
    return (1 - t) ** 3 * p0 + 3 * (1 - t) ** 2 * t * p1 + 3 * (1 - t) * t ** 2 * p2 + t ** 3 * p3


# --------------------------------------------------------------------------------------------------------------
# MLOD export
# --------------------------------------------------------------------------------------------------------------

def tagg_prop(name, value):
    return (1, "#Property#", name.encode().ljust(64, b"\0") + value.encode().ljust(64, b"\0"))


def lod(res, mesh, materials, props=None, sels=None, extra_faces=None):
    """MLOD LOD from a Mesh. materials: key -> (texture path, rvmat path). sels: name -> set of face indices
    (points of those faces are selected too). extra_faces: raw (point idx list MLOD order, tex, mat) appended
    as-is, for meshes copied from other MLODs (pts given via mesh.P)."""
    raw = np.array(mesh.P, float).reshape(-1, 3)
    # Faces carry their own corner points; weld equal positions so pieces are closed (Geometry components)
    # and the file stays small. Normals and UVs stay per corner.
    if len(raw):
        _, first, weld = np.unique(np.round(raw / 1e-5).astype(np.int64), axis=0, return_index=True,
                                   return_inverse=True)
        weld = np.asarray(weld).ravel()
        P = raw[first]
    else:
        weld, P = np.zeros(0, int), raw
    normals, nindex = [], {}

    def nid(n):
        n = _unit(-np.asarray(n, float))                  # MLOD stores normals pointing inward
        key = tuple(np.round(n, 4))
        if key not in nindex:
            nindex[key] = len(normals)
            normals.append(n)
        return nindex[key]
    faces, uvdata = [], bytearray(struct.pack("<I", 0))
    for idx, ns, uvs, mat in mesh.F:
        tex, rvmat = materials[mat]
        corners = list(zip(idx, ns, uvs))[::-1]           # MLOD order: +cross inward
        v = [(int(weld[i]), nid(n), float(u), float(w)) for i, n, (u, w) in corners]
        for _, _, u, w in v:
            uvdata += struct.pack("<ff", u, w)
        v += [(0, 0, 0.0, 0.0)] * (4 - len(v))
        faces.append(dict(n=len(corners), v=v, flags=0, tex=tex, mat=rvmat))
    taggs = []
    npts, nf = len(P), len(faces)
    for name, fset in (sels or {}).items():
        pb = np.zeros(npts, np.uint8)
        fb = np.zeros(nf, np.uint8)
        for fi in fset:
            fb[fi] = 1
            for i in mesh.F[fi][0]:
                pb[weld[i]] = 1
        taggs.append((1, name, bytes(pb) + bytes(fb)))
    for k, val in (props or {}).items():
        taggs.append(tagg_prop(k, val))
    if faces:
        taggs.append((1, "#UVSet#", bytes(uvdata)))
    taggs.append((1, "#EndOfFile#", b""))
    if not normals:
        normals = [np.zeros(3)]
    return dict(res=float(res), major=28, minor=256, flags=0, pts=P, pflags=np.zeros(npts, np.uint32),
                norms=np.array(normals, float), faces=faces, taggs=taggs)


def convex_lod(res, pieces, props=None, mass=None, materials=None):
    """Geometry-style LOD: closed convex pieces (each a Mesh) as Component01.. selections; optional mass per
    piece spread over its points; materials: per-piece rvmat (Fire Geometry)."""
    m = Mesh()
    sels, masses = {}, []
    for i, piece in enumerate(pieces):
        start_f, start_p = len(m.F), len(m.P)
        m.extend(piece)
        sels["Component%02d" % (i + 1)] = set(range(start_f, len(m.F)))
        if mass is not None:
            masses += [mass[i] / (len(m.P) - start_p)] * (len(m.P) - start_p)
    mats = {}
    for i in range(len(pieces)):
        mats["c%d" % i] = ("", materials[i] if materials else "")
    # pieces were built with material key "c<i>"
    L = lod(res, m, mats, props, sels)
    if mass is not None:
        L["taggs"].insert(-1, (1, "#Mass#", struct.pack("<%df" % len(masses), *masses)))
    return L


def prism(r, y0, y1, sides, key, centre=(0, 0)):
    """Closed convex n-gon prism (Geometry pieces)."""
    m = lathe([(0, y0), (r, y0), (r, y1), (0, y1)], sides, key)
    return Mesh().extend(m, None, (centre[0], 0, centre[1]))
