r"""Rebuild data/aliens/geb_GreenAlien.p3d as a vanilla-style infected (Blender, background).

    blender -b --factory-startup --python tools/alienrig/build_alien.py -- <alien T-pose.p3d> <donor.p3d> <out.p3d>

    alien T-pose.p3d  the alien as modelled (T-pose): git show c20764e:data/aliens/geb_GreenAlien.p3d
    donor.p3d         a debinarized vanilla infected, kept local: tools/debinned/_patient_skinny_m.p3d
                      (DeODOL6_2.exe P:\DZ\characters\zombies\patient_skinny_m.p3d <dir>; never commit it)

- LOD 1 = the alien's own mesh, UVs and texture, moved onto the infected bind pose (conform.py), so the
  infected animations bend it at the right joints.
- Bone weights come from the donor zombie: nearest point on the donor's surface (same or neighbouring body
  region), interpolated across that triangle, at most 4 bones, summing to 1. Finger bones are folded into the
  hand: the alien's three-fingered hands don't line up with the zombie's fingers, so they stay rigid.
- LODs 2-4 = Blender decimations of LOD 1, weights re-sampled from LOD 1.
- Fire/View Geometry = one convex piece per hit zone, built from the alien's own vertices, each rigid on its
  bone and named like vanilla's components (config DamageZones componentNames).
- Memory = the donor's skeleton points without proxies (aim/camera/footstep/axis points of the shared
  infected skeleton). Geometry = a plain 8-sided capsule (class=man). LandContact = one point.

Check the result with posetest.py and preview.py, then binarize a scratch copy from cwd P:\.
"""
import os
import struct
import sys

import bmesh
import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import conform  # noqa: E402
import mlod  # noqa: E402
import rig  # noqa: E402

TEX = r"alieninvasion\data\aliens\newgreenalien_ca.paa"
MAT = r"alieninvasion\data\aliens\greenalien.rvmat"
# Fire Geometry surface, as on vanilla infected: meatbones.bisurf gives the "body" hit sound and the Hit_MeatBones
# impact (blood, turned green for aliens by script). Without a material hits sounded like metal.
HIT_MAT = r"dz\data\data\penetration\meatbones.rvmat"
LOD_FACES = [None, 3000, 1400, 520]          # target triangles per visual LOD (None = full)
GEOMETRY_MASS = 70.83                         # vanilla infected Geometry LOD total

FIRE_ZONES = {
    "head": ["head", "neck1"],
    "neck": ["neck"],
    "spine3": ["spine3", "spine2", "leftshoulder", "rightshoulder"],
    "spine1": ["spine1", "spine", "pelvis", "lefthipextra", "righthipextra"],
}
for _s in ("left", "right"):
    FIRE_ZONES[_s + "arm"] = [_s + "arm", _s + "armroll", _s + "armextra", _s + "elbowextra"]
    FIRE_ZONES[_s + "forearm"] = [_s + "forearm", _s + "forearmroll", _s + "forearmextra", _s + "wristextra"]
    FIRE_ZONES[_s + "upleg"] = [_s + "upleg", _s + "uplegroll", _s + "kneeextra"]
    FIRE_ZONES[_s + "leg"] = [_s + "leg", _s + "legroll"]
    FIRE_ZONES[_s + "foot"] = [_s + "foot", _s + "toebase"]
ZONE_ORDER = ["rightfoot", "leftfoot", "head", "neck", "spine1", "rightforearm", "rightarm", "leftarm",
              "leftforearm", "rightupleg", "leftupleg", "leftleg", "rightleg", "spine3"]


# ---------------------------------------------------------------------------------------------------------
# MLOD assembly
# ---------------------------------------------------------------------------------------------------------

def tagg_prop(name, value):
    return (1, "#Property#", name.encode().ljust(64, b"\0") + value.encode().ljust(64, b"\0"))


def make_lod(res, pts, faces, sels=None, props=None, mass=None, normals=None):
    """faces: list of (idx list in MLOD order, uv list or None, tex, mat). sels: name -> (pt bytes, face bytes)."""
    pts = np.asarray(pts, float).reshape(-1, 3)
    n = len(pts)
    if normals is None:
        normals = np.zeros((n, 3))
    out_faces, uvdata = [], bytearray(struct.pack("<I", 0))
    for idx, uv, tex, mat in faces:
        uv = uv or [(0.0, 0.0)] * len(idx)
        v = [(int(i), int(i), float(u), float(w)) for i, (u, w) in zip(idx, uv)]
        v += [(0, 0, 0.0, 0.0)] * (4 - len(v))
        out_faces.append(dict(n=len(idx), v=v, flags=0, tex=tex, mat=mat))
        for u, w in uv:
            uvdata += struct.pack("<ff", u, w)
    taggs = []
    for name, (pb, fb) in (sels or {}).items():
        taggs.append((1, name, bytes(np.asarray(pb, np.uint8)) + bytes(np.asarray(fb, np.uint8))))
    if mass is not None:
        taggs.append((1, "#Mass#", struct.pack("<%df" % n, *mass)))
    for k, v in (props or {}).items():
        taggs.append(tagg_prop(k, v))
    taggs.append((1, "#UVSet#", bytes(uvdata)))
    taggs.append((1, "#EndOfFile#", b""))
    return dict(res=res, major=28, minor=256, flags=0, pts=pts, pflags=np.zeros(n, np.uint32),
                norms=np.asarray(normals, float), faces=out_faces, taggs=taggs)


def all_sel(npts, nfaces):
    return (np.ones(npts, np.uint8), np.ones(nfaces, np.uint8))


# ---------------------------------------------------------------------------------------------------------
# geometry helpers
# ---------------------------------------------------------------------------------------------------------

def triangles(faces):
    """MLOD-order corner index lists -> triangle list [(a, b, c)] and owning face per triangle."""
    tris, owner = [], []
    for fi, idx in enumerate(faces):
        for k in range(1, len(idx) - 1):
            tris.append((idx[0], idx[k], idx[k + 1]))
            owner.append(fi)
    return np.array(tris), np.array(owner)


def barycentric(p, a, b, c):
    v0, v1, v2 = b - a, c - a, p - a
    d00, d01, d11 = v0 @ v0, v0 @ v1, v1 @ v1
    d20, d21 = v2 @ v0, v2 @ v1
    den = d00 * d11 - d01 * d01
    if abs(den) < 1e-14:
        return np.array([1.0, 0.0, 0.0])
    v = (d11 * d20 - d01 * d21) / den
    w = (d00 * d21 - d01 * d20) / den
    bc = np.clip(np.array([1 - v - w, v, w]), 0, 1)
    return bc / bc.sum()


def sample_weights(Q, src_P, src_tris, src_W, groups=None, tri_group=None):
    """For each point in Q: nearest point on the source triangles (optionally only triangles whose group is
    allowed for that point), weights interpolated across the triangle. groups[i] = allowed tri groups."""
    out = np.zeros((src_W.shape[0], len(Q)))
    trees = {}

    def tree(key):
        if key not in trees:
            sel = np.arange(len(src_tris)) if key is None else np.nonzero(np.isin(tri_group, list(key)))[0]
            verts = [Vector(p) for p in src_P]
            trees[key] = (BVHTree.FromPolygons(verts, [tuple(map(int, src_tris[t])) for t in sel]), sel)
        return trees[key]

    for i, q in enumerate(Q):
        key = None if groups is None else tuple(sorted(groups[i]))
        bvh, sel = tree(key)
        loc, nrm, ti, dist = bvh.find_nearest(Vector(q))
        a, b, c = src_tris[sel[ti]]
        bc = barycentric(np.array(loc), src_P[a], src_P[b], src_P[c])
        out[:, i] = bc[0] * src_W[:, a] + bc[1] * src_W[:, b] + bc[2] * src_W[:, c]
    return out


def clean_weights(W, names, keep=4, floor=0.02):
    """Fold finger/toe-tip bones into the hand, keep the strongest `keep` bones per point, renormalise."""
    W = W.copy()
    idx = {n: i for i, n in enumerate(names)}
    for n in names:
        for side in ("left", "right"):
            if n.startswith(side + "hand") and n != side + "hand":
                W[idx[side + "hand"]] += W[idx[n]]
                W[idx[n]] = 0
    for i in range(W.shape[1]):
        col = W[:, i]
        order = np.argsort(-col)
        col[order[keep:]] = 0
        col[col < floor] = 0
        s = col.sum()
        W[:, i] = col / s if s > 0 else col
    return W


def vertex_normals(P, faces):
    """Area-weighted outward vertex normals for MLOD-order faces (+cross points inward in MLOD)."""
    N = np.zeros_like(P)
    for idx in faces:
        for k in range(1, len(idx) - 1):
            a, b, c = idx[0], idx[k], idx[k + 1]
            cr = np.cross(P[b] - P[a], P[c] - P[a])
            for v in (a, b, c):
                N[v] -= cr
    ln = np.linalg.norm(N, axis=1, keepdims=True)
    ln[ln == 0] = 1
    return N / ln


def fib_dirs(n=64):
    i = np.arange(n) + 0.5
    phi = np.arccos(1 - 2 * i / n)
    th = np.pi * (1 + 5 ** 0.5) * i
    return np.stack([np.cos(th) * np.sin(phi), np.cos(phi), np.sin(th) * np.sin(phi)], 1)


def convex_piece(points):
    """Convex hull of the support points of `points` in 64 directions. Returns (pts, tris in MLOD order)."""
    D = fib_dirs(64)
    sup = np.unique(np.argmax(points @ D.T, axis=0))
    bm = bmesh.new()
    for p in points[sup]:
        bm.verts.new(Vector(p))
    bm.verts.ensure_lookup_table()
    res = bmesh.ops.convex_hull(bm, input=bm.verts[:], use_existing_faces=False)
    for g in res["geom_interior"] + res["geom_unused"]:
        if isinstance(g, bmesh.types.BMVert) and g.is_valid:
            bm.verts.remove(g)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bm.normal_update()
    bm.verts.ensure_lookup_table()
    bm.verts.index_update()
    pts = np.array([v.co[:] for v in bm.verts])
    centre = pts.mean(0)
    tris = []
    for f in bm.faces:
        a, b, c = [v.index for v in f.verts]
        # MLOD: +cross must point inward
        if np.cross(pts[b] - pts[a], pts[c] - pts[a]) @ (pts[a] - centre) > 0:
            a, b, c = a, c, b
        tris.append([a, b, c])
    bm.free()
    return pts, tris


def capsule(radius=0.3, y0=0.0, y1=1.62, sides=8):
    """Plain 8-sided capsule-like convex body for the Geometry LOD."""
    pts = [(0.0, y0, 0.0)]
    rings = [y0 + 0.14, y1 - 0.14]
    for y in rings:
        for k in range(sides):
            a = 2 * np.pi * k / sides
            pts.append((radius * np.cos(a), y, radius * np.sin(a)))
    pts.append((0.0, y1, 0.0))
    pts = np.array(pts)
    top = len(pts) - 1
    faces = []
    for k in range(sides):
        a, b = 1 + k, 1 + (k + 1) % sides
        c, d = a + sides, b + sides
        faces += [[0, a, b], [a, c, d, b], [c, top, d]]
    centre = pts.mean(0)
    out = []
    for f in faces:
        cr = np.cross(pts[f[1]] - pts[f[0]], pts[f[2]] - pts[f[0]])
        out.append(f if cr @ (pts[f[0]] - centre) < 0 else f[::-1])
    return pts, out


# ---------------------------------------------------------------------------------------------------------
# decimation through Blender
# ---------------------------------------------------------------------------------------------------------

def decimate(P, faces, uvs, target_tris):
    me = bpy.data.meshes.new("lod")
    # Blender wants outward CCW: reverse MLOD order
    me.from_pydata([tuple(p) for p in P], [], [list(reversed(f)) for f in faces])
    uvl = me.uv_layers.new(name="uv")
    k = 0
    for poly, uv in zip(me.polygons, uvs):
        ruv = list(reversed(uv))
        for j, li in enumerate(poly.loop_indices):
            uvl.data[li].uv = (ruv[j][0], 1 - ruv[j][1])
        k += 1
    ob = bpy.data.objects.new("lod", me)
    bpy.context.scene.collection.objects.link(ob)
    ntri = sum(len(f) - 2 for f in faces)
    mod = ob.modifiers.new("dec", "DECIMATE")
    mod.ratio = target_tris / ntri
    mod.use_collapse_triangulate = True
    dg = bpy.context.evaluated_depsgraph_get()
    ev = ob.evaluated_get(dg)
    em = ev.to_mesh()
    P2 = np.array([v.co[:] for v in em.vertices])
    uvd = em.uv_layers.active.data
    F2, UV2 = [], []
    for poly in em.polygons:
        vs = [em.loops[li].vertex_index for li in poly.loop_indices]
        us = [(uvd[li].uv[0], 1 - uvd[li].uv[1]) for li in poly.loop_indices]
        F2.append(list(reversed(vs)))
        UV2.append(list(reversed(us)))
    ev.to_mesh_clear()
    bpy.data.objects.remove(ob)
    # collapsing thin parts can leave back-to-back face pairs on the same corners: binarize warns about them
    # and they enclose nothing, so drop every copy
    seen = {}
    for i, f in enumerate(F2):
        seen.setdefault(tuple(sorted(f)), []).append(i)
    drop = {i for g in seen.values() if len(g) > 1 for i in g}
    F2 = [f for i, f in enumerate(F2) if i not in drop]
    UV2 = [u for i, u in enumerate(UV2) if i not in drop]
    used = sorted({i for f in F2 for i in f})
    remap = {o: n for n, o in enumerate(used)}
    return P2[used], [[remap[i] for i in f] for f in F2], UV2


# ---------------------------------------------------------------------------------------------------------

def main():
    argv = sys.argv[sys.argv.index("--") + 1:]
    apath, dpath, out = argv[:3]
    bpy.ops.wm.read_factory_settings(use_empty=True)

    ma, La = rig.visual(apath)
    md, Ld = rig.visual(dpath)
    Q, alab, _ = conform.conform(La, Ld)
    a_faces = [[f["v"][k][0] for k in range(f["n"])] for f in La["faces"]]
    a_uvs = [[(f["v"][k][2], f["v"][k][3]) for k in range(f["n"])] for f in La["faces"]]

    Pd = Ld["pts"]
    fmask = rig.face_mask_without_proxies(Ld)
    d_faces = [[f["v"][k][0] for k in range(f["n"])] for fi, f in enumerate(Ld["faces"]) if fmask[fi]]
    Wd_map = rig.bone_weights(Ld)
    bone_names = sorted(n for n in Wd_map if n not in ("weapon", "launcher"))
    Wd = np.vstack([Wd_map[n] for n in bone_names])
    dlab = rig.regions_from_weights(Wd_map, len(Pd))
    d_tris, _ = triangles(d_faces)
    # triangle region = most common region of its corners
    tri_region = np.array([np.bincount(dlab[t].clip(0), minlength=len(rig.REGIONS)).argmax() for t in d_tris])

    # 1) weights for LOD 1 from the donor, region-restricted
    allowed = []
    for r in alab:
        name = rig.REGIONS[r]
        allowed.append({rig.REGIONS.index(x) for x in [name] + rig.ADJ[name]})
    W1 = sample_weights(Q, Pd, d_tris, Wd, allowed, tri_region)
    W1 = clean_weights(W1, bone_names)
    used_bones = [n for i, n in enumerate(bone_names) if W1[i].any()]
    print("LOD1 weights: %d bones used, max %d per point, sums %.3f..%.3f" % (
        len(used_bones), int((W1 > 0).sum(0).max()), W1.sum(0).min(), W1.sum(0).max()))

    # 2) visual LODs
    lods_geo = [(Q, a_faces, a_uvs, W1)]
    a_tris, _ = triangles(a_faces)
    for target in LOD_FACES[1:]:
        P2, F2, UV2 = decimate(Q, a_faces, a_uvs, target)
        W2 = clean_weights(sample_weights(P2, Q, a_tris, W1), bone_names)
        lods_geo.append((P2, F2, UV2, W2))
        print("LOD: %d points, %d faces" % (len(P2), len(F2)))

    model_lods = []
    for li, (P, F, UV, W) in enumerate(lods_geo):
        sels = {}
        for bi, n in enumerate(bone_names):
            if W[bi].any():
                sels[n] = (rig.enc(W[bi]), np.zeros(len(F), np.uint8))
        sels["camo"] = all_sel(len(P), len(F))
        sels["body_m"] = all_sel(len(P), len(F))
        normals = -vertex_normals(P, F)          # MLOD stores them pointing inward
        model_lods.append(make_lod(float(li + 1), P, [(f, uv, TEX, MAT) for f, uv in zip(F, UV)], sels,
                                   {"lodnoshadow": "1"}, normals=normals))

    # 3) Geometry: capsule, class=man
    cp, cf = capsule()
    model_lods.append(make_lod(1e13, cp, [(f, None, "", "") for f in cf], {"Component01": all_sel(len(cp), len(cf))},
                               {"class": "man"}, mass=[GEOMETRY_MASS / len(cp)] * len(cp),
                               normals=-vertex_normals(cp, cf)))

    # 4) Memory: donor skeleton points without proxies
    mem = [l for l in md["lods"] if mlod.lod_name(l["res"]) == "Memory"][0]
    msels = mlod.selections(mem)
    keep = np.ones(len(mem["pts"]), bool)
    for n, (pv, pf) in msels.items():
        if n.startswith("proxy:"):
            keep &= pv == 0
    kidx = np.nonzero(keep)[0]
    mem_sels = {n: (pv[kidx], np.zeros(0, np.uint8)) for n, (pv, pf) in msels.items()
                if not n.startswith("proxy:") and pv[kidx].any()}
    model_lods.append(make_lod(1e15, mem["pts"][kidx], [], mem_sels))
    print("Memory: %d points, %d selections" % (len(kidx), len(mem_sels)))

    # 5) LandContact
    model_lods.append(make_lod(2e15, [(0.0, 0.0, 0.0)], []))

    # 6) View + Fire Geometry: convex piece per hit zone, rigid on its bone
    dom = np.array(bone_names)[W1.argmax(0)]
    zone_of_bone = {b: z for z, bs in FIRE_ZONES.items() for b in bs}
    gpts, gfaces, gsel = [], [], {}
    for ci, zone in enumerate(ZONE_ORDER):
        members = np.array([zone_of_bone.get(b) == zone for b in dom])
        if members.sum() < 4:
            raise SystemExit("zone %s has no vertices" % zone)
        hp, ht = convex_piece(Q[members])
        base = len(gpts)
        gpts.extend(hp.tolist())
        f0 = len(gfaces)
        gfaces.extend([[base + i for i in t] for t in ht])
        rng = (base, len(gpts), f0, len(gfaces))
        for name in ("Component%02d" % (ci + 1), zone) + (("dmgzone_spine",) if zone == "neck" else ()):
            gsel.setdefault(name, []).append(rng)
        print("  %-12s %2d points %2d triangles from %d vertices" % (zone, len(hp), len(ht), members.sum()))
    gpts = np.array(gpts)
    sel_bytes = {}
    for name, ranges in gsel.items():
        pb = np.zeros(len(gpts), np.uint8)
        fb = np.zeros(len(gfaces), np.uint8)
        for a, b, c, d in ranges:
            pb[a:b] = 1
            fb[c:d] = 1
        sel_bytes[name] = (pb, fb)
    gn = -vertex_normals(gpts, gfaces)
    model_lods.append(make_lod(6e15, gpts, [(f, None, "", "") for f in gfaces], sel_bytes, normals=gn))
    model_lods.append(make_lod(7e15, gpts, [(f, None, "", HIT_MAT) for f in gfaces], sel_bytes, normals=gn))

    model = dict(version=257, lods=model_lods)
    mlod.write(model, out)
    back = mlod.read(out)
    assert mlod.write(back) == open(out, "rb").read()
    print("wrote", out, "with", len(model_lods), "LODs")


main()
