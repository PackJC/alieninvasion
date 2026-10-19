"""Distance LODs for the UFO wreck and the plasma rifle, decimated in Blender from each model's LOD 0.

    blender -b --factory-startup --python tools/build_lods.py -- [crash] [rifle] [--out DIR]

Neither model had real distance LODs. The wreck's six visual LODs were copies of LOD 0 (32,431 faces; LODs 1-5
even used mi8_body_crash.rvmat and no dirt material), and the rifle had only LOD 0 plus its 1100 first-person
LOD. DayZ draws a model's shadow from its last visual LOD and binarize switches shadows off when that LOD has
about 2,000 faces or more ("Too detailed shadow lod ... shadows disabled"), so neither cast a shadow.

The script reads the p3d in place and keeps LOD 0 and every LOD it doesn't generate byte for byte (Geometry,
Memory, Roadway, Fire Geometry, the rifle's 1100). It replaces the visual LODs between LOD 0 and 1000 with the
levels in PLAN, so rerunning it gives the same file. Each level is:
  - decimated from LOD 0 with Blender's collapse decimator, after welding the points LOD 0 split along UV seams;
  - given LOD 0's UVs, texture, material and named selections per face;
  - given normals taken from LOD 0's surface (nearest triangle of the same part, facing the same way).
Parts smaller than the level's `drop` size (bounding-box diagonal) are left out first: the wreck is 412 loose
pieces, mostly dirt clods, and the far levels can't spend their faces on them.

--out DIR writes <DIR>/<name>.p3d and leaves data/ alone. Check the result with tools/alienrig/preview.py and a
scratch binarize (grep its log for "Too detailed shadow lod").
"""
import os
import struct
import sys

import bpy
import numpy as np
from mathutils import Vector
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(HERE, "alienrig"))
import mlod  # noqa: E402

# (resolution, faces as a share of LOD 0 or a count, drop parts with a smaller bbox diagonal in metres)
PLAN = {
    "crash": dict(path="data/ufocrash/crash.p3d", lods=[
        (1.0, 0.50, 0.0),
        (2.0, 0.25, 0.06),
        (3.0, 0.12, 0.12),
        (4.0, 0.06, 0.25),
        (5.0, 1300, 0.40),       # the shadow level: well under binarize's ~2,000-face limit
    ]),
    "rifle": dict(path="data/weapons/geb_plasmarifle.p3d", lods=[
        (1.0, 0.50, 0.0),
        (2.0, 0.20, 0.0),
        (3.0, 1100, 0.0),        # the shadow level
    ]),
}
SMOOTH_COS = np.cos(np.radians(45))     # fallback normals average neighbours within 45 degrees
GUARD_COS = np.cos(np.radians(75))      # a transferred normal further off its new face than this is replaced


def lod_bytes(version, L):
    return mlod.write(dict(version=version, lods=[L]))[12:]


def unit(v):
    n = np.linalg.norm(v, axis=-1, keepdims=True)
    return v / np.where(n == 0, 1, n)


class Source:
    """LOD 0 welded on exact positions within each face group (texture, material, selections), split into parts."""

    def __init__(self, L):
        self.L = L
        F = L["faces"]
        assert all(f["n"] == 3 for f in F), "LOD 0 must be all triangles"
        self.sels = [(n, fb) for n, (pb, fb) in mlod.selections(L).items() if fb.any()]
        sigs = [(f["tex"], f["mat"], tuple(i for i, (n, fb) in enumerate(self.sels) if fb[fi]))
                for fi, f in enumerate(F)]
        self.groups = sorted(set(sigs))
        self.fgroup = np.array([self.groups.index(s) for s in sigs])
        raw = L["raw_pts"][:, :3]
        key, pos = {}, []
        corners = np.array([[f["v"][k][0] for k in range(3)] for f in F])
        wf = np.zeros_like(corners)
        for fi in range(len(F)):
            for k in range(3):
                pi = corners[fi, k]
                w = key.setdefault((self.fgroup[fi], raw[pi].tobytes()), len(key))
                if w == len(pos):
                    pos.append(L["pts"][pi])
                wf[fi, k] = w
        self.W = np.array(pos)
        ok = (wf[:, 0] != wf[:, 1]) & (wf[:, 1] != wf[:, 2]) & (wf[:, 0] != wf[:, 2])
        self.faces = wf
        self.ok = ok
        self.uvs = np.array([[(f["v"][k][2], f["v"][k][3]) for k in range(3)] for f in F])
        self.cnorm = np.array([[L["norms"][f["v"][k][1]] for k in range(3)] for f in F])
        # parts: connected through welded points
        parent = np.arange(len(self.W))

        def find(a):
            while parent[a] != a:
                parent[a] = parent[parent[a]]
                a = parent[a]
            return a
        for a, b, c in wf:
            for x in (b, c):
                ra, rb = find(a), find(x)
                if ra != rb:
                    parent[ra] = rb
        roots = np.array([find(a) for a in wf[:, 0]])
        _, self.part = np.unique(roots, return_inverse=True)
        self.nparts = self.part.max() + 1
        self.diag = np.zeros(self.nparts)
        for p in range(self.nparts):
            idx = np.unique(wf[self.part == p])
            self.diag[p] = np.linalg.norm(self.W[idx].max(0) - self.W[idx].min(0))
        self.bvh = {}
        size = np.linalg.norm(self.W.max(0) - self.W.min(0))
        self.tol = 1e-3 * size
        P0 = L["pts"]
        nf0 = unit(np.cross(P0[corners[:, 1]] - P0[corners[:, 0]], P0[corners[:, 2]] - P0[corners[:, 0]]))
        off = int((np.einsum("fkc,fc->fk", self.cnorm, nf0) < GUARD_COS).sum())
        print("  LOD 0: %d faces, %d welded points (from %d), %d groups, %d parts, %d corners off their face" % (
            len(F), len(self.W), len(L["pts"]), len(self.groups), self.nparts, off))

    def part_bvh(self, p):
        if p not in self.bvh:
            fis = np.nonzero((self.part == p) & self.ok)[0]
            idx, inv = np.unique(self.faces[fis], return_inverse=True)
            tris = inv.reshape(-1, 3)
            P = self.W[idx]
            tn = unit(np.cross(P[tris[:, 1]] - P[tris[:, 0]], P[tris[:, 2]] - P[tris[:, 0]]))
            bvh = BVHTree.FromPolygons([Vector(v) for v in P], [tuple(map(int, t)) for t in tris], all_triangles=True)
            self.bvh[p] = (bvh, fis, P, tris, tn)
        return self.bvh[p]

    def normal_at(self, p, q, nf=None):
        """LOD 0's interpolated (inward) normal at the point nearest q on part p. With nf, the triangle facing nf
        best among those about as near wins (the far side of a thin plate is as near as the near side)."""
        bvh, fis, P, tris, tn = self.part_bvh(p)
        loc, _, ti, d = bvh.find_nearest(Vector(q))
        if nf is not None:
            best = (tn[ti] @ nf, ti, loc)
            for loc2, _, ti2, d2 in bvh.find_nearest_range(Vector(q), d + self.tol):
                s = tn[ti2] @ nf
                if s > best[0]:
                    best = (s, ti2, loc2)
            _, ti, loc = best
        a, b, c = P[tris[ti]]
        bc = barycentric(np.array(loc), a, b, c)
        return unit(bc @ self.cnorm[fis[ti]])


def barycentric(p, a, b, c):
    v0, v1, v2 = b - a, c - a, p - a
    d00, d01, d11 = v0 @ v0, v0 @ v1, v1 @ v1
    d20, d21 = v2 @ v0, v2 @ v1
    den = d00 * d11 - d01 * d01
    if abs(den) < 1e-18:
        return np.array([1.0, 0.0, 0.0])
    v = (d11 * d20 - d01 * d21) / den
    w = (d00 * d21 - d01 * d20) / den
    bc = np.clip(np.array([1 - v - w, v, w]), 0, 1)
    return bc / bc.sum()


def decimate(src, keep, target):
    """Decimate the kept LOD 0 faces to about `target` triangles. Returns points, MLOD-order faces, UVs, parts."""
    fis = np.nonzero(keep)[0]
    used, inv = np.unique(src.faces[fis], return_inverse=True)
    F = inv.reshape(-1, 3)
    me = bpy.data.meshes.new("lod")
    me.from_pydata(src.W[used].tolist(), [], F[:, ::-1].tolist())       # Blender wants outward CCW
    assert len(me.polygons) == len(fis)
    uv = src.uvs[fis][:, ::-1].copy()
    uv[..., 1] = 1 - uv[..., 1]
    me.uv_layers.new(name="uv").data.foreach_set("uv", uv.ravel())
    me.attributes.new("part", "INT", "FACE").data.foreach_set("value", src.part[fis].astype(np.int32))
    ob = bpy.data.objects.new("lod", me)
    bpy.context.scene.collection.objects.link(ob)
    if target < len(fis):
        mod = ob.modifiers.new("dec", "DECIMATE")
        mod.ratio = target / len(fis)
        mod.use_collapse_triangulate = True
    em = ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh()
    nv, nf = len(em.vertices), len(em.polygons)
    P = np.zeros(nv * 3)
    em.vertices.foreach_get("co", P)
    tot = np.zeros(nf, np.int32)
    em.polygons.foreach_get("loop_total", tot)
    assert (tot == 3).all()
    lv = np.zeros(nf * 3, np.int32)
    em.loops.foreach_get("vertex_index", lv)
    uv = np.zeros(nf * 6, np.float32)
    em.uv_layers[0].data.foreach_get("uv", uv)
    part = np.zeros(nf, np.int32)
    em.attributes["part"].data.foreach_get("value", part)
    ob.evaluated_get(bpy.context.evaluated_depsgraph_get()).to_mesh_clear()
    bpy.data.objects.remove(ob)
    bpy.data.meshes.remove(me)
    P = P.reshape(-1, 3)
    F = lv.reshape(-1, 3)[:, ::-1]                                      # back to MLOD order
    uv = uv.reshape(-1, 3, 2)[:, ::-1].astype(np.float64)
    uv[..., 1] = 1 - uv[..., 1]
    # zero-area faces, and back-to-back pairs on the same corners (collapsed thin parts: they enclose nothing and
    # binarize warns about them), go
    area = np.linalg.norm(np.cross(P[F[:, 1]] - P[F[:, 0]], P[F[:, 2]] - P[F[:, 0]]), axis=1)
    good = area > 1e-12
    seen = {}
    for i, f in enumerate(F):
        seen.setdefault(tuple(sorted(f)), []).append(i)
    for g in seen.values():
        if len(g) > 1:
            good[g] = False
    F, uv, part = F[good], uv[good], part[good]
    used, inv = np.unique(F, return_inverse=True)
    return P[used], inv.reshape(-1, 3), uv, part


def normals(src, P, F, part):
    """Per-corner inward normals: LOD 0's at the nearest point (one per point unless a face looks the other way),
    or a 45-degree smooth normal where LOD 0's is way off the new face."""
    cr = np.cross(P[F[:, 1]] - P[F[:, 0]], P[F[:, 2]] - P[F[:, 0]])       # MLOD: +cross points inward
    nf = unit(cr)
    adj = [[] for _ in range(len(P))]
    for fi, f in enumerate(F):
        for v in f:
            adj[v].append(fi)
    vpart = np.zeros(len(P), int)
    vpart[F.ravel()] = np.repeat(part, 3)
    vn = np.array([src.normal_at(vpart[v], P[v]) for v in range(len(P))])
    out = np.zeros((len(F), 3, 3))
    replaced = 0
    for fi, f in enumerate(F):
        for k, v in enumerate(f):
            n = vn[v]
            if n @ nf[fi] < SMOOTH_COS:
                n = src.normal_at(part[fi], P[v], nf[fi])
            if n @ nf[fi] < GUARD_COS:
                g = np.array(adj[v])
                g = g[nf[g] @ nf[fi] > SMOOTH_COS]
                n = unit(cr[g].sum(0))
                replaced += 1
            out[fi, k] = n
    key = np.round(out.reshape(-1, 3), 6)
    uniq, inv = np.unique(key, axis=0, return_inverse=True)
    return unit(uniq), inv.reshape(-1, 3), replaced


def build_lod(src, res, P, F, uv, part, norms, nidx):
    L0 = src.L
    group = np.array([src.fgroup[np.nonzero(src.part == p)[0][0]] for p in range(src.nparts)])[part]
    faces = []
    for fi, f in enumerate(F):
        tex, mat, _ = src.groups[group[fi]]
        v = [(int(f[k]), int(nidx[fi, k]), float(uv[fi, k, 0]), float(uv[fi, k, 1])) for k in range(3)]
        faces.append(dict(n=3, v=v + [(0, 0, 0.0, 0.0)], flags=0, tex=tex, mat=mat))
    taggs, dropped = [], []
    sel_index = {n: i for i, (n, fb) in enumerate(src.sels)}
    for act, name, data in L0["taggs"]:
        if name in sel_index:
            i = sel_index[name]
            fb = np.array([i in src.groups[g][2] for g in group], np.uint8)
            pb = np.zeros(len(P), np.uint8)
            pb[np.unique(F[fb > 0])] = 1
            taggs.append((act, name, bytes(pb) + bytes(fb)))
        elif name == "#UVSet#" and struct.unpack_from("<I", data)[0] == 0:
            taggs.append((act, name, struct.pack("<I", 0) + uv.astype("<f4").tobytes()))
        elif name == "#EndOfFile#":
            taggs.append((act, name, data))
        else:
            dropped.append(name)
    L = dict(res=res, major=L0["major"], minor=L0["minor"], flags=L0["flags"], pts=P,
             pflags=np.zeros(len(P), np.uint32), norms=norms, faces=faces, taggs=taggs)
    return L, dropped


def build(name, out_dir):
    plan = PLAN[name]
    path = os.path.join(ROOT, plan["path"])
    print("==", name, path)
    m = mlod.read(path)
    L0 = m["lods"][0]
    kept = [L for L in m["lods"][1:] if not (L0["res"] < L["res"] < 1000)]
    src = Source(L0)
    n0 = len(L0["faces"])
    new = []
    for res, faces, drop in plan["lods"]:
        target = int(round(faces * n0)) if faces <= 1 else int(faces)
        keep = src.ok & (src.diag[src.part] >= drop)
        P, F, uv, part = decimate(src, keep, target)
        norms, nidx, replaced = normals(src, P, F, part)
        L, dropped = build_lod(src, res, P, F, uv, part, norms, nidx)
        new.append(L)
        counts = {}
        for f in L["faces"]:
            k = f["tex"].split("\\")[-1]
            counts[k] = counts.get(k, 0) + 1
        print("  LOD %g: %d faces (target %d), %d points, %d normals (%d corners off LOD 0's), parts kept %d/%d %s" % (
            res, len(F), target, len(P), len(norms), replaced, len(np.unique(part)), src.nparts, counts))
    if dropped:
        print("  not carried to the new levels:", sorted(set(dropped)))
    out = dict(version=m["version"], lods=[L0] + new + kept)
    dest = os.path.join(out_dir, name + ".p3d") if out_dir else path
    data = mlod.write(out, dest)
    check(m, data, len(new))
    print("  wrote", dest)


def check(orig, data, nnew):
    """Read back and compare: unchanged LODs byte-identical, LODs ordered by face count, UVs stored twice alike."""
    tmp = os.path.join(bpy.app.tempdir or os.environ.get("TEMP", "."), "build_lods_check.p3d")
    open(tmp, "wb").write(data)
    back = mlod.read(tmp)
    os.remove(tmp)
    assert mlod.write(back) == data
    v = orig["version"]
    ob = [lod_bytes(v, L) for L in orig["lods"]]
    nb = [lod_bytes(v, L) for L in back["lods"]]
    assert nb[0] == ob[0], "LOD 0 changed"
    L0 = orig["lods"][0]
    kept = [b for L, b in zip(orig["lods"][1:], ob[1:]) if not (L0["res"] < L["res"] < 1000)]
    assert nb[1 + nnew:] == kept, "a kept LOD changed"
    vis = [L for L in back["lods"] if L["res"] < 1000]
    counts = [len(L["faces"]) for L in vis]
    assert all(a > b for a, b in zip(counts, counts[1:])), counts
    texmats = {(f["tex"], f["mat"]) for f in L0["faces"]}
    for L in back["lods"][1:1 + nnew]:
        assert {(f["tex"], f["mat"]) for f in L["faces"]} <= texmats
        uvs = mlod.uvset(L, 0)
        assert all(abs(f["v"][k][2] - u[k][0]) < 1e-7 and abs(f["v"][k][3] - u[k][1]) < 1e-7
                   for f, u in zip(L["faces"], uvs) for k in range(3))
    print("  checks passed: visual faces", counts)


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    out_dir = None
    if "--out" in argv:
        i = argv.index("--out")
        out_dir = argv[i + 1]
        del argv[i:i + 2]
        os.makedirs(out_dir, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    for name in argv or list(PLAN):
        build(name, out_dir)


main()
