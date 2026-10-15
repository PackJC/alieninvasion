"""The alien floating in the tank: data/aliens/geb_GreenAlien.p3d posed through its own bone weights.

Pivots are the alien's joint landmarks (alienrig/rig.py), bones are bent parent-first like alienrig/posetest.py.
The model faces -Z and +X is its left. Returns each visual LOD as (points, faces in MLOD order, face UVs,
smooth outward normals per point), ready to be copied into the tank.
"""
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "alienrig"))
import mlod  # noqa: E402
import rig   # noqa: E402

ALIEN = os.path.join(HERE, "..", "..", "data", "aliens", "geb_GreenAlien.p3d")


def rot(axis, deg):
    a = np.asarray(axis, float)
    a /= np.linalg.norm(a)
    t = np.radians(deg)
    K = np.array([[0, -a[2], a[1]], [a[2], 0, -a[0]], [-a[1], a[0], 0]])
    return np.eye(3) + np.sin(t) * K + (1 - np.cos(t)) * K @ K


# Suspended, asleep: head bowed, arms hanging a little forward of the body with soft elbows, knees slightly
# bent and the toes pointing down, a little asymmetric. bone: [(pivot landmark, axis, degrees), ...] applied in
# order. About +X a positive angle swings a downward bone's tip forward (-Z) and a negative one tips an upward
# bone (spine, neck, head) forward; about +Z a negative angle brings a left (+X) arm in toward the body.
POSE = {
    "spine1": [("hipmid", (1, 0, 0), -4)],
    "spine3": [("neck", (1, 0, 0), -3)],
    "neck": [("neck", (1, 0, 0), -10)],
    "head": [("head", (1, 0, 0), -16), ("head", (0, 0, 1), 6)],
    "leftarm": [("Lshoulder", (0, 0, 1), -16), ("Lshoulder", (1, 0, 0), 10)],
    "leftforearm": [("Lelbow", (1, 0, 0), 22)],
    "lefthand": [("Lwrist", (1, 0, 0), 10)],
    "rightarm": [("Rshoulder", (0, 0, 1), 13), ("Rshoulder", (1, 0, 0), 15)],
    "rightforearm": [("Relbow", (1, 0, 0), 28)],
    "righthand": [("Rwrist", (1, 0, 0), 6)],
    "leftupleg": [("Lhip", (1, 0, 0), 9), ("Lhip", (0, 0, 1), 2)],
    "leftleg": [("Lknee", (1, 0, 0), -18)],
    "leftfoot": [("Lankle", (1, 0, 0), -36)],
    "rightupleg": [("Rhip", (1, 0, 0), 3), ("Rhip", (0, 0, 1), -2)],
    "rightleg": [("Rknee", (1, 0, 0), -10)],
    "rightfoot": [("Rankle", (1, 0, 0), -30)],
}


def bone_transforms(J, pose=POSE):
    """World (M, t) per bone: each bone carries its parent's motion plus its own bends."""
    G = {}
    for b in [a.lower() for a, _ in rig.SKELETON]:          # parents come before children
        Mp, tp = G.get(rig.PARENT[b], (np.eye(3), np.zeros(3)))
        Ml, tl = np.eye(3), np.zeros(3)
        for mark, axis, deg in pose.get(b, []):
            R = rot(axis, deg)
            piv = J[mark]
            Ml, tl = R @ Ml, R @ (tl - piv) + piv            # bend about the pivot, after the earlier bends
        G[b] = (Mp @ Ml, Mp @ tl + tp)
    return G


def skin(L, G):
    P = L["pts"].astype(float)
    Q = np.zeros_like(P)
    tot = np.zeros(len(P))
    for b, w in rig.bone_weights(L).items():
        if b in G and w.any():
            M, t = G[b]
            Q += w[:, None] * (P @ M.T + t)
            tot += w
    un = tot < 1e-6
    Q[un] = P[un]
    Q[~un] /= tot[~un, None]
    return Q


def point_normals(P, faces):
    """Area-weighted outward normals per point for MLOD-order faces (+cross points inward)."""
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


def posed(path=ALIEN, pose=POSE):
    m = mlod.read(path)
    vis = [L for L in m["lods"] if L["res"] < 100]
    J, _ = rig.donor_landmarks(vis[0])
    G = bone_transforms(J, pose)
    out = []
    for L in vis:
        Q = skin(L, G)
        faces = [[c[0] for c in f["v"][:f["n"]]] for f in L["faces"]]
        uvs = mlod.uvset(L)
        out.append(dict(pts=Q, faces=faces, uvs=uvs, normals=point_normals(Q, faces)))
    return out, J


if __name__ == "__main__":
    # python tools/alientank/specimen.py <out.p3d>: the posed alien alone, for alienrig/preview.py
    m = mlod.read(ALIEN)
    lods, J = posed()
    vi = 0
    for L in m["lods"]:
        if L["res"] < 100:
            L["pts"] = lods[vi]["pts"]
            vi += 1
    mlod.write(m, sys.argv[1])
    print("wrote", sys.argv[1])
