r"""Specimen tank prop (geb_AlienTank): the model, its textures and materials, its particles and its hum.

    python tools/alientank/build.py [--skip-textures]

Writes data/tank/ (geb_alientank.p3d, *_co/_ca/_nohq/_smdi.paa, *.rvmat, geb_tank_hum.ogg) and
graphics/particles/geb_tank_*.ptc. Everything is generated here: the tank is built from lathed rings, boxes and
swept tubes (mesh.py), its textures are painted procedurally (paint.py), the alien inside is the mod's own
geb_GreenAlien.p3d posed asleep (specimen.py) and textured with its own greenalien_co.paa.

Layout (metres, model faces -Z like the alien, +Y up): a riveted round base 2 m across, a glass cylinder of
glowing green fluid 1.24 m across and 2 m tall with the alien floating in it, cables from the lid to its back,
a domed lid with a vent valve, three conduits down the sides and back, four struts, and a console with a
screen in front. Glass and fluid are ONE translucent surface (inside faces stored before outside ones, so the
far wall draws first from any side); everything inside is opaque, the bubbles and motes are particles.
Needs numpy, Pillow, DayZ Tools (ImageToPAA) and ffmpeg.
"""
import argparse
import os
import sys

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "alienrig"))
import mlod       # noqa: E402
import specimen   # noqa: E402
from mesh import (Mesh, bezier, box, convex_lod, cylinder, frame_to, lathe, lod, prism, rot_y,  # noqa: E402
                  smooth_path, tube)

REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(REPO, "data", "tank")
P3D = os.path.join(OUT, "geb_alientank.p3d")
TEX = "alieninvasion\\data\\tank\\"

R_GLASS = 0.62          # fluid/glass radius
Y_FLOOR = 0.58          # diffuser plate inside the glass
Y_CEIL = 2.76           # lid's underside inside the glass
LID = Y_CEIL - 2.62     # the lid's parts are laid out for a 2.62 m ceiling and moved up by this
CONSOLE_Z = -1.06       # back of the console (front of the base is at z -0.99)

MATERIALS = {
    "metal": (TEX + "geb_tank_metal_co.paa", TEX + "geb_tank_metal.rvmat"),
    "trim": (TEX + "geb_tank_trim_co.paa", TEX + "geb_tank_trim.rvmat"),
    "hazard": (TEX + "geb_tank_hazard_co.paa", TEX + "geb_tank_hazard.rvmat"),
    "hose": (TEX + "geb_tank_hose_co.paa", TEX + "geb_tank_hose.rvmat"),
    "grille": (TEX + "geb_tank_grille_co.paa", TEX + "geb_tank_grille.rvmat"),
    "glow": (TEX + "geb_tank_glow_co.paa", TEX + "geb_tank_glow.rvmat"),
    "screen": (TEX + "geb_tank_screen_co.paa", TEX + "geb_tank_screen.rvmat"),
    "label": (TEX + "geb_tank_label_co.paa", TEX + "geb_tank_label.rvmat"),
    "fluid": (TEX + "geb_tank_fluid_ca.paa", TEX + "geb_tank_fluid.rvmat"),
    "alien": ("alieninvasion\\data\\aliens\\greenalien_co.paa", "alieninvasion\\data\\aliens\\greenalien.rvmat"),
}

# metres per texture repeat (u, v)
T_METAL = (1.0, 1.0)
T_TRIM = (0.5, 0.5)
T_HAZARD = (0.8, 0.1)
T_HOSE = (0.3, 0.3)

# glow texture patches (u0, v0, u1, v1) in the glow atlas: paint.glow() paints the same layout
GLOW_GREEN = (0.02, 0.02, 0.48, 0.48)
GLOW_RED = (0.52, 0.02, 0.73, 0.23)
GLOW_AMBER = (0.77, 0.02, 0.98, 0.23)
GLOW_WHITE = (0.52, 0.27, 0.73, 0.48)


# --------------------------------------------------------------------------------------------------------------
# small helpers
# --------------------------------------------------------------------------------------------------------------

def patch_uv(m, rect):
    """Squash every UV of a mesh into one patch (for the glow atlas)."""
    u0, v0, u1, v1 = rect
    uc, vc = (u0 + u1) / 2, (v0 + v1) / 2
    m.F = [(i, n, tuple((uc + (u1 - u0) * 0.3 * np.tanh(a), vc + (v1 - v0) * 0.3 * np.tanh(b)) for a, b in uv),
            mat) for i, n, uv, mat in m.F]
    return m


def bolt_ring(m, r, y, count, size, mat, up=True, a0=0.0, sides=6):
    """Hex bolt heads standing on a flat ring (up=False: hanging under it)."""
    for k in range(count):
        a = a0 + 2 * np.pi * k / count
        head = lathe([(0, 0), (size, 0), (size, size * 0.55), (size * 0.7, size * 0.8), (0, size * 0.8)],
                     sides, mat, T_TRIM)
        M = rot_y(-a) if up else rot_y(-a) @ np.diag([1.0, -1.0, -1.0])
        m.extend(head, M, (r * np.cos(a), y, r * np.sin(a)))


def radial_bolts(m, r, y, count, size, mat, a0=0.0, sides=6):
    """Bolt heads on a vertical wall of radius r, pointing out."""
    for k in range(count):
        a = a0 + 2 * np.pi * k / count
        head = lathe([(0, 0), (size, 0), (size, size * 0.5), (size * 0.7, size * 0.75), (0, size * 0.75)],
                     sides, mat, T_TRIM)
        d = np.array([np.cos(a), 0, np.sin(a)])
        m.extend(head, frame_to(d), d * r + np.array([0, y, 0]))


# --------------------------------------------------------------------------------------------------------------
# the tank
# --------------------------------------------------------------------------------------------------------------

def tank(detail):
    """Visual mesh of the tank without the alien. detail 0 (closest) .. 3 (far, shadow)."""
    seg = [64, 40, 24, 14][detail]
    m = Mesh()

    # --- base: foot ring, panelled drum, ledge, hazard band, flange, glass collar
    m.extend(lathe([(0, 0.0), (0.97, 0.0), (0.99, 0.02), (0.99, 0.07), (0.93, 0.085)], seg, "trim", T_TRIM))
    m.extend(lathe([(0.93, 0.085), (0.93, 0.29)], seg, "metal", T_METAL))
    m.extend(lathe([(0.93, 0.29), (0.955, 0.30), (0.955, 0.325), (0.885, 0.335)], seg, "trim", T_TRIM))
    m.extend(lathe([(0.885, 0.335), (0.885, 0.435)], seg, "hazard", T_HAZARD))
    m.extend(lathe([(0.885, 0.435), (0.84, 0.47), (0.84, 0.485), (0.69, 0.485)], seg, "trim", T_TRIM))
    m.extend(lathe([(0.69, 0.485), (0.69, 0.55), (0.665, 0.58), (R_GLASS + 0.004, 0.58)], seg, "trim", T_TRIM,
                   smooth=(1,)))
    if detail <= 1:
        bolt_ring(m, 0.765, 0.485, [32, 20][detail], 0.016, "trim")
        radial_bolts(m, 0.93, 0.19, [48, 24][detail], 0.012, "trim", a0=np.pi / 48)

    # --- floor inside the glass: diffuser grille with a glowing ring around it
    m.extend(lathe([(R_GLASS + 0.004, Y_FLOOR), (0.53, Y_FLOOR)], seg, "trim", T_TRIM))
    m.extend(patch_uv(lathe([(0.53, Y_FLOOR), (0.47, Y_FLOOR)], seg, "glow"), GLOW_GREEN))
    m.extend(lathe([(0.47, Y_FLOOR), (0.0, Y_FLOOR)], seg, "grille", (0.4, 0.4)))

    # --- lid: collar, flange underside, drum, dome, hub, valve
    m.extend(lathe([(R_GLASS + 0.004, Y_CEIL), (0.665, Y_CEIL), (0.69, Y_CEIL + 0.03), (0.69, LID + 2.715),
                    (0.84, LID + 2.715), (0.84, LID + 2.73), (0.885, LID + 2.765)], seg, "trim", T_TRIM, smooth=(1,)))
    m.extend(lathe([(0.885, LID + 2.765), (0.885, LID + 2.95)], seg, "metal", T_METAL))
    m.extend(lathe([(0.885, LID + 2.95), (0.86, LID + 2.975), (0.74, LID + 3.07), (0.56, LID + 3.13), (0.34, LID + 3.16)], seg, "metal",
                   T_METAL, smooth=(1, 2, 3)))
    m.extend(lathe([(0.34, LID + 3.16), (0.30, LID + 3.17), (0.30, LID + 3.25), (0.0, LID + 3.25)], seg, "trim", T_TRIM))
    if detail <= 1:
        bolt_ring(m, 0.765, LID + 2.715, [32, 20][detail], 0.016, "trim", up=False)
        radial_bolts(m, 0.885, LID + 2.86, [48, 24][detail], 0.012, "trim", a0=np.pi / 48)
        bolt_ring(m, 0.32, LID + 3.25, 8, 0.014, "trim", a0=np.pi / 8)
    # ceiling inside the glass: light ring around a grille
    m.extend(lathe([(0.0, Y_CEIL), (0.40, Y_CEIL)], seg, "grille", (0.4, 0.4)))
    m.extend(patch_uv(lathe([(0.40, Y_CEIL), (0.50, Y_CEIL)], seg, "glow"), GLOW_GREEN))
    m.extend(lathe([(0.50, Y_CEIL), (R_GLASS + 0.004, Y_CEIL)], seg, "trim", T_TRIM))
    # vent valve on the hub: riser, elbow, outlet facing back-left
    vs = [10, 8, 6, 6][detail]
    valve_dir = np.array([np.cos(np.radians(135)), 0, np.sin(np.radians(135))])
    riser = smooth_path([(0.12, LID + 3.25, 0.12), (0.12, LID + 3.40, 0.12), (0.12, LID + 3.40, 0.12) + valve_dir * 0.16],
                        0.05, [6, 4, 2, 1][detail])
    m.extend(tube(riser, 0.035, vs, "trim", T_TRIM, caps=True))
    if detail <= 2:
        wheel = lathe([(0.0, 0.0), (0.06, 0.0), (0.06, 0.012), (0.0, 0.012)], [16, 12, 8, 6][detail], "hazard",
                      T_HAZARD)
        m.extend(wheel, None, (0.12, LID + 3.44, 0.12))
        m.extend(cylinder(0.012, LID + 3.40, LID + 3.445, 8, "trim", T_TRIM), None, (0.12, 0, 0.12))
    steam = np.array([0.12, LID + 3.40, 0.12]) + valve_dir * 0.17

    # --- struts between the flanges, on the back diagonals (front ones would cross the view of the alien)
    for k in range(2):
        a = np.pi / 4 + k * np.pi / 2
        m.extend(box((0.05, 2.23 + LID, 0.04), "trim", T_TRIM), rot_y(-a),
                 (0.76 * np.cos(a), 1.60 + LID / 2, 0.76 * np.sin(a)))
        if detail <= 1:
            for y in (0.78, 1.60 + LID / 2, 2.42 + LID):     # clamp pads on the strut
                m.extend(box((0.07, 0.06, 0.06), "trim", T_TRIM), rot_y(-a),
                         (0.76 * np.cos(a), y, 0.76 * np.sin(a)))

    # --- three conduits from the lid down into the base (left, right, back)
    hs = [12, 10, 8, 6][detail]
    for deg in (0, 180, 90):
        d = np.array([np.cos(np.radians(deg)), 0, np.sin(np.radians(deg))])
        path = smooth_path([d * 0.86 + [0, LID + 2.86, 0], d * 1.06 + [0, LID + 2.86, 0], d * 1.06 + [0, 0.19, 0],
                            d * 0.92 + [0, 0.19, 0]], 0.14, [8, 5, 3, 2][detail])
        m.extend(tube(path, 0.045, hs, "hose", T_HOSE))
        for end in (d * 0.905 + [0, LID + 2.86, 0], d * 0.945 + [0, 0.19, 0]):     # collars where they enter
            collar = cylinder(0.062, -0.04, 0.04, hs, "trim", T_TRIM)
            m.extend(collar, frame_to(d), end)

    # --- console in front: body, slanted top with the screen, label, buttons, lamps, conduit to the base
    cw, cz0, cz1 = 0.27, -1.36, CONSOLE_Z
    m.extend(box((2 * cw, 0.82, cz1 - cz0), "metal", T_METAL), None, (0, 0.41, (cz0 + cz1) / 2))
    top = Mesh()
    yf, yb = 0.82, 1.00                     # slanted top: low at the front, high at the back
    pts = {"fl": (-cw, yf, cz0), "fr": (cw, yf, cz0), "bl": (-cw, yb, cz1), "br": (cw, yb, cz1),
           "fl0": (-cw, 0.82, cz0), "fr0": (cw, 0.82, cz0), "bl0": (-cw, 0.82, cz1), "br0": (cw, 0.82, cz1)}
    P = {k: np.array(v, float) for k, v in pts.items()}
    slope_n = np.cross(P["br"] - P["bl"], P["fl"] - P["bl"])
    slope_n /= np.linalg.norm(slope_n)
    if slope_n[1] < 0:
        slope_n = -slope_n
    top.poly([P["fl"], P["fr"], P["br"], P["bl"]], slope_n,
             [(cw, 0.0), (-cw, 0.0), (-cw, -0.34), (cw, -0.34)], "trim")
    top.poly([P["bl0"], P["br0"], P["br"], P["bl"]], (0, 0, 1),
             [(cw, 0.82), (-cw, 0.82), (-cw, 0.64), (cw, 0.64)], "metal")
    for sgn, a, b, c in ((-1, "fl0", "bl0", "bl"), (1, "br0", "fr0", "br")):
        tri = [P[a], P[b], P[c]]
        top.poly(tri, (sgn, 0, 0), [(p[2], -p[1]) for p in tri], "metal")
    m.extend(top)
    # screen: a quad lifted 6 mm off the slope, upper part of the panel
    up_slope = (P["bl"] - P["fl"]) / np.linalg.norm(P["bl"] - P["fl"])
    F0 = (P["fl"] + P["fr"]) / 2                 # front edge, centre
    sc = F0 + up_slope * 0.21 + slope_n * 0.006
    sx, sy = 0.21, 0.13
    corners = [sc - [sx, 0, 0] - up_slope * sy, sc + [sx, 0, 0] - up_slope * sy, sc + [sx, 0, 0] + up_slope * sy,
               sc - [sx, 0, 0] + up_slope * sy]
    m.poly(corners, slope_n, [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)], "screen")
    # bezel around the screen
    for (cx, cy, w, h) in ((0, sy + 0.012, sx + 0.024, 0.012), (0, -sy - 0.012, sx + 0.024, 0.012),
                           (sx + 0.012, 0, 0.012, sy), (-sx - 0.012, 0, 0.012, sy)):
        c = sc + np.array([cx, 0, 0]) + up_slope * cy - slope_n * 0.002
        Mb = np.column_stack([[1, 0, 0], slope_n, np.cross([1, 0, 0], slope_n)])
        m.extend(box((2 * w, 0.012, 2 * h), "trim", T_TRIM), Mb, c)
    if detail <= 2:
        # buttons and switches below the screen
        for i, (bx, rect) in enumerate(((-0.17, GLOW_RED), (-0.10, GLOW_AMBER), (-0.03, GLOW_GREEN),
                                        (0.04, GLOW_GREEN))):
            c = F0 + up_slope * 0.045 + slope_n * 0.008 + np.array([bx, 0, 0])
            Mb = np.column_stack([[1, 0, 0], slope_n, np.cross([1, 0, 0], slope_n)])
            m.extend(patch_uv(box((0.04, 0.016, 0.03), "glow"), rect), Mb, c)
        for i in range(3):          # toggle switches
            c = F0 + up_slope * 0.05 + slope_n * 0.012 + np.array([0.12 + i * 0.045, 0, 0])
            Mb = np.column_stack([[1, 0, 0], slope_n, np.cross([1, 0, 0], slope_n)])
            m.extend(box((0.012, 0.035, 0.012), "trim", T_TRIM), Mb @ np.array(
                [[1, 0, 0], [0, np.cos(0.4), -np.sin(0.4)], [0, np.sin(0.4), np.cos(0.4)]]), c)
    # label plate on the console front
    lab = Mesh()
    lab.poly([(-0.22, 0.44, cz0 - 0.004), (0.22, 0.44, cz0 - 0.004), (0.22, 0.70, cz0 - 0.004),
              (-0.22, 0.70, cz0 - 0.004)], (0, 0, -1), [(0, 1), (1, 1), (1, 0), (0, 0)], "label")
    m.extend(lab)
    # vent slots on the console front (dark recesses via a grille strip) and lamps on the base front
    vent = Mesh()
    vent.poly([(-0.20, 0.12, cz0 - 0.003), (0.20, 0.12, cz0 - 0.003), (0.20, 0.30, cz0 - 0.003),
               (-0.20, 0.30, cz0 - 0.003)], (0, 0, -1), [(0, 0.45), (1.0, 0.45), (1.0, 0), (0, 0)], "grille")
    m.extend(vent)
    if detail <= 2:
        # status lamps on the lower drum, either side of the console, in gaps of the bolt row
        for deg, rect in ((30, GLOW_GREEN), (-30, GLOW_RED)):
            lamp = patch_uv(lathe([(0, 0), (0.028, 0), (0.028, 0.012), (0.02, 0.02), (0, 0.022)],
                                  [12, 10, 8, 6][detail], "glow", smooth=(2, 3)), rect)
            ang = np.radians(deg)
            pos = np.array([0.93 * np.sin(ang), 0.19, -0.93 * np.cos(ang)])
            m.extend(lamp, frame_to(pos * [1, 0, 1]), pos)
    # conduit from the console's back into the base
    path = smooth_path([(0, 0.55, cz1), (0, 0.55, -0.98), (0, 0.22, -0.98), (0, 0.22, -0.93)], 0.06,
                       [5, 3, 2, 1][detail])
    m.extend(tube(path + [0, 0, -0.03], 0.035, hs, "hose", T_HOSE))

    mid = (Y_FLOOR + Y_CEIL) / 2
    # the light sits in the fluid in front of the alien (inside its body it would only light the inside)
    mem = {"light": (0, Y_FLOOR + 0.45, -0.40), "bubbles": (0, Y_FLOOR + 0.02, 0), "motes": (0, mid, 0),
           "steam": tuple(steam), "screen": tuple(sc), "hum": (0, 0.30, 0)}
    return m, mem


def fluid(detail):
    """The glass and the green fluid in it, one translucent surface: inside faces first, then outside."""
    seg = [64, 40, 24, 14][detail]
    h = Y_CEIL - Y_FLOOR
    circ = 2 * np.pi * R_GLASS
    tile = (circ / 4, h / 0.99)                          # 4 repeats round, the texture's height once
    prof = [(R_GLASS, Y_FLOOR), (R_GLASS, Y_CEIL)]
    m = Mesh()
    m.extend(lathe(prof, seg, "fluid", tile, v0=0.005 * tile[1], inward=True))
    m.extend(lathe(prof, seg, "fluid", tile, v0=0.005 * tile[1]))
    return m


def alien_mesh(L, offset):
    """A posed alien LOD as tank faces (outward CCW, per-corner normals and UVs)."""
    m = Mesh()
    P = L["pts"] + offset
    for idx, uv in zip(L["faces"], L["uvs"]):
        cs = list(zip(idx, uv))[::-1]                     # MLOD order -> outward CCW
        m.face([P[i] for i, _ in cs], [L["normals"][i] for i, _ in cs], [tuple(t) for _, t in cs], "alien")
    return m


def cables(alien_pts, detail):
    """Two cables from the lid into the alien's upper back, sagging a little."""
    m = Mesh()
    P = alien_pts
    top = P[:, 1].max()
    band = P[(P[:, 1] > top - 0.62) & (P[:, 1] < top - 0.50) & (np.abs(P[:, 0]) < 0.09)]
    back = band[np.argmax(band[:, 2])]
    n = [16, 10, 6][detail]
    for sx in (-1, 1):
        end = back + np.array([sx * 0.045, 0, -0.012])
        start = np.array([sx * 0.16, Y_CEIL, 0.30])
        path = bezier(start, start + [0, -0.45, 0.06], end + [0, 0.18, 0.22], end, n)
        m.extend(tube(path, 0.011, [8, 6, 5][detail], "hose", T_HOSE))
        m.extend(cylinder(0.022, -0.01, 0.025, [10, 8, 6][detail], "trim", T_TRIM), frame_to(start - end),
                 end)                                     # socket on the skin
    return m


def build_model():
    lods_alien, _ = specimen.posed()
    base = lods_alien[0]["pts"]
    lo = base.min(0)
    hi = base.max(0)
    offset = np.array([-(lo[0] + hi[0]) / 2, Y_FLOOR + 0.07 - lo[1], -(lo[2] + hi[2]) / 2])
    reach = np.sqrt(((base + offset)[:, [0, 2]] ** 2).sum(1)).max()
    print("alien: %.2f m tall, top at %.2f (lid %.2f), widest %.3f m from the axis (glass %.2f)"
          % (hi[1] - lo[1], hi[1] + offset[1], Y_CEIL, reach, R_GLASS))
    assert reach < R_GLASS - 0.02 and hi[1] + offset[1] < Y_CEIL - 0.03

    lods = []
    stats = []
    for detail in range(4):
        m, mem = tank(detail)
        m.extend(alien_mesh(lods_alien[detail], offset))
        if detail <= 2:
            m.extend(cables(lods_alien[0]["pts"] + offset, detail))
        m.extend(fluid(detail))                          # translucent last, inside faces first
        lods.append(lod([1.0, 2.0, 4.0, 8.0][detail], m, MATERIALS))
        stats.append(len(m.F))
    print("visual LOD faces:", stats)

    # Geometry / View Geometry: base, glass and lid, console (closed convex pieces, house properties)
    def pieces():
        return [prism(0.99, 0.0, 0.485, 16, "c0"), prism(0.90, 0.485, LID + 3.25, 16, "c1"),
                Mesh().extend(box((0.54, 1.0, 0.30), "c2"), None, (0, 0.5, -1.21))]
    house = {"autocenter": "0", "class": "house", "damage": "no", "map": "hide", "forcenoalpha": "1"}
    geo = convex_lod(1e13, pieces(), house)
    view = convex_lod(6e15, pieces())
    fire = convex_lod(7e15, [prism(0.99, 0.0, 0.485, 16, "c0"), prism(R_GLASS, 0.485, Y_CEIL, 16, "c1"),
                             prism(0.885, Y_CEIL, LID + 3.25, 16, "c2"),
                             Mesh().extend(box((0.54, 1.0, 0.30), "c3"), None, (0, 0.5, -1.21))],
                      materials=["dz\\data\\data\\penetration\\metalplate.rvmat",
                                 "dz\\data\\data\\penetration\\glass.rvmat",
                                 "dz\\data\\data\\penetration\\metalplate.rvmat",
                                 "dz\\data\\data\\penetration\\metalplate.rvmat"])
    _, mem = tank(0)
    memory = memory_lod(mem)
    model = dict(version=mlod.read(specimen.ALIEN)["version"], lods=lods + [geo, memory, view, fire])
    os.makedirs(OUT, exist_ok=True)
    mlod.write(model, P3D)
    print("wrote", os.path.relpath(P3D, REPO))
    return mem


def memory_lod(points):
    """Memory LOD: one named point per entry (no faces)."""
    names = list(points)
    L = lod(1e15, Mesh(), {}, {"lodnoshadow": "1"})
    L["pts"] = np.array([points[n] for n in names], float)
    L["pflags"] = np.zeros(len(names), np.uint32)
    L["taggs"] = [(1, n, bytes(np.eye(len(names), dtype=np.uint8)[i])) for i, n in enumerate(names)] + L["taggs"]
    return L


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--skip-textures", action="store_true")
    a = ap.parse_args()
    build_model()
    if not a.skip_textures:
        import paint
        paint.build_all(OUT)
    import effects
    effects.build_all(REPO, OUT)


if __name__ == "__main__":
    main()
