r"""Plasma rifle muzzle effects, one per cartridge colour (they replaced geb_plasma_shot.ptc).

    python tools/plasmashots/shots.py

Writes graphics/particles/geb_plasma_<id>.ptc for each variant below (registered in
scripts/3_game/particles/particlelist.c). data/config.cpp gives each cell its effect through CfgAmmo
muzzleFlashParticle, which beats the rifle's MuzzleFlash overrideParticle:
  ionburst  Plasma_Cell        geb_PlasmaCartridge        green
  arc       Plasma_Cell_Arc    geb_PlasmaCartridge_Arc    blue
  lance     Plasma_Cell_Lance  geb_PlasmaCartridge_Lance  lime
  void      Plasma_Cell_Void   geb_PlasmaCartridge_Void   violet
tools/plasmashots/preview.py plays them side by side with the original shot.

Every variant is built from the same parts, layered the way vanilla's weapon_shot_* effects are:
  core      white-hot glow1 at the muzzle, 2 sprites, ~0.06 s (the part you see in first person)
  halo      one wide faint glow1, ~0.1 s, kept faint and short so ADS isn't a screen-wide flash
  jet/lance glow1 streaks fired forward with heavy drag, VelAngle so they stretch along the shot
  crackle   electric.emat flickers re-emitted over ~0.15 s with a decaying BRateMast
  sparks    sparks_06 / sparks_04 / sparks1 streaks (VelAngle + LockedAxis like vanilla's sparks_lines)
  motes     tiny glow1 ions that drift and flicker (alpha curve wobbles) for up to a second
  vapor     faint steam_small_cook_01, the lit smoke vanilla muzzles use, tinted
Shock Lance adds "shock diamonds": glow1 sprites fired with VelocityRND 0 into AirResistance 40, so each stops
at velocity / 40 m from the muzzle (0.13, 0.27, 0.42 m) without relying on Offset's axis convention.

Conventions copied from vanilla weapon_shot_akm_01 / mosin9130_01: Angles 0 0 -90 points the emitter's +Y
along the barrel; LocalTransform 1 for what must stay on the muzzle (core, halo, crackle), 0 for what is left
in the air (sparks, motes, vapor); Restitution -1 means no collision. Colour curves are "t r g b", the others
"t v". Materials are vanilla, GUIDs as vanilla's own .ptc files reference them. Of the sprites, glow, spark,
sparks_06 and ring_02 are white or grey and take any tint; electric is pink-violet and sparks_04 orange, so
they lean the tint (good for violet / lime). Blue or green electric needs a tint with almost no red, or it
comes out magenta / pink.
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(REPO, "graphics", "particles")

GLOW = "{CB7AF4FD8ACBFDFC}Graphics/Particles/materials/glow/glow1.emat"
ELECTRIC = "{9833D8BE7A512C63}Graphics/Particles/materials/flash/electric.emat"
SPARK = "{009C2EBAACD2D72A}Graphics/Particles/materials/fire/sparks1.emat"
STREAK = "{A1C2A33044F4946D}Graphics/Particles/materials/fire/sparks_06.emat"       # 8 white streaks, TileU 8
STREAK_GOLD = "{0B22CDEBF6A47216}Graphics/Particles/materials/fire/sparks_04.emat"  # 16 orange streaks, TileU 16
RING = "{EA2686148E09D4DD}Graphics/Particles/materials/enviroment/ring_01.emat"     # ring_02 sprite, dim
SMOKE = "{8E25684854DAB008}Graphics/Particles/materials/smoke/steam_small_cook_01.emat"

# (key, default) in the order Workbench writes them; curves are lists of (t, value) / (t, (r, g, b))
FIELDS = [
    ("ShapeType", "Point"), ("ShapeSize", (0, 0, 0)), ("ConeAngle", (360, 0, 0)), ("LOD", 0), ("MaxNum", 1),
    ("BirthRate", 300), ("BirthRateRND", 0), ("BirthRateVel", 0), ("Offset", (0, 0, 0)), ("Angles", (0, 0, -90)),
    ("LocalTransform", 1), ("Material", None), ("CenterX", 0), ("CenterY", 0), ("ScaleX", 1), ("ScaleY", 1),
    ("SizeMultiplier", 0.1), ("SizeRND", 0), ("RandomRotDir", 0), ("RotMultiplier", 0), ("StretchMultiplier", 1),
    ("StreakFullUV", 0), ("VelAngle", 0), ("RandomAngle", 1), ("Sort", 0), ("BillboardingType", "Full"),
    ("Velocity", 0), ("VelocityRND", 0), ("VelAffect", 0), ("AirResistance", 0), ("AirResistanceRND", 0),
    ("GravityMultiply", 0), ("GravityMultiplyRND", 0), ("Restitution", -1), ("Spring", 0), ("Wind", 0),
    ("AnimOnce", 0), ("AnimFPS", 0), ("LifetimeByAnim", 0), ("RandomFrame", 0), ("LifeTime", 0.1),
    ("LifeTimeRND", 0), ("LifetimeShortening", 0),
    ("Color", [(0, (1, 1, 1)), (1, (1, 1, 1))]), ("Alpha", [(0, 1), (1, 0)]), ("RotationSpeed", [(0, 1)]),
    ("Size", [(0, 1)]),
    ("EffectTime", 0.02), ("Repeat", 0),
    ("ColorMast", [(0, (1, 1, 1))]), ("AlphaMast", [(0, 1)]), ("RotationSpeedMast", [(0, 1)]),
    ("SizeMast", [(0, 1)]), ("BRateMast", [(0, 1)]),
]
DEFAULTS = dict(FIELDS)


def _num(x):
    return "%g" % round(x, 5)


def _value(v):
    if isinstance(v, str):
        return v
    if isinstance(v, (tuple, list)):
        return " ".join(_num(x) for x in v)
    return _num(v)


def _curve(points):
    flat = []
    for t, v in points:
        flat.append(t)
        flat += list(v) if isinstance(v, (tuple, list)) else [v]
    return " ".join(_num(x) for x in flat)


def emitter(name, **kw):
    unknown = set(kw) - set(DEFAULTS)
    assert not unknown, unknown
    lines = ["EmitorDef %s {" % name]
    for key, default in FIELDS:
        v = kw.get(key, default)
        if key == "Material":
            lines.append(' Material "%s"' % v)
        elif isinstance(default, list):
            lines += [" %s {" % key, "  " + _curve(v), " }"]
        else:
            lines.append(" %s %s" % (key, _value(v)))
    return lines + ["}"]


def effect(emitters):
    out = ["EffectDef {", " Emitors {"]
    for e in emitters:
        out += ["  " + line for line in e]
    return "\n".join(out + [" }", "}"]) + "\n"


def mix(a, b, f):
    return tuple(x + (y - x) * f for x, y in zip(a, b))


# --------------------------------------------------------------------------------------------------------------
# parts
# --------------------------------------------------------------------------------------------------------------

def core(c, size=0.15, life=0.06):
    return emitter("core", Material=GLOW, MaxNum=2, SizeMultiplier=size, SizeRND=size * 0.3, LifeTime=life,
                   Color=[(0, (1, 1, 1)), (0.3, c["hot"]), (1, c["main"])], Alpha=[(0, 1), (0.5, 0.85), (1, 0)],
                   Size=[(0, 0.6), (0.12, 1), (1, 0.35)], EffectTime=0.01)


def halo(c, size=0.4, alpha=0.3, life=0.08):
    return emitter("halo", Material=GLOW, SizeMultiplier=size, SizeRND=size * 0.25, LifeTime=life,
                   Color=[(0, c["main"]), (1, c["deep"])], Alpha=[(0, alpha), (1, 0)],
                   Size=[(0, 0.5), (0.25, 1), (1, 1.15)], EffectTime=0.01)


def jet(c, name="jet", count=6, speed=20, speed_rnd=8, drag=16, size=0.1, scale=(0.45, 1.8), life=0.08):
    return emitter(name, Material=GLOW, ConeAngle=(360, 0, 1.5), MaxNum=count, BirthRate=count * 100,
                   SizeMultiplier=size, SizeRND=size * 0.4, ScaleX=scale[0], ScaleY=scale[1], VelAngle=1,
                   RandomAngle=0, Velocity=speed, VelocityRND=speed_rnd, AirResistance=drag, LifeTime=life,
                   LifeTimeRND=life * 0.5, Color=[(0, c["hot"]), (0.5, c["main"]), (1, c["deep"])],
                   Alpha=[(0, 1), (0.6, 0.7), (1, 0)], Size=[(0, 0.5), (0.2, 1), (1, 0.6)], EffectTime=0.01)


def crackle(c, name="crackle", tint=None, tint_end=None, count=3, rate=50, size=0.2, life=0.03, time=0.14,
            alpha=1.0):
    return emitter(name, Material=ELECTRIC, ShapeType="Sphere", ShapeSize=(0.025, 0, 0), ConeAngle=(360, 360, 0),
                   MaxNum=count, BirthRate=rate, BirthRateRND=rate * 0.6, SizeMultiplier=size, SizeRND=size * 0.7,
                   LifeTime=life, LifeTimeRND=life * 0.8,
                   Color=[(0, tint or c["hot"]), (1, tint_end or c["main"])],
                   Alpha=[(0, alpha), (1, alpha * 0.2)], EffectTime=time, BRateMast=[(0, 1), (0.5, 0.6), (1, 0)])


def sparks(c, material=STREAK, tiles=8, count=26, cone=50, speed=4, speed_rnd=7, drag=3, gravity=0.5,
           size=0.08, scale=None, life=0.18, life_rnd=0.25, bounce=-1, start=None):
    sx, sy = scale or (1.0 / tiles, 1)
    return emitter("sparks", Material=material, ConeAngle=(360, 0, cone), MaxNum=count, BirthRate=count * 50,
                   LocalTransform=0, ScaleX=sx, ScaleY=sy, SizeMultiplier=size, SizeRND=size * 0.7,
                   StretchMultiplier=1.01, VelAngle=1, RandomAngle=0, BillboardingType="LockedAxis",
                   Velocity=speed, VelocityRND=speed_rnd, AirResistance=drag, GravityMultiply=gravity,
                   GravityMultiplyRND=gravity * 0.4, Restitution=bounce, RandomFrame=1 if tiles > 1 else 0,
                   LifeTime=life, LifeTimeRND=life_rnd,
                   Color=[(0, start or c["hot"]), (0.3, c["main"]), (1, c["deep"])],
                   Alpha=[(0, 1), (0.75, 0.8), (1, 0)], Size=[(0, 1), (1, 0.5)])


def motes(c, count=16, speed=0.6, speed_rnd=2.2, gravity=-0.05, life=0.45):
    return emitter("motes", Material=GLOW, ShapeType="Sphere", ShapeSize=(0.03, 0, 0), ConeAngle=(360, 0, 75),
                   MaxNum=count, BirthRate=count * 50, LocalTransform=0, SizeMultiplier=0.014, SizeRND=0.016,
                   Velocity=speed, VelocityRND=speed_rnd, AirResistance=3, GravityMultiply=gravity, Wind=0.4,
                   LifeTime=life, LifeTimeRND=life * 1.5, Color=[(0, c["hot"]), (0.2, c["main"]), (1, c["deep"])],
                   Alpha=[(0, 0), (0.04, 1), (0.18, 0.35), (0.3, 0.9), (0.46, 0.3), (0.62, 0.75), (0.8, 0.2),
                          (1, 0)])


def vapor(colour, alpha=0.14, size=0.1, count=4):
    return emitter("vapor", Material=SMOKE, ConeAngle=(360, 0, 20), MaxNum=count, LocalTransform=0,
                   SizeMultiplier=size, SizeRND=size * 0.8, RandomRotDir=1, RotMultiplier=25, Velocity=1.2,
                   VelocityRND=2.5, AirResistance=4.5, GravityMultiply=-0.06, Wind=1, LifeTime=1.1, LifeTimeRND=0.7,
                   Color=[(0, colour), (1, mix(colour, (0.45, 0.45, 0.45), 0.6))],
                   Alpha=[(0, 0), (0.08, alpha), (1, 0)], Size=[(0, 0.35), (1, 1.9)], EffectTime=0.015)


def ring(c, name="ring", size=0.6, life=0.11, speed=0, drag=0):
    return emitter(name, Material=RING, SizeMultiplier=size, SizeRND=size * 0.15, Velocity=speed,
                   AirResistance=drag, LifeTime=life, Color=[(0, mix(c["hot"], (1, 1, 1), 0.5)), (1, c["main"])],
                   Alpha=[(0, 0), (0.1, 1), (1, 0)] if speed else [(0, 1), (1, 0)], Size=[(0, 0.08), (1, 1)],
                   EffectTime=0.01)


def diamond(c, i, distance, size):
    return emitter("diamond%d" % i, Material=GLOW, ConeAngle=(360, 0, 0), SizeMultiplier=size, RandomAngle=0,
                   Velocity=distance * 40, AirResistance=40, LifeTime=0.1, Color=[(0, c["hot"]), (1, c["main"])],
                   Alpha=[(0, 0), (0.12, 1), (0.55, 0.8), (1, 0)], Size=[(0, 0.4), (0.3, 1), (1, 0.7)],
                   EffectTime=0.01)


# --------------------------------------------------------------------------------------------------------------
# variants
# --------------------------------------------------------------------------------------------------------------

ION = {"hot": (0.85, 1, 0.85), "main": (0.25, 1, 0.35), "deep": (0.02, 0.35, 0.08)}
ARC = {"hot": (0.85, 0.95, 1), "main": (0.3, 0.7, 1), "deep": (0.05, 0.15, 0.6)}
LANCE = {"hot": (1, 1, 0.75), "main": (0.65, 1, 0.1), "deep": (0.2, 0.45, 0)}
VOID = {"hot": (1, 0.85, 1), "main": (0.7, 0.25, 1), "deep": (0.25, 0.02, 0.45)}


def ion_burst():
    c = ION
    return [core(c), halo(c), jet(c), crackle(c, tint=(0.15, 1, 0.3), tint_end=(0.08, 1, 0.2)), sparks(c), motes(c),
            vapor((0.6, 0.8, 0.65))]


def arc_discharge():
    c = ARC
    # electric.edds is pink-violet: any red in the tint turns these magenta, so the arcs get a red-free blue
    arcs = crackle(c, "arcs", tint=(0.1, 0.85, 1), tint_end=(0.04, 0.5, 1), count=4, rate=80, size=0.26,
                   life=0.028, time=0.2)
    wide = crackle(c, "arcs_wide", tint=(0.06, 0.6, 1), tint_end=(0.03, 0.4, 1), count=2, rate=40, size=0.55,
                   life=0.04, time=0.08, alpha=0.7)
    static = emitter("static", Material=ELECTRIC, ShapeType="Sphere", ShapeSize=(0.03, 0, 0),
                     ConeAngle=(360, 360, 0), BirthRate=14, BirthRateRND=10, SizeMultiplier=0.07, SizeRND=0.06,
                     LifeTime=0.02, LifeTimeRND=0.02, Color=[(0, (0.05, 0.6, 1))], Alpha=[(0, 1), (1, 0)],
                     EffectTime=0.5, BRateMast=[(0, 1), (1, 0)])
    return [core(c, size=0.12, life=0.045), halo(c, size=0.35, life=0.07), arcs, wide,
            jet(c, count=4, speed=14, speed_rnd=6, drag=22, size=0.08, scale=(0.4, 1.6), life=0.06),
            sparks(c, material=SPARK, tiles=1, count=45, cone=85, speed=3, speed_rnd=6, drag=1.2, gravity=1,
                   size=0.028, scale=(0.35, 1.4), life=0.35, life_rnd=0.55, bounce=0.35),
            static, vapor((0.6, 0.68, 0.78), alpha=0.1)]


def shock_lance():
    c = LANCE
    lance = jet(c, "lance", count=10, speed=55, speed_rnd=20, drag=22, size=0.09, scale=(0.3, 3.2), life=0.07)
    embers = emitter("embers", Material=GLOW, ConeAngle=(360, 0, 60), MaxNum=10, BirthRate=600, LocalTransform=0,
                     SizeMultiplier=0.012, SizeRND=0.01, Velocity=0.8, VelocityRND=1.6, AirResistance=1.5,
                     GravityMultiply=0.9, LifeTime=0.5, LifeTimeRND=0.4,
                     Color=[(0, c["hot"]), (0.3, c["main"]), (1, c["deep"])], Alpha=[(0, 1), (0.7, 0.8), (1, 0)])
    return [core(c, size=0.18), halo(c, size=0.45), lance,
            diamond(c, 1, 0.13, 0.085), diamond(c, 2, 0.27, 0.07), diamond(c, 3, 0.42, 0.055),
            ring(c), ring(c, "ring_wave", size=0.4, life=0.15, speed=9, drag=18),
            sparks(c, material=STREAK_GOLD, tiles=16, count=22, cone=22, speed=9, speed_rnd=9, drag=3.5,
                   gravity=0.35, size=0.16, life=0.12, life_rnd=0.14, start=(1, 1, 0.6)),
            embers, vapor((0.7, 0.78, 0.6), alpha=0.12)]


def void_plasma():
    c = VOID
    vortex = emitter("vortex", Material=ELECTRIC, MaxNum=2, BirthRate=30, SizeMultiplier=0.32, SizeRND=0.1,
                     RandomRotDir=1, RotMultiplier=900, LifeTime=0.12, LifeTimeRND=0.04,
                     Color=[(0, (1, 0.75, 1)), (1, c["main"])], Alpha=[(0, 1), (0.6, 0.6), (1, 0)],
                     RotationSpeed=[(0, 1), (1, 0.3)], Size=[(0, 0.5), (0.3, 1), (1, 1.3)], EffectTime=0.06)
    smoke = emitter("void_smoke", Material=SMOKE, ConeAngle=(360, 0, 30), MaxNum=5, LocalTransform=0,
                    SizeMultiplier=0.14, SizeRND=0.08, RandomRotDir=1, RotMultiplier=30, Velocity=1,
                    VelocityRND=2, AirResistance=5, GravityMultiply=-0.04, Wind=1, LifeTime=0.9, LifeTimeRND=0.5,
                    Color=[(0, (0.35, 0.18, 0.45)), (1, (0.25, 0.2, 0.3))], Alpha=[(0, 0), (0.08, 0.3), (1, 0)],
                    Size=[(0, 0.4), (1, 2)], EffectTime=0.015)
    floaty = sparks(c, material=SPARK, tiles=1, count=30, cone=70, speed=2.5, speed_rnd=4.5, drag=4,
                    gravity=-0.12, size=0.022, scale=(0.4, 1.3), life=0.5, life_rnd=0.6)
    return [core(c, size=0.14), halo(c, size=0.45, alpha=0.32, life=0.12), vortex, crackle(c, tint=(1, 0.8, 1)),
            ring(c, size=0.75, life=0.18), floaty, smoke]


VARIANTS = [
    {"id": "ionburst", "cartridge": "geb_PlasmaCartridge", "name": "Ion Burst", "colour": ION, "build": ion_burst,
     "desc": "The original look done properly: a white-hot core, a short forward jet, green crackle, "
             "streak sparks and ions that hang in the air for a second."},
    {"id": "arc", "cartridge": "geb_PlasmaCartridge_Arc", "name": "Arc Discharge", "colour": ARC,
     "build": arc_discharge,
     "desc": "Electric blue. Lightning flickers on the muzzle for a fifth of a second, sparks fall and bounce, "
             "and small static zaps keep going after the shot."},
    {"id": "lance", "cartridge": "geb_PlasmaCartridge_Lance", "name": "Shock Lance", "colour": LANCE,
     "build": shock_lance,
     "desc": "Acid lime and aggressive: a two-metre lance, three shock diamonds, a ring pushed forward off "
             "the muzzle and molten embers dripping. Fires the yellow bolt."},
    {"id": "void", "cartridge": "geb_PlasmaCartridge_Void", "name": "Void Plasma", "colour": VOID,
     "build": void_plasma,
     "desc": "Violet. A spinning arc vortex inside a dark smoke puff, a wide ring and slow sparks that float "
             "upward instead of falling. Fires a violet bolt."},
]


def build_all():
    for v in VARIANTS:
        path = os.path.join(OUT, "geb_plasma_%s.ptc" % v["id"])
        with open(path, "w", encoding="ascii", newline="\n") as f:
            f.write(effect(v["build"]()))
        print("wrote graphics/particles/geb_plasma_%s.ptc" % v["id"])


if __name__ == "__main__":
    build_all()
