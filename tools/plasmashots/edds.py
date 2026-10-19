r"""Reads Enfusion .edds textures (the particle sprites under graphics/particles/sprites) as Pillow images.

An .edds is a DDS header followed by one (tag, size) entry per mip, smallest mip first, then the mips' data in
the same order. "COPY" data is stored as is; "LZ4 " data is a uint32 unpacked size followed by chunks, each a
uint32 whose low 24 bits are the chunk's packed size and whose top bit marks the last chunk. The chunks are one
LZ4 stream: later chunks copy from what earlier ones unpacked, so they are unpacked into one buffer. Only the
largest mip is read; it is handed to Pillow as a plain one-mip DDS (Pillow reads the BGRA and DXT formats the
sprites use). Pure Python, no lz4 module needed: the sprites are small.

    python tools/plasmashots/edds.py in.edds out.png
"""
import io
import struct
import sys

from PIL import Image


def _lz4_chunk(src, pos, end, out):
    while pos < end:
        token = src[pos]
        pos += 1
        n = token >> 4
        if n == 15:
            while True:
                b = src[pos]
                pos += 1
                n += b
                if b != 255:
                    break
        out += src[pos:pos + n]
        pos += n
        if pos >= end:
            break
        offset = src[pos] | (src[pos + 1] << 8)
        pos += 2
        n = token & 15
        if n == 15:
            while True:
                b = src[pos]
                pos += 1
                n += b
                if b != 255:
                    break
        n += 4
        start = len(out) - offset
        if offset >= n:
            out += out[start:start + n]
        else:                                   # overlapping copy repeats the last `offset` bytes
            for i in range(n):
                out.append(out[start + i])


def _lz4_mip(data):
    size = struct.unpack_from("<I", data, 0)[0]
    out = bytearray()
    pos = 4
    while True:
        head = struct.unpack_from("<I", data, pos)[0]
        pos += 4
        n = head & 0xFFFFFF
        _lz4_chunk(data, pos, pos + n, out)
        pos += n
        if head & 0x80000000 or pos >= len(data):
            break
    assert len(out) == size, ("edds lz4 size", len(out), size)
    return bytes(out)


def read(path):
    raw = open(path, "rb").read()
    assert raw[:4] == b"DDS ", path
    header_end = 128
    if raw[84:88] == b"DX10":
        header_end += 20
    mips = struct.unpack_from("<I", raw, 28)[0] or 1
    table = []
    pos = header_end
    for _ in range(mips):
        tag, n = raw[pos:pos + 4], struct.unpack_from("<I", raw, pos + 4)[0]
        table.append((tag, n))
        pos += 8
    for tag, n in table[:-1]:
        pos += n
    tag, n = table[-1]
    data = raw[pos:pos + n]
    data = _lz4_mip(data) if tag == b"LZ4 " else data
    header = bytearray(raw[:header_end])
    struct.pack_into("<I", header, 8, struct.unpack_from("<I", header, 8)[0] & ~0x20000)   # no DDSD_MIPMAPCOUNT
    struct.pack_into("<I", header, 28, 1)
    struct.pack_into("<I", header, 108, struct.unpack_from("<I", header, 108)[0] & ~0x400008)  # no mip caps
    img = Image.open(io.BytesIO(bytes(header) + data))
    img.load()
    return img


if __name__ == "__main__":
    read(sys.argv[1]).save(sys.argv[2])
