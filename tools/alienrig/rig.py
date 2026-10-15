"""Body regions, joint landmarks and weight helpers for fitting the alien onto the infected skeleton.

All vanilla infected share one skeleton and one bind pose (config ZombieBase enfanimsys: z_hermit_m.xob /
hermit_newbindpose.xob). The skeleton's bone list is the one in data/aliens/model.cfg. Bones are grouped into
body regions; a joint is measured the same way on any mesh: the centroid of the vertices where two regions
meet along mesh edges.
"""
import os
import re

import numpy as np

import mlod

HERE = os.path.dirname(os.path.abspath(__file__))
MODEL_CFG = os.path.join(HERE, "..", "..", "data", "aliens", "model.cfg")


def _skeleton():
    text = open(MODEL_CFG, encoding="utf-8").read()
    bones = re.findall(r'"([^"]*)"', re.search(r"SkeletonBones\[\]\s*=\s*\{(.*?)\};", text, re.S).group(1))
    return list(zip(bones[0::2], bones[1::2]))


SKELETON = _skeleton()
PARENT = {a.lower(): b.lower() for a, b in SKELETON}
BONES = set(PARENT)


def dec(b):
    """MLOD selection byte -> weight (0 = unselected, 1 = full, else (255 - b) / 254)."""
    b = b.astype(float)
    w = np.where(b == 1, 1.0, (255 - b) / 254)
    w[b == 0] = 0
    return w


def enc(w):
    """Weight 0..1 -> MLOD selection byte."""
    w = np.asarray(w, float)
    out = np.round(255 - 254 * w).astype(np.uint8)
    out[w >= 0.999] = 1
    out[w <= 0.001] = 0
    return out


REGION_OF = {}
for _side, _s in (("left", "L"), ("right", "R")):
    REGION_OF[_side + "shoulder"] = _s + "_CLAV"
    for _b in ("arm", "armroll", "armextra", "elbowextra"):
        REGION_OF[_side + _b] = _s + "_UARM"
    for _b in ("forearm", "forearmroll", "forearmextra", "wristextra"):
        REGION_OF[_side + _b] = _s + "_FARM"
    REGION_OF[_side + "hand"] = _s + "_HAND"
    for _f in ("index", "middle", "ring", "pinky", "thumb"):
        for _k in ("", "1", "2", "3"):
            REGION_OF[_side + "hand" + _f + _k] = _s + "_HAND"
    for _b in ("upleg", "uplegroll", "kneeextra"):
        REGION_OF[_side + _b] = _s + "_THIGH"
    REGION_OF[_side + "hipextra"] = "TORSO"
    for _b in ("leg", "legroll"):
        REGION_OF[_side + _b] = _s + "_SHIN"
    for _b in ("foot", "toebase"):
        REGION_OF[_side + _b] = _s + "_FOOT"
for _b in ("pelvis", "spine", "spine1", "spine2", "spine3"):
    REGION_OF[_b] = "TORSO"
REGION_OF["neck"] = "NECK"
REGION_OF["neck1"] = "HEAD"
REGION_OF["head"] = "HEAD"

REGIONS = ["HEAD", "NECK", "TORSO"] + [s + p for s in ("L", "R")
                                       for p in ("_CLAV", "_UARM", "_FARM", "_HAND", "_THIGH", "_SHIN", "_FOOT")]
ADJ = {"HEAD": ["NECK"], "NECK": ["HEAD", "TORSO", "L_CLAV", "R_CLAV"],
       "TORSO": ["NECK", "L_CLAV", "R_CLAV", "L_UARM", "R_UARM", "L_THIGH", "R_THIGH"]}
for _s in ("L", "R"):
    ADJ[_s + "_CLAV"] = ["TORSO", "NECK", _s + "_UARM"]
    ADJ[_s + "_UARM"] = [_s + "_CLAV", "TORSO", _s + "_FARM"]
    ADJ[_s + "_FARM"] = [_s + "_UARM", _s + "_HAND"]
    ADJ[_s + "_HAND"] = [_s + "_FARM"]
    ADJ[_s + "_THIGH"] = ["TORSO", _s + "_SHIN"]
    ADJ[_s + "_SHIN"] = [_s + "_THIGH", _s + "_FOOT"]
    ADJ[_s + "_FOOT"] = [_s + "_SHIN"]


def visual(path, lodi=0):
    m = mlod.read(path)
    return m, m["lods"][lodi]


def bone_weights(L):
    return {n.lower(): dec(pv) for n, (pv, pf) in mlod.selections(L).items() if n.lower() in BONES}


def face_mask_without_proxies(L):
    pf_all = np.zeros(len(L["faces"]), bool)
    for n, (pv, pf) in mlod.selections(L).items():
        if n.startswith("proxy:"):
            pf_all |= pf > 0
    return ~pf_all


def edges(L, faces=None):
    E = set()
    for fi, f in enumerate(L["faces"]):
        if faces is not None and not faces[fi]:
            continue
        idx = [f["v"][k][0] for k in range(f["n"])]
        for a, b in zip(idx, idx[1:] + idx[:1]):
            E.add((min(a, b), max(a, b)))
    return np.array(sorted(E))


def regions_from_weights(W, n, ignore=()):
    """Region index per point (strongest summed region), -1 where no listed bone touches it."""
    acc = {r: np.zeros(n) for r in REGIONS}
    for k, w in W.items():
        if k in REGION_OF and k not in ignore:
            acc[REGION_OF[k]] += w
    M = np.vstack([acc[r] for r in REGIONS])
    lab = M.argmax(0)
    lab[M.max(0) == 0] = -1
    return lab


def joint(P, E, lab, A, B):
    """Centroid of the boundary between region set A and region set B."""
    ia = np.isin(lab, [REGIONS.index(r) for r in A])
    ib = np.isin(lab, [REGIONS.index(r) for r in B])
    e = E[(ia[E[:, 0]] & ib[E[:, 1]]) | (ib[E[:, 0]] & ia[E[:, 1]])]
    if not len(e):
        return None
    return P[np.unique(e.ravel())].mean(0)


def landmarks(P, E, lab):
    J = {"neck": joint(P, E, lab, ["NECK"], ["TORSO", "L_CLAV", "R_CLAV"]),
         "head": joint(P, E, lab, ["HEAD"], ["NECK"])}
    for s in ("L", "R"):
        J[s + "clav"] = joint(P, E, lab, [s + "_CLAV"], ["TORSO", "NECK"])
        J[s + "shoulder"] = joint(P, E, lab, [s + "_UARM"], [s + "_CLAV", "TORSO"])
        J[s + "elbow"] = joint(P, E, lab, [s + "_FARM"], [s + "_UARM"])
        J[s + "wrist"] = joint(P, E, lab, [s + "_HAND"], [s + "_FARM"])
        J[s + "hip"] = joint(P, E, lab, [s + "_THIGH"], ["TORSO"])
        J[s + "knee"] = joint(P, E, lab, [s + "_SHIN"], [s + "_THIGH"])
        J[s + "ankle"] = joint(P, E, lab, [s + "_FOOT"], [s + "_SHIN"])
        hand = P[lab == REGIONS.index(s + "_HAND")]
        d = J[s + "wrist"] - J[s + "elbow"]
        proj = (hand - J[s + "wrist"]) @ (d / np.linalg.norm(d))
        J[s + "handtip"] = hand[proj >= np.percentile(proj, 95)].mean(0)
        foot = P[lab == REGIONS.index(s + "_FOOT")]
        J[s + "toe"] = foot[foot[:, 2] <= np.percentile(foot[:, 2], 5)].mean(0)
    head = P[lab == REGIONS.index("HEAD")]
    J["headtop"] = head[head[:, 1] >= np.percentile(head[:, 1], 97)].mean(0)
    J["hipmid"] = (J["Lhip"] + J["Rhip"]) / 2
    return J


def donor_landmarks(L):
    """Joint landmarks of a vanilla infected visual LOD (its own weights, proxies left out)."""
    lab = regions_from_weights(bone_weights(L), len(L["pts"]))
    return landmarks(L["pts"], edges(L, face_mask_without_proxies(L)), lab), lab
