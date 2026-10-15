"""Full MLOD (.p3d) reader/writer that round-trips byte-exactly.

python mlod.py file.p3d [...]   lists each LOD (points, faces, mass, properties,
                                selections, box) and checks the round trip.
read(path) -> dict(version, lods=[LOD]); write(model, path)
LOD: res, major, minor, flags, pts (N,3) float32, pflags (N,) uint32, norms (M,3),
     faces: list of dict(n, v=[(pi, ni, u, v)]*4, flags, tex, mat), taggs: list of (active, name, bytes)
"""
import struct
import numpy as np


def _cstr(b, o):
    e = b.index(b"\0", o)
    return b[o:e].decode("latin-1"), e + 1


def read(path):
    b = open(path, "rb").read()
    assert b[:4] == b"MLOD", b[:4]
    version, n = struct.unpack_from("<II", b, 4)
    o = 12
    lods = []
    for _ in range(n):
        assert b[o:o + 4] == b"P3DM"
        major, minor, npts, nnorm, nfaces, flags = struct.unpack_from("<6I", b, o + 4)
        o += 28
        raw = np.frombuffer(b, dtype="<f4", count=npts * 4, offset=o).reshape(-1, 4) if npts else np.zeros((0, 4), "<f4")
        pts = raw[:, :3].astype(np.float64)
        pflags = np.frombuffer(b, dtype="<u4", count=npts * 4, offset=o).reshape(-1, 4)[:, 3].copy() if npts else np.zeros(0, "<u4")
        o += 16 * npts
        norms = np.frombuffer(b, dtype="<f4", count=nnorm * 3, offset=o).reshape(-1, 3).astype(np.float64) if nnorm else np.zeros((0, 3))
        o += 12 * nnorm
        faces = []
        for _f in range(nfaces):
            cnt = struct.unpack_from("<I", b, o)[0]
            vs = [struct.unpack_from("<IIff", b, o + 4 + 16 * k) for k in range(4)]
            fl = struct.unpack_from("<I", b, o + 68)[0]
            o += 72
            tex, o = _cstr(b, o)
            mat, o = _cstr(b, o)
            faces.append(dict(n=cnt, v=vs, flags=fl, tex=tex, mat=mat))
        assert b[o:o + 4] == b"TAGG", o
        o += 4
        taggs = []
        while True:
            act = b[o]; o += 1
            name, o = _cstr(b, o)
            size = struct.unpack_from("<I", b, o)[0]; o += 4
            data = b[o:o + size]; o += size
            taggs.append((act, name, data))
            if name == "#EndOfFile#":
                break
        res = struct.unpack_from("<f", b, o)[0]; o += 4
        lods.append(dict(res=res, major=major, minor=minor, flags=flags, pts=pts, pflags=pflags,
                         norms=norms, faces=faces, taggs=taggs, raw_pts=raw.copy()))
    assert o == len(b), (o, len(b))
    return dict(version=version, lods=lods)


def write(m, path=None):
    out = bytearray(b"MLOD")
    out += struct.pack("<II", m["version"], len(m["lods"]))
    for L in m["lods"]:
        npts = len(L["pts"]); nn = len(L["norms"]); nf = len(L["faces"])
        out += b"P3DM" + struct.pack("<6I", L["major"], L["minor"], npts, nn, nf, L["flags"])
        for p, fl in zip(L["pts"], L["pflags"]):
            out += struct.pack("<fffI", p[0], p[1], p[2], int(fl))
        for q in L["norms"]:
            out += struct.pack("<fff", *q)
        for f in L["faces"]:
            out += struct.pack("<I", f["n"])
            for v in f["v"]:
                out += struct.pack("<IIff", *v)
            out += struct.pack("<I", f["flags"])
            out += f["tex"].encode("latin-1") + b"\0" + f["mat"].encode("latin-1") + b"\0"
        out += b"TAGG"
        for act, name, data in L["taggs"]:
            out += bytes([act]) + name.encode("latin-1") + b"\0" + struct.pack("<I", len(data)) + data
        out += struct.pack("<f", L["res"])
    if path:
        open(path, "wb").write(out)
    return bytes(out)


def tagg(L, name):
    for a, n, d in L["taggs"]:
        if n.lower() == name.lower():
            return d
    return None


def selections(L):
    npts = len(L["pts"]); nf = len(L["faces"])
    out = {}
    for a, n, d in L["taggs"]:
        if not n.startswith("#") and len(d) == npts + nf:
            out[n] = (np.frombuffer(d[:npts], np.uint8).copy(), np.frombuffer(d[npts:], np.uint8).copy())
    return out


def props(L):
    out = {}
    for a, n, d in L["taggs"]:
        if n == "#Property#":
            out[d[:64].split(b"\0")[0].decode("latin-1")] = d[64:128].split(b"\0")[0].decode("latin-1")
    return out


def uvset(L, sid=0):
    """Return list per face of [(u,v)]*n from #UVSet# with the given id."""
    for a, n, d in L["taggs"]:
        if n == "#UVSet#" and struct.unpack_from("<I", d, 0)[0] == sid:
            o = 4; out = []
            for f in L["faces"]:
                out.append([struct.unpack_from("<ff", d, o + 8 * k) for k in range(f["n"])])
                o += 8 * f["n"]
            assert o == len(d)
            return out
    return None


def tris(L):
    """Triangulated faces as list of (face index, (i0,i1,i2) corner indices into f['v'])."""
    out = []
    for fi, f in enumerate(L["faces"]):
        if f["n"] == 3:
            out.append((fi, (0, 1, 2)))
        elif f["n"] == 4:
            out.append((fi, (0, 1, 2)))
            out.append((fi, (0, 2, 3)))
    return out


LOD_NAMES = {1e13: "Geometry", 1e15: "Memory", 2e15: "LandContact", 6e15: "ViewGeometry", 7e15: "FireGeometry"}


def lod_name(res):
    for k, v in LOD_NAMES.items():
        if abs(res - k) <= k * 1e-6:
            return v
    return "visual %g" % res


if __name__ == "__main__":
    import struct, sys
    for p in sys.argv[1:]:
        m = read(p)
        print("==", p, "round trip", write(m) == open(p, "rb").read())
        for L in m["lods"]:
            line = "  %-14s pts=%-6d faces=%-6d" % (lod_name(L["res"]), len(L["pts"]), len(L["faces"]))
            if L["res"] > 1e12:                 # Geometry and up (stored as float32, 1e13 reads just under)
                d = tagg(L, "#Mass#")
                mass = sum(struct.unpack("<%df" % (len(d) // 4), d)) if d else None
                sels = [n for n in selections(L) if not n.startswith("-")]
                lo, hi = (L["pts"].min(0), L["pts"].max(0)) if len(L["pts"]) else ((0, 0, 0), (0, 0, 0))
                line += " mass=%s props=%s sels=%s box=%s..%s" % (mass, props(L), sels[:12], np.round(lo, 3), np.round(hi, 3))
            print(line)
