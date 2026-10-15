"""Readers for DayZ's Enfusion animation files: the skeleton in an .xob and the keyframes in an .anm.

Both are IFF-style chunk files (FORM, big-endian chunk sizes, little-endian contents). Layouts were worked out
from the infected files on P:\\DZ (2026-10-15); only what the infected skeleton and animations use is read.
"""
import struct
import zlib

import numpy as np


def chunks(data, start, end):
    out = []
    o = start
    while o + 8 <= end:
        tag = data[o:o + 4].decode("latin-1")
        size = struct.unpack(">I", data[o + 4:o + 8])[0]
        out.append((tag, o + 8, size))
        o += 8 + size
    return out


# ---------------------------------------------------------------------------------------------------------
# XOB6 skeleton
# ---------------------------------------------------------------------------------------------------------

class Bone:
    def __init__(self, index, name):
        self.index = index
        self.name = name
        self.parent = -1
        self.sibling = -1
        self.child = -1
        self.t = np.zeros(3)                 # local translation (parent space)
        self.q = np.array([0, 0, 0, 1.0])    # local rotation, quaternion x, y, z, w


def read_xob_skeleton(path, data=None):
    """Bones of an XOB6 file in file order, with parents and local rest transforms.

    HEAD: bounding box, material names, then the bone names (null-terminated, starting with Scene_Root),
    the LOD name, 6 bytes, then one 36-byte record per bone in name order:
    float3 translation, float4 quaternion (x, y, z, w), int16 next sibling, int16 first child, int16, uint16.
    Translations are in the parent bone's frame; limb bones point down their local X axis.
    `data` (the file's bytes, e.g. read from a PBO) is used instead of reading `path` when given.
    """
    data = data if data is not None else open(path, "rb").read()
    assert data[:4] == b"FORM" and data[8:12] == b"XOB6", path
    head = [c for c in chunks(data, 12, len(data)) if c[0] == "HEAD"][0]
    h = data[head[1]:head[1] + head[2]]
    i = h.index(b"Scene_Root\0")
    names = []
    o = i
    while True:                              # bone names run up to the mesh LOD's name ("Hermit_lod1")
        e = h.index(b"\0", o)
        s = h[o:e].decode("latin-1")
        o = e + 1
        if "_lod" in s.lower():
            break
        names.append(s)
    o += 6
    bones = [Bone(k, n) for k, n in enumerate(names)]
    for b in bones:
        b.t = np.array(struct.unpack_from("<3f", h, o))
        b.q = np.array(struct.unpack_from("<4f", h, o + 12))
        b.sibling, b.child = struct.unpack_from("<hh", h, o + 28)
        o += 36
    for b in bones:                          # parents from the first-child / next-sibling links
        c, seen = b.child, set()
        while c != -1:
            assert c not in seen and 0 <= c < len(bones), "bad sibling chain under %s" % b.name
            seen.add(c)
            bones[c].parent = b.index
            c = bones[c].sibling
    return bones


# ---------------------------------------------------------------------------------------------------------
# ANIMSET6 animation
# ---------------------------------------------------------------------------------------------------------

class Track:
    def __init__(self, name):
        self.name = name
        self.t_frames = self.r_frames = self.s_frames = None
        self.t = self.r = self.s = None       # (n, 3) / (n, 4 x, y, z, w) / (n, 3)


class Animation:
    def __init__(self, path, fps, frames, tracks, events):
        self.path = path
        self.fps = fps
        self.frames = frames                  # frame count
        self.tracks = tracks                  # name -> Track
        self.events = events                  # raw EVNT bytes
        self.additive = False                 # set by mark_additive()


def read_anm(path, data=None):
    """An ANIMSET6 .anm, or None for other versions (the infected crawl turns are ANIMSET5).

    FORM ANIM { SET6 { FPS, HEAD, DATA, EVNT } }. HEAD has one record per bone: float translation min and
    range, rotation min and range, scale min and range, uint16 frame count, uint16 translation / rotation /
    scale key counts, uint8, uint8 name length, name. DATA has, per bone in HEAD order: translation key frame
    numbers then values, scale keys likewise, rotation keys likewise; every frame number and value component
    is a uint16, and value = min + v / 65535 * range (quaternions x, y, z, w, normalised afterwards).
    `data` (the file's bytes) is used instead of reading `path` when given.
    """
    data = data if data is not None else open(path, "rb").read()
    if data[:4] != b"FORM" or data[8:16] != b"ANIMSET6":
        return None
    parts = {tag: data[o:o + n] for tag, o, n in chunks(data, 20, 20 + struct.unpack(">I", data[16:20])[0])}
    fps = struct.unpack("<I", parts["FPS\0"])[0]
    head = parts["HEAD"]
    recs = []
    p = 0
    while p + 34 <= len(head):
        rng = struct.unpack_from("<6f", head, p)
        frames, nt, nr, ns = struct.unpack_from("<4H", head, p + 24)
        n = head[p + 33]
        recs.append((head[p + 34:p + 34 + n].decode("latin-1"), rng, frames, nt, nr, ns))
        p += 34 + n
    body = parts["DATA"]
    o = 0

    def take(count, width, lo, rg):
        nonlocal o
        fr = np.frombuffer(body, "<u2", count, o).astype(int)
        o += 2 * count
        v = np.frombuffer(body, "<u2", count * width, o).reshape(count, width) / 65535.0 * rg + lo
        o += 2 * count * width
        return fr, v

    tracks = {}
    nframes = 1
    for name, rng, frames, nt, nr, ns in recs:
        tr = Track(name)
        tr.t_frames, tr.t = take(nt, 3, rng[0], rng[1])
        tr.s_frames, tr.s = take(ns, 3, rng[4], rng[5])
        tr.r_frames, tr.r = take(nr, 4, rng[2], rng[3])
        tr.r = tr.r / np.linalg.norm(tr.r, axis=1, keepdims=True)
        tracks[name] = tr
        nframes = max(nframes, frames)
    assert o == len(body), "%s: DATA not fully read" % path
    return Animation(path, fps, nframes, tracks, parts.get("EVNT", b""))


def mark_additive(anim, bones):
    """Hit reactions and look-at layers store zero translations (offsets on top of another animation)."""
    for b in bones:
        tr = anim.tracks.get(b.name)
        if tr is not None and np.linalg.norm(b.t) > 0.02 and len(tr.t) and not np.abs(tr.t).max() > 1e-4:
            anim.additive = True
            return True
    return False


def sample(frames, values, f, quat=False):
    """Value at frame f: linear between keys (slerp for quaternions), held past the ends; None if unkeyed."""
    if len(frames) == 0:
        return None
    if len(frames) == 1 or f <= frames[0]:
        return values[0]
    if f >= frames[-1]:
        return values[-1]
    k = np.searchsorted(frames, f) - 1
    a, b = values[k], values[k + 1]
    u = (f - frames[k]) / float(frames[k + 1] - frames[k])
    if not quat:
        return a + (b - a) * u
    if a @ b < 0:
        b = -b
    d = min(1.0, a @ b)
    if d > 0.9995:
        q = a + (b - a) * u
    else:
        th = np.arccos(d)
        q = (np.sin((1 - u) * th) * a + np.sin(u * th) * b) / np.sin(th)
    return q / np.linalg.norm(q)


def local_pose(anim, bones, f):
    """{bone index: (t, q)} local transforms at frame f; bones the animation doesn't key keep their rest."""
    out = {}
    for b in bones:
        tr = anim.tracks.get(b.name)
        if tr is None:
            continue
        out[b.index] = keyed_local(anim, b, tr, f)
    return out


def keyed_local(anim, b, tr, f, base=None):
    """Local (t, q) of bone b from its track at frame f. An unkeyed channel takes the base pose's value (the
    rest pose unless base = (t, q) is given); additive clips are offsets on top of that base."""
    bt, bq = base if base is not None else (b.t, b.q)
    t = sample(tr.t_frames, tr.t, f)
    q = sample(tr.r_frames, tr.r, f, quat=True)
    if anim.additive:
        t = bt + (t if t is not None else 0)
        q = qmul(bq, q) if q is not None else bq
    else:
        t = bt if t is None else t
        q = bq if q is None else q
    return t, q


def is_partial(anim):
    """Layer clips (light hits, look-at) key only part of the body: no pelvis track."""
    return "Pelvis" not in anim.tracks or anim.additive


def qmul(a, b):
    ax, ay, az, aw = a
    bx, by, bz, bw = b
    return np.array([aw * bx + ax * bw + ay * bz - az * by,
                     aw * by - ax * bz + ay * bw + az * bx,
                     aw * bz + ax * by - ay * bx + az * bw,
                     aw * bw - ax * bx - ay * by - az * bz])


def qrot(q, v):
    x, y, z, w = q
    u = np.array([x, y, z])
    return v + 2 * np.cross(u, np.cross(u, v) + w * v)


def world(bones, local=None):
    """World (rest, or given local {index: (t, q)}) transforms: list of (t, q)."""
    out = [None] * len(bones)
    for b in bones:                          # parents precede children in file order
        t, q = local[b.index] if local and b.index in local else (b.t, b.q)
        if b.parent == -1:
            out[b.index] = (t.copy(), q.copy())
        else:
            pt, pq = out[b.parent]
            out[b.index] = (pt + qrot(pq, t), qmul(pq, q))
    return out
