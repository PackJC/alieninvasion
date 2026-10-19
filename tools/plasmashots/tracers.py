r"""Plasma bolts (tracer models) for the blue and violet plasma cells.

    python tools/plasmashots/tracers.py

Writes data/weapons/geb_tracer_<id>.p3d (MLOD), geb_tracer_<id>_ca.paa and geb_tracer_<id>.rvmat for each colour
below. data/config.cpp gives each cell its bolt through CfgAmmo model=:
  blue    Plasma_Cell_Arc     geb_tracer_blue.p3d
  violet  Plasma_Cell_Void    geb_tracer_violet.p3d
Green (Plasma_Cell) and lime (Plasma_Cell_Lance) fire vanilla's tracer_green / tracer_yellow. Vanilla ships only
green, red and yellow tracers, so blue and violet are made here. Needs numpy, Pillow and DayZ Tools (ImageToPAA).

How vanilla's tracers are built (P:\DZ\weapons\projectiles\tracer_*.p3d, ODOL; read from a debinarized copy kept
outside the repo, and their rvmats and _ca textures in projectiles\data):
  - one visual LOD (resolution 1) and nothing else: no Geometry, Memory or Shadow LOD, so no collision and no
    shadow. LOD property autocenter=0 keeps the origin where it is modelled instead of at the bounding box centre.
  - an 8-sided spindle along +Z, 1.65 m long and 5 cm thick: a point at z -0.013, rings of radius 0.025, 0.024 and
    0.020 at z 0.009, 0.342 and 1.2, a point at z 1.637. CfgAmmo tracerScale scales the whole model (1.5 on
    Plasma_Cell). The bright end is at the origin and the spindle fades out towards +Z.
  - at the origin, an 8-segment disc whose faces point along +Z (a very flat cone, centre 2 cm towards -Z): the
    round glow seen when looking along the bolt. tracer_green's is 7.4 cm in radius, tracer_yellow's 3.6 cm.
  - every vertex normal is the same (+Y), so nothing shades across the bolt.
  - one 256 x 64 _ca texture. The left ~80 % is an alpha ramp along u (opaque at u 0, gone by u 0.81, the same in
    every row); the spindle is mapped along it, u 0.013 at the origin, 0.066 / 0.303 / 0.718 at the rings and
    0.771 at the far point. The rightmost ~20 % holds a soft round glow (gaussian, ~16 px radius, around u 0.9)
    that the disc is mapped onto. Colour is nearly flat; red leans pink near u 0.
  - rvmat (tracer_green/red): PixelShaderID Normal, VertexShaderID Basic, diffuse 0, emmisive 3, renderFlags
    NoZWrite + AddBlend. The bolt only adds light, writes no depth and ignores the sun, so the texture's colour
    times its alpha is what shows.
  - tracer_green/red also carry a skeleton (Skeleton_DZbullet, bones bullet_rot / bullet_rot2 turned by the
    "time" source). tracer_yellow has none and Plasma_Cell_Lance fires it fine, so these bolts stay static.
Making new models was the only route: CfgAmmo has no hiddenSelectionsTextures (a shot isn't an EntityAI), and
patching the path inside a binarized vanilla model would ship an edited vanilla asset.

What these bolts keep and change:
  - Same mesh layout and size as tracer_green (its larger disc), so all four colours read as the same bolt.
  - The texture is painted here: the same ramp and glow shapes, coloured from the cell's palette in shots.py
    (ARC / VOID, the muzzle effects' "main" and "hot") so bolt and muzzle match. "main" is the cells'
    geb_muzzleLightColor; texels take the palette values as sRGB, as vanilla's saturated green does. The bright
    end and the disc centre lean towards "hot" (whiter), like red's pink core.
  - The rvmat copies tracer_green's, with emmisive per colour (EMISSIVE): violet is the darkest of the four
    colours (linear luminance ~0.2 against red's 0.29 and green's 0.74), so it gets a little more.
"""
import os
import struct
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "alienrig"))
import mlod    # noqa: E402
import shots   # noqa: E402

DAYZ_TOOLS = os.environ.get("DAYZ_TOOLS", r"G:\SteamLibrary\steamapps\common\DayZ Tools")
IMAGE_TO_PAA = os.path.join(DAYZ_TOOLS, "Bin", "ImageToPAA", "ImageToPAA.exe")
OUT = os.path.join(shots.REPO, "data", "weapons")
GAME_DIR = "alieninvasion\\data\\weapons\\"

# id, palette (shots.py), emmisive
TRACERS = [
    ("blue", shots.ARC, 3.0),
    ("violet", shots.VOID, 3.5),
]

# --------------------------------------------------------------------------------------------------------------
# mesh (vanilla tracer_green's layout, see the docstring)
# --------------------------------------------------------------------------------------------------------------

SIDES = 8
# Along the bolt: (z, radius, u). Radius 0 is a point. u is where that ring samples the texture's ramp.
PROFILE = [(-0.013, 0.0, 0.013), (0.009, 0.025, 0.066), (0.342, 0.0242, 0.303), (1.2, 0.02, 0.718),
           (1.637, 0.0, 0.771)]
V_SPAN = 0.3                 # spindle v = 0.5 + V_SPAN * sin(angle); the ramp is the same in every row
DISC_R, DISC_Z, DISC_CENTRE_Z = 0.0735, 0.0101, -0.0091
# Disc glow in the texture: centre (u, v) and radius (u, v); 20 px either way on a 256 x 64 texture
GLOW_UV, GLOW_R = (0.9, 0.5), (0.08, 0.32)


def _face(pts, idx, uv, outward):
    """Face dict in MLOD order (corners reversed, so +cross points inward) after checking that idx, listed
    counter-clockwise seen from outside, really faces `outward`."""
    a, b, c = (pts[i] for i in idx[:3])
    assert np.dot(np.cross(b - a, c - a), outward) > 0, (idx, outward)
    v = [(i, 0, float(u), float(w)) for i, (u, w) in zip(idx[::-1], uv[::-1])]
    return dict(n=len(v), v=v + [(0, 0, 0.0, 0.0)] * (4 - len(v)), flags=0)


def bolt_lod(texture, rvmat):
    pts, faces = [], []
    ang = np.arange(SIDES) * 2 * np.pi / SIDES

    def add(p):
        pts.append(np.array(p, float))
        return len(pts) - 1

    # spindle: one point per apex, SIDES per ring
    rings = []
    for z, r, u in PROFILE:
        if r == 0:
            rings.append([(add((0, 0, z)), (u, 0.5))] * SIDES)
        else:
            rings.append([(add((r * np.cos(t), r * np.sin(t), z)), (u, 0.5 + V_SPAN * np.sin(t))) for t in ang])
    P = np.array(pts)
    for k in range(SIDES):
        k1 = (k + 1) % SIDES
        mid = ang[k] + np.pi / SIDES
        outward = np.array([np.cos(mid), np.sin(mid), 0.0])
        for lo, hi in zip(rings[:-1], rings[1:]):
            # counter-clockwise seen from outside: round the near ring, then back along the far one
            corners = [lo[k], lo[k1], hi[k1], hi[k]]
            if corners[0][0] == corners[1][0]:          # near point
                del corners[1]
            elif corners[2][0] == corners[3][0]:        # far point
                del corners[3]
            faces.append(_face(P, [c[0] for c in corners], [c[1] for c in corners], outward))

    # disc at the origin, facing +Z
    centre = add((0, 0, DISC_CENTRE_Z))
    rim = [add((DISC_R * np.cos(t), DISC_R * np.sin(t), DISC_Z)) for t in ang]
    P = np.array(pts)

    def disc_uv(i):
        x, y = P[i][0] / DISC_R, P[i][1] / DISC_R
        return GLOW_UV[0] + GLOW_R[0] * x, GLOW_UV[1] - GLOW_R[1] * y
    for k in range(SIDES):
        idx = [centre, rim[k], rim[(k + 1) % SIDES]]
        faces.append(_face(P, idx, [disc_uv(i) for i in idx], np.array([0, 0, 1.0])))

    for f in faces:
        f["tex"], f["mat"] = texture, rvmat
    uvdata = bytearray(struct.pack("<I", 0))
    for f in faces:
        for _, _, u, w in f["v"][:f["n"]]:
            uvdata += struct.pack("<ff", u, w)
    autocenter = (1, "#Property#", b"autocenter".ljust(64, b"\0") + b"0".ljust(64, b"\0"))
    return dict(res=1.0, major=28, minor=256, flags=0, pts=P, pflags=np.zeros(len(P), np.uint32),
                # one normal for every corner, stored inward like all MLOD normals: the bolt is lit evenly
                norms=np.array([[0.0, -1.0, 0.0]]), faces=faces,
                taggs=[autocenter, (1, "#UVSet#", bytes(uvdata)), (1, "#EndOfFile#", b"")])


# --------------------------------------------------------------------------------------------------------------
# texture and material
# --------------------------------------------------------------------------------------------------------------

W, H = 256, 64
RAMP_END = 0.81              # u where the spindle's ramp reaches 0
HOT_TAIL, HOT_GLOW = 0.45, 0.7


def smoothstep(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1)
    return t * t * (3 - 2 * t)


def texture(palette):
    """RGBA uint8, 256 x 64: alpha ramp for the spindle on the left, round glow for the disc on the right."""
    u = (np.arange(W) + 0.5) / W
    v = (np.arange(H) + 0.5) / H
    U, V = np.meshgrid(u, v)
    ramp = np.clip(1 - (np.minimum(U, RAMP_END) / RAMP_END) ** 2.2, 0, 1) ** 1.2
    rho = np.hypot((U - GLOW_UV[0]) / GLOW_R[0], (V - GLOW_UV[1]) / GLOW_R[1])
    glow = np.minimum(1, 1.15 * np.exp(-(rho / 0.42) ** 2)) * (1 - smoothstep(0.85, 1, rho))
    in_glow = U > RAMP_END
    alpha = np.where(in_glow, glow, ramp)
    hot = np.where(in_glow, HOT_GLOW * np.exp(-(rho / 0.3) ** 2), HOT_TAIL * (1 - smoothstep(0, 0.35, U)))
    main, white = np.array(palette["main"], float), np.array(palette["hot"], float)
    rgb = main + (white - main) * hot[..., None]      # transparent texels keep "main", so mips don't darken edges
    img = np.dstack([rgb, alpha]) * 255
    return np.clip(np.round(img), 0, 255).astype(np.uint8)


def rvmat(emissive):
    return ("ambient[]={0,0,0,0.1};\n"
            "diffuse[]={0,0,0,0};\n"
            "forcedDiffuse[]={0,0,0,0};\n"
            "emmisive[]={%g,%g,%g,2};\n"
            "specular[]={0,0,0,1};\n"
            "specularPower=1;\n"
            "renderFlags[]=\n{\n\t\"NoZWrite\",\n\t\"AddBlend\"\n};\n"
            "PixelShaderID=\"Normal\";\n"
            "VertexShaderID=\"Basic\";\n") % (emissive, emissive, emissive)


def to_paa(rgba, path, tmp):
    png = os.path.join(tmp, os.path.basename(path)[:-4] + ".png")
    Image.fromarray(rgba, "RGBA").save(png)
    if os.path.exists(path):
        os.remove(path)
    subprocess.run([IMAGE_TO_PAA, png, path], check=True, capture_output=True)


def build_all():
    os.makedirs(OUT, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        for name, palette, emissive in TRACERS:
            base = "geb_tracer_" + name
            to_paa(texture(palette), os.path.join(OUT, base + "_ca.paa"), tmp)
            with open(os.path.join(OUT, base + ".rvmat"), "w", encoding="ascii", newline="\r\n") as f:
                f.write(rvmat(emissive))
            lod = bolt_lod(GAME_DIR + base + "_ca.paa", GAME_DIR + base + ".rvmat")
            mlod.write(dict(version=257, lods=[lod]), os.path.join(OUT, base + ".p3d"))
            for ext in ("_ca.paa", ".rvmat", ".p3d"):
                print("wrote data/weapons/" + base + ext)


if __name__ == "__main__":
    build_all()
