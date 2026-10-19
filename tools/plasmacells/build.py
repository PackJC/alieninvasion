r"""Montauk Cartridge colours: a glowing cell window on the cartridge, one texture and material per plasma colour.

    python tools/plasmacells/build.py [--png DIR] [--skip-model] [--skip-textures]

The four cartridges (geb_PlasmaCartridge, _Arc, _Lance, _Void in data/config.cpp) share one model. This gives
it a "camo" hidden selection, a window along the lower half of both long sides, so config can show each
cartridge's colour there (hiddenSelectionsTextures/Materials). Writes, in data/weapons/:

  geb_plasmacartridge.p3d          from geb_plasmacartridge_orig.p3d (next to this file, the model as it was
                                   before). Only the two visual LODs change: their 32 window faces (the lower
                                   two rows of the 4x4 grid on each long side) join a new "camo" selection,
                                   are mapped across the whole window texture instead of a strip of the
                                   rifle's, and name the green window texture and material, so the model
                                   shows green without config. Points, normals and every other face are
                                   untouched; model.cfg lists "camo" in the model's sections[].
  geb_plasmacartridge_<colour>_co.paa   per colour: a dark bezel around a sunken window of plasma (two
                                   twisting strands with a white-hot core) split into six cells by struts
  geb_plasmacartridge_nohq.paa     shared: the window sunk into the bezel, the struts and the panel seams
  geb_plasmacartridge_smdi.paa     shared: matte bezel, glossy glass
  geb_plasmacartridge_<colour>.rvmat    per colour: Super shader with emmisive {1,1,1,brightness}

The colour lives in the texture. The Super shader multiplies emmisive by the texture colour (vanilla's
Christmas headband lights use {1,1,1,15}; tools/alientank's glow uses {1,1,1,3.5}), so a coloured emmisive on
a coloured texture would square the colour: lime would drift to green and violet to blue. A white emmisive
keeps each window exactly its colour, lit or dark, and the website's renders (which ignore emissive) still
show it. The bezel is almost black so that it barely glows. Brightness is evened out between colours by
sqrt(luminance), so the violet window is not much dimmer than the lime one.

Colours are the cells' muzzle-light colours (cfgAmmo geb_muzzleLightColor): green (0.3,1,0.4), blue
(0.3,0.7,1), lime (0.7,1,0.15), violet (0.7,0.3,1). Window UVs: u runs from the cartridge's +x end (u 0) to
its -x end, v from the window's top (v 0) to the bottom edge of the box, the same way round as the rifle
texture's mapping, so the window is mirrored on the side the inventory camera faces. The texture is drawn
symmetric for that reason.

Needs numpy, Pillow and DayZ Tools (ImageToPAA); reuses tools/alientank's texture and material helpers.
"""
import argparse
import os
import shutil
import struct
import sys
import tempfile

import numpy as np
from PIL import Image, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "alienrig"))
sys.path.insert(0, os.path.join(HERE, "..", "alientank"))
import mlod                                       # noqa: E402
from effects import rvmat                         # noqa: E402
from paint import noise, normal_map, smdi, to_paa  # noqa: E402

REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(REPO, "data", "weapons")
SRC_P3D = os.path.join(HERE, "geb_plasmacartridge_orig.p3d")
P3D = os.path.join(OUT, "geb_plasmacartridge.p3d")
TEX = "alieninvasion\\data\\weapons\\"
SELECTION = "camo"

# colour name: (cartridge class, sRGB colour)
COLOURS = {
    "green": ("geb_PlasmaCartridge", (0.3, 1.0, 0.4)),
    "blue": ("geb_PlasmaCartridge_Arc", (0.3, 0.7, 1.0)),
    "lime": ("geb_PlasmaCartridge_Lance", (0.7, 1.0, 0.15)),
    "violet": ("geb_PlasmaCartridge_Void", (0.7, 0.3, 1.0)),
}
DEFAULT = "green"
BRIGHTNESS = 4.0        # emmisive multiplier for green; the others are scaled to look about as bright

# The window: faces facing +-z (the long sides) between the bottom of the box (y 0.0162) and the second grid line
# (y 0.0285). The rows above end at 0.0285 and the tapered part below faces sideways and down, so neither matches.
BAND_Y = (0.0155, 0.0290)
EXPECT_FACES = 32

# texture layout (pixels). The window is 94.7 x 12.3 mm, so 512 x 64 is ~5.3 px/mm both ways.
W, H = 512, 64
WIN = (18, 11, W - 18, H - 11)      # window rectangle inside the bezel: x0, y0, x1, y1
WIN_R = 6                           # corner radius
CELLS = 6                           # cells the struts split the window into
STRUT = 1.6                         # strut half-width


def co_path(name):
    return TEX + "geb_plasmacartridge_%s_co.paa" % name


def rvmat_path(name):
    return TEX + "geb_plasmacartridge_%s.rvmat" % name


# --------------------------------------------------------------------------------------------------------------
# model
# --------------------------------------------------------------------------------------------------------------

def window_faces(L):
    P = L["pts"]
    out = []
    for fi, f in enumerate(L["faces"]):
        q = P[[v[0] for v in f["v"][:f["n"]]]]
        n = np.cross(q[1] - q[0], q[2] - q[0])
        n /= np.linalg.norm(n)
        if abs(n[2]) > 0.99 and q[:, 1].min() > BAND_Y[0] and q[:, 1].max() < BAND_Y[1]:
            out.append(fi)
    return out


def set_uvset(L, uvs):
    for i, (act, name, data) in enumerate(L["taggs"]):
        if name == "#UVSet#" and struct.unpack_from("<I", data, 0)[0] == 0:
            out = bytearray(struct.pack("<I", 0))
            for f, face_uv in zip(L["faces"], uvs):
                assert len(face_uv) == f["n"]
                for u, v in face_uv:
                    out += struct.pack("<ff", u, v)
            assert len(out) == len(data)
            L["taggs"][i] = (act, name, bytes(out))
            return
    raise ValueError("no #UVSet# 0")


def build_model():
    m = mlod.read(SRC_P3D)
    for L in m["lods"]:
        if L["res"] >= 1e12:            # Geometry, Memory, View/Fire Geometry: no texture, no hidden selections
            continue
        assert mlod.tagg(L, SELECTION) is None, "source already has a %s selection" % SELECTION
        faces = window_faces(L)
        assert len(faces) == EXPECT_FACES, (L["res"], len(faces))
        P = L["pts"]
        uvs = mlod.uvset(L, 0)
        zc = (P[:, 2].min() + P[:, 2].max()) / 2
        for side in (-1, 1):            # each long side's window spans the whole texture
            fs = [fi for fi in faces if np.sign(P[L["faces"][fi]["v"][0][0]][2] - zc) == side]
            assert len(fs) == EXPECT_FACES // 2, (side, len(fs))
            pis = sorted({v[0] for fi in fs for v in L["faces"][fi]["v"][:3]})
            (x0, y0), (x1, y1) = P[pis][:, :2].min(0), P[pis][:, :2].max(0)
            for fi in fs:
                f = L["faces"][fi]
                new = []
                for k, (pi, ni, _u, _v) in enumerate(f["v"]):
                    if k < f["n"]:
                        x, y = P[pi][0], P[pi][1]
                        u, v = float((x1 - x) / (x1 - x0)), float((y1 - y) / (y1 - y0))
                        new.append((pi, ni, u, v))
                        uvs[fi][k] = (u, v)
                    else:
                        new.append((pi, ni, _u, _v))
                f["v"] = new
                f["tex"], f["mat"] = co_path(DEFAULT), rvmat_path(DEFAULT)
        set_uvset(L, uvs)
        sel = np.zeros(len(P) + len(L["faces"]), np.uint8)
        for fi in faces:
            sel[len(P) + fi] = 1
            for v in L["faces"][fi]["v"][:L["faces"][fi]["n"]]:
                sel[v[0]] = 1
        at = next(i for i, t in enumerate(L["taggs"]) if t[1] == "#UVSet#")
        L["taggs"].insert(at, (1, SELECTION, sel.tobytes()))
    data = mlod.write(m)
    check_model(data)
    with open(P3D, "wb") as fh:
        fh.write(data)
    print("wrote data/weapons/geb_plasmacartridge.p3d (%d bytes, from %s)" % (len(data), os.path.basename(SRC_P3D)))


def check_model(data):
    """The new model differs from the original only in the window faces' UVs, texture and material and the new
    selection."""
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "check.p3d")
        with open(path, "wb") as fh:
            fh.write(data)
        new = mlod.read(path)
    old = mlod.read(SRC_P3D)
    assert len(new["lods"]) == len(old["lods"])
    for a, b in zip(old["lods"], new["lods"]):
        assert np.array_equal(a["raw_pts"], b["raw_pts"]) and np.array_equal(a["norms"], b["norms"])
        sel = mlod.selections(b).get(SELECTION)
        win = set(np.flatnonzero(sel[1])) if sel is not None else set()
        assert len(win) == (EXPECT_FACES if a["res"] < 1e12 else 0)
        for fi, (fa, fb) in enumerate(zip(a["faces"], b["faces"])):
            same_corners = [c[:2] for c in fa["v"]] == [c[:2] for c in fb["v"]]
            assert same_corners and fa["n"] == fb["n"] and fa["flags"] == fb["flags"]
            if fi not in win:
                assert fa == fb, fi
        names_a = [t[1] for t in a["taggs"]]
        names_b = [t[1] for t in b["taggs"]]
        assert [n for n in names_b if n != SELECTION] == names_a
        if win:
            uv = np.array([c for fi in win for c in mlod.uvset(b, 0)[fi]])
            assert uv.min() >= -1e-6 and uv.max() <= 1 + 1e-6


# --------------------------------------------------------------------------------------------------------------
# textures
# --------------------------------------------------------------------------------------------------------------

def _grid():
    y, x = np.mgrid[0:H, 0:W] + 0.5
    return x, y


def _window_sd():
    """Signed distance (px) to the window's rounded rectangle, negative inside."""
    x, y = _grid()
    x0, y0, x1, y1 = WIN
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    qx = np.abs(x - cx) - ((x1 - x0) / 2 - WIN_R)
    qy = np.abs(y - cy) - ((y1 - y0) / 2 - WIN_R)
    return np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) - WIN_R


def _struts():
    """Coverage of the struts between the cells (0..1)."""
    x, _ = _grid()
    x0, _, x1, _ = WIN
    cov = np.zeros((H, W))
    for k in range(1, CELLS):
        cov = np.maximum(cov, np.clip(STRUT + 0.5 - np.abs(x - (x0 + (x1 - x0) * k / CELLS)), 0, 1))
    return cov


def plasma():
    """Glow intensity in the window (about 0.2 at the rim to 1.6 on the strands' cores), the same for all colours."""
    x, y = _grid()
    x0, y0, x1, y1 = WIN
    s = (x - x0) / (x1 - x0)
    mid, half = (y0 + y1) / 2, (y1 - y0) / 2
    # two strands twisting round each other; symmetric about the middle, since one side shows it mirrored
    phase = 2 * np.pi * 2.5 * np.abs(s - 0.5)
    strands = [mid + 0.42 * half * np.sin(phase + a) for a in (0, np.pi)]
    core = np.maximum.reduce([np.exp(-((y - c) / 1.8) ** 2) for c in strands])
    halo = np.maximum.reduce([np.exp(-((y - c) / 6.5) ** 2) for c in strands])
    centre = np.exp(-((y - mid) / (0.75 * half)) ** 2)
    n = noise(H, W, 5, seed=11)                     # 5 blobs each way: long streaks in a 512 x 64 image
    sd = _window_sd()
    rim = np.clip(-sd / 5, 0, 1)                    # dimmer towards the window's edge, in the bezel's shadow
    return (0.22 + 0.30 * centre + 0.45 * halo + 0.85 * core + 0.18 * (n - 0.5)) * (0.55 + 0.45 * rim)


def colour_texture(rgb):
    rgb = np.asarray(rgb, float)
    sd = _window_sd()
    cov = np.clip(0.5 - sd, 0, 1)[..., None]
    struts = _struts()[..., None] * cov
    # bezel: near-black gunmetal with a hint of the colour (it glows faintly through emmisive), a lit lip around
    # the window and dark seams where the panel meets the rows above and below
    grain = noise(H, W, 24, seed=5)[..., None]
    lip = np.exp(-((sd - 1.4) / 1.0) ** 2)[..., None]
    _, y = _grid()
    seam = (np.exp(-((y - 0.5) / 0.8) ** 2) + np.exp(-((y - H + 0.5) / 0.8) ** 2))[..., None]
    bezel = np.array([0.105, 0.11, 0.10]) + 0.025 * rgb + 0.025 * (grain - 0.5) + 0.07 * lip
    bezel = bezel * (1 - 0.5 * seam)
    i = plasma()[..., None]
    glow = rgb * np.clip(i, 0, 1)
    white = np.clip((i - 0.85) / 0.6, 0, 1) * 0.75     # white-hot where the strands are brightest
    glow = glow + (1 - glow) * white
    col = bezel * (1 - cov) + glow * cov
    col = col * (1 - struts) + (bezel + 0.03) * struts
    return Image.fromarray(np.clip(col * 255 + 0.5, 0, 255).astype(np.uint8), "RGB")


def relief():
    """_nohq: bezel flat, window glass sunk below it, struts and seams in between."""
    sd = _window_sd()
    cov = np.clip(0.5 - sd, 0, 1)
    _, y = _grid()
    seam = np.exp(-((y - 0.5) / 0.8) ** 2) + np.exp(-((y - H + 0.5) / 0.8) ** 2)
    h = 1 - 0.6 * cov + 0.45 * _struts() * cov - 0.35 * seam
    h = np.asarray(Image.fromarray((h * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(0.8)), float) / 255
    return normal_map(h, 3.0)


def shine():
    cov = np.clip(0.5 - _window_sd(), 0, 1) * (1 - _struts())
    return smdi(0.28 + 0.6 * cov, 0.4 + 0.5 * cov)


def luminance(rgb):
    lin = [((c + 0.055) / 1.055) ** 2.4 if c > 0.04045 else c / 12.92 for c in rgb]
    return 0.2126 * lin[0] + 0.7152 * lin[1] + 0.0722 * lin[2]


def brightness(rgb):
    return round(BRIGHTNESS * (luminance(COLOURS[DEFAULT][1]) / luminance(rgb)) ** 0.5, 1)


def build_textures(png_dir=None):
    images = {"geb_plasmacartridge_nohq": relief(), "geb_plasmacartridge_smdi": shine()}
    for name, (_cls, rgb) in COLOURS.items():
        images["geb_plasmacartridge_%s_co" % name] = colour_texture(rgb)
    with tempfile.TemporaryDirectory() as tmp:
        for name, img in images.items():
            to_paa(img, os.path.join(OUT, name + ".paa"), tmp)
            print("wrote data/weapons/%s.paa" % name)
            if png_dir:
                os.makedirs(png_dir, exist_ok=True)
                shutil.copy(os.path.join(tmp, name + ".png"), os.path.join(png_dir, name + ".png"))


def build_materials():
    for name, (_cls, rgb) in COLOURS.items():
        k = brightness(rgb)
        text = rvmat(TEX + "geb_plasmacartridge_nohq.paa", TEX + "geb_plasmacartridge_smdi.paa",
                     emissive=(1, 1, 1, k), specular=(0.6, 0.6, 0.6), power=90)
        path = os.path.join(OUT, "geb_plasmacartridge_%s.rvmat" % name)
        with open(path, "w", encoding="ascii", newline="\r\n") as fh:
            fh.write(text)
        print("wrote data/weapons/%s (emmisive brightness %g)" % (os.path.basename(path), k))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--png", help="also save the textures as PNG in this folder")
    ap.add_argument("--skip-model", action="store_true")
    ap.add_argument("--skip-textures", action="store_true")
    a = ap.parse_args()
    if not a.skip_model:
        build_model()
    if not a.skip_textures:
        build_textures(a.png)
    build_materials()
    print("config: hiddenSelections[]={\"%s\"}; textures and materials per cartridge:" % SELECTION)
    for name, (cls, _rgb) in COLOURS.items():
        print("  %-26s \\%s  \\%s" % (cls, co_path(name), rvmat_path(name)))


if __name__ == "__main__":
    main()
