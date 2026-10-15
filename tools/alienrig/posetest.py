"""Bend the same bones on skinned MLODs and write posed copies, to compare the alien with a vanilla infected.

    python tools/alienrig/posetest.py <donor.p3d> <out dir> model.p3d [model2.p3d ...]

Pivots are the donor's joint landmarks (rig.donor_landmarks); children follow their parents through the
skeleton in data/aliens/model.cfg, twist/extra bones inherit their parent's motion. The pose is a zombie
lunge: arms forward, elbows bent, one leg stepping, spine leaning, head turned. View the copies with
preview.py next to the posed donor: a correctly skinned model bends exactly like it.
"""
import os
import sys

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mlod  # noqa: E402
import rig  # noqa: E402


def rot(axis, deg):
    a = np.asarray(axis, float)
    a /= np.linalg.norm(a)
    t = np.radians(deg)
    K = np.array([[0, -a[2], a[1]], [a[2], 0, -a[0]], [-a[1], a[0], 0]])
    return np.eye(3) + np.sin(t) * K + (1 - np.cos(t)) * K @ K


def main():
    dpath, outdir = sys.argv[1:3]
    D, _ = rig.donor_landmarks(rig.visual(dpath)[1])
    pose = {  # bone: (pivot, axis, degrees); models face -Z, +X is the model's left
        "spine1": (D["hipmid"] + [0, 0.1, 0], [1, 0, 0], -18),
        "head": (D["head"], [0, 1, 0], 35),
        "leftarm": (D["Lshoulder"], [1, 0, 0], -75),
        "leftforearm": (D["Lelbow"], [1, 0, 0], -60),
        "rightarm": (D["Rshoulder"], [0.3, 0, 1], 35),
        "rightforearm": (D["Relbow"], [1, 0, 0], -90),
        "leftupleg": (D["Lhip"], [1, 0, 0], -45),
        "leftleg": (D["Lknee"], [1, 0, 0], 70),
        "rightleg": (D["Rknee"], [1, 0, 0], 25),
        "leftfoot": (D["Lankle"], [1, 0, 0], -20),
    }
    G = {}
    for b in [a.lower() for a, _ in rig.SKELETON]:          # parents come before children
        Mp, tp = G.get(rig.PARENT[b], (np.eye(3), np.zeros(3)))
        if b in pose:
            piv, ax, deg = pose[b]
            Ml = rot(ax, deg)
            tl = piv - Ml @ piv
        else:
            Ml, tl = np.eye(3), np.zeros(3)
        G[b] = (Mp @ Ml, Mp @ tl + tp)
    os.makedirs(outdir, exist_ok=True)
    for path in sys.argv[3:]:
        m = mlod.read(path)
        for L in m["lods"]:
            if L["res"] > 100:
                continue
            P = L["pts"]
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
            L["pts"] = Q
        out = os.path.join(outdir, "posed_" + os.path.basename(path))
        mlod.write(m, out)
        print("wrote", out)


main()
