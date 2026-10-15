"""Move the alien's T-pose mesh onto the infected bind pose.

Each body region gets a transform that carries its alien joints onto the donor zombie's joints: rotation plus
a stretch along the bone (girth kept). Head, hands and feet keep their size. The region transforms are blended
across the joints with weights diffused along the mesh edges.

The alien's own selections only mark body regions: its "head" selection covered every vertex, which is why
the old model followed the head around. The head region is what no other bone touches.
"""
import numpy as np

import rig


def rot_between(a, b):
    a = a / np.linalg.norm(a)
    b = b / np.linalg.norm(b)
    v = np.cross(a, b)
    c = float(a @ b)
    if np.linalg.norm(v) < 1e-9:
        return np.eye(3)
    vx = np.array([[0, -v[2], v[1]], [v[2], 0, -v[0]], [-v[1], v[0], 0]])
    return np.eye(3) + vx + vx @ vx * (1 / (1 + c))


def seg(a0, a1, d0, d1):
    """Affine (M, t) taking alien segment a0->a1 onto donor d0->d1, stretched along the bone: p' = M p + t."""
    da, dd = a1 - a0, d1 - d0
    u = da / np.linalg.norm(da)
    k = np.linalg.norm(dd) / np.linalg.norm(da)
    M = rot_between(da, dd) @ (np.eye(3) + (k - 1) * np.outer(u, u))
    return M, d0 - M @ a0


def alien_regions(L):
    lab = rig.regions_from_weights(rig.bone_weights(L), len(L["pts"]), ignore=("head",))
    lab[lab == -1] = rig.REGIONS.index("HEAD")
    return lab


def conform(La, Ld, log=print):
    """Returns (conformed points, alien region per point, donor landmarks)."""
    P = La["pts"].copy()
    lab = alien_regions(La)
    E = rig.edges(La)
    A = rig.landmarks(P, E, lab)
    D, _ = rig.donor_landmarks(Ld)

    # The alien's shin/foot selection boundary runs along the sole; use the shin's lower end as the ankle.
    for s in ("L", "R"):
        shin = P[lab == rig.REGIONS.index(s + "_SHIN")]
        low = shin[shin[:, 1] <= np.percentile(shin[:, 1], 12)]
        A[s + "ankle"] = np.array([low[:, 0].mean(), 0.07, low[:, 2].mean()])

    T = {"TORSO": seg(A["hipmid"], A["neck"], D["hipmid"], D["neck"])}
    Mt = T["TORSO"][0]
    T["NECK"] = (np.eye(3), D["neck"] - A["neck"])          # head + neck: upright, alien size
    T["HEAD"] = T["NECK"]
    for s in ("L", "R"):
        T[s + "_CLAV"] = (Mt, D[s + "shoulder"] - Mt @ A[s + "shoulder"])
        T[s + "_UARM"] = seg(A[s + "shoulder"], A[s + "elbow"], D[s + "shoulder"], D[s + "elbow"])
        T[s + "_FARM"] = seg(A[s + "elbow"], A[s + "wrist"], D[s + "elbow"], D[s + "wrist"])
        # hand: alien size, turned with its forearm (keeps the alien's wrist angle and palm), at the donor's
        # wrist. The donor's own hands aren't mirror images (patient's right hand is curled forward).
        Rf = rot_between(A[s + "wrist"] - A[s + "elbow"], D[s + "wrist"] - D[s + "elbow"])
        T[s + "_HAND"] = (Rf, D[s + "wrist"] - Rf @ A[s + "wrist"])
        T[s + "_THIGH"] = seg(A[s + "hip"], A[s + "knee"], D[s + "hip"], D[s + "knee"])
        T[s + "_SHIN"] = seg(A[s + "knee"], A[s + "ankle"], D[s + "knee"], D[s + "ankle"])
        # foot: alien size, heel->toe turned like the donor's, sole kept on the ground
        fa = A[s + "toe"] - A[s + "ankle"]
        fd = D[s + "toe"] - D[s + "ankle"]
        fa[1] = fd[1] = 0
        R = rot_between(fa, fd)
        t = D[s + "ankle"] - R @ A[s + "ankle"]
        t[1] = 0.0
        T[s + "_FOOT"] = (R, t)

    n = len(P)
    Wr = np.zeros((len(rig.REGIONS), n))
    Wr[lab, np.arange(n)] = 1
    deg = np.bincount(E.ravel(), minlength=n).astype(float)
    for _ in range(12):
        acc = Wr.copy()
        np.add.at(acc.T, E[:, 0], Wr[:, E[:, 1]].T)
        np.add.at(acc.T, E[:, 1], Wr[:, E[:, 0]].T)
        Wr = acc / (1 + deg)
    Q = np.zeros_like(P)
    for i, r in enumerate(rig.REGIONS):
        M, t = T[r]
        Q += Wr[i][:, None] * (P @ M.T + t)
    log("conformed %d points; height %.3f -> %.3f m" % (n, np.ptp(P[:, 1]), np.ptp(Q[:, 1])))
    return Q, lab, D
