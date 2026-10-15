"""Read files straight out of a PBO (uncompressed or LZSS 'Cprs' entries), for the installed game's own data.

    files = entries(path)            -> {lower-case path with '/': (offset, stored size, original size, packed)}
    data = read(path, entry)         -> bytes
"""
import struct


def _lzss(data, size):
    out = bytearray()
    i = 0
    while len(out) < size and i < len(data):
        flags = data[i]
        i += 1
        for b in range(8):
            if len(out) >= size or i >= len(data):
                break
            if flags & (1 << b):
                out.append(data[i])
                i += 1
            else:
                lo, hi = data[i], data[i + 1]
                i += 2
                rpos = len(out) - (lo | ((hi & 0xF0) << 4))
                for k in range((hi & 0x0F) + 3):
                    p = rpos + k
                    out.append(out[p] if p >= 0 else 0x20)
    return bytes(out[:size])


def _cstr(f):
    b = bytearray()
    while True:
        c = f.read(1)
        if not c or c == b"\0":
            return b.decode("latin-1")
        b += c


def entries(path):
    with open(path, "rb") as f:
        heads = []
        while True:
            name = _cstr(f)
            mime, orig, _, _, size = struct.unpack("<5I", f.read(20))
            if not name and mime == 0x56657273:          # 'Vers' header and its properties
                while _cstr(f):
                    _cstr(f)
                continue
            if not name:
                break
            heads.append((name, mime, orig, size))
        off = f.tell()
    out = {}
    for name, mime, orig, size in heads:
        packed = mime == 0x43707273 or (orig and orig != size)
        out[name.replace("\\", "/").lower()] = (off, size, orig, packed)
        off += size
    return out


def read(path, entry):
    off, size, orig, packed = entry
    with open(path, "rb") as f:
        f.seek(off)
        data = f.read(size)
    return _lzss(data, orig) if packed else data
