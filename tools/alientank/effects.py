r"""Specimen tank materials (data/tank/*.rvmat), particles (graphics/particles/geb_tank_*.ptc) and the hum
(data/tank/geb_tank_hum.ogg).

Materials are vanilla Super-shader layouts. The glowing ones (lamps, light rings, screen, diffuser grille) use
emmisive's 4th value as the brightness, like vanilla's Christmas lights; emissive light is the texture colour
times emmisive, so the grille's dark metal stays dark and only its holes glow. The fluid is window glass
(Super, NoZWrite, glass fresnel and environment map) with a green, partly transparent _ca and a faint glow.

Particles (played by scripts/4_world/entities/building/geb_AlienTank.c at the model's memory points):
  geb_tank_bubbles  streams rising from the diffuser plus a fine fizz, vanilla bubbles_01_em sprites. They rise at
                    a steady speed (no drag, no gravity, like vanilla's rising dust): with breath_bubbles' buoyancy
                    and drag they only made ~0.2 m/s in game and died a third of the way up (Cole, 2026-10-15).
                    Speed x lifetime is set so the slowest get within ~10 cm of the lid (2.16 m above the floor)
                    and the fastest end inside its metal (under 2.79 m), never above it; narrow cones keep them
                    inside the glass.
  geb_tank_motes    slow green specks drifting in the fluid, vanilla glow1
  geb_tank_steam    wisps from the lid's vent valve, vanilla steam_small_cook_01
Emitter boxes are kept inside the glass whether DayZ reads ShapeSize as the full or the half size.

The hum is synthesised: a 120 Hz machine drone with harmonics, a faint whine and buzz and a slow swell, 6 s long
and seamless (every component repeats a whole number of times in 6 s; encoded at Vorbis q 10 and checked so the
loop point doesn't click). Needs ffmpeg.
"""
import os
import subprocess

import numpy as np

TEX = "alieninvasion\\data\\tank\\"
BUBBLES = "{3E3B5AA16932182E}Graphics/Particles/materials/enviroment/bubbles_01_em.emat"
GLOW = "{CB7AF4FD8ACBFDFC}Graphics/Particles/materials/glow/glow1.emat"
STEAM = "{8E25684854DAB008}Graphics/Particles/materials/smoke/steam_small_cook_01.emat"


# --------------------------------------------------------------------------------------------------------------
# materials
# --------------------------------------------------------------------------------------------------------------

def _stage(i, texture, extra=""):
    return ("class Stage%d\n{\n\ttexture=\"%s\";\n\tuvSource=\"tex\";\n%s\tclass uvTransform\n\t{\n"
            "\t\taside[]={1,0,0};\n\t\tup[]={0,1,0};\n\t\tdir[]={0,0,0};\n\t\tpos[]={0,0,0};\n\t};\n};\n"
            % (i, texture, extra))


def rvmat(nohq, smdi, emissive=(0, 0, 0, 1), specular=(0.2, 0.2, 0.2), power=60, fresnel="fresnel(2.0,0.5)",
          glass=False):
    v = lambda t: ",".join("%g" % x for x in t)          # noqa: E731
    s = "ambient[]={1,1,1,1};\ndiffuse[]={1,1,1,1};\nforcedDiffuse[]={0,0,0,%s};\n" % ("1" if glass else "0")
    s += "emmisive[]={%s};\nspecular[]={%s,1};\nspecularPower=%g;\n" % (v(emissive), v(specular), power)
    if glass:
        s += "renderFlags[]=\n{\n\t\"NoZWrite\"\n};\n"
    s += "PixelShaderID=\"Super\";\nVertexShaderID=\"Super\";\n"
    s += _stage(1, nohq)
    s += _stage(2, "#(argb,8,8,3)color(0.5,0.5,0.5,1,DT)")
    s += _stage(3, "#(argb,8,8,3)color(0,0,0,0,MC)")
    s += _stage(4, "#(argb,8,8,3)color(1,1,1,1,AS)")
    s += _stage(5, smdi)
    s += _stage(6, "#(ai,64,64,1)%s" % fresnel)
    s += _stage(7, "dz\\data\\data\\env_land_co.paa", "\tuseWorldEnvMap=\"true\";\n")
    return s


FLAT_NOHQ = "#(argb,8,8,3)color(0.5,0.5,1,1,NOHQ)"


def materials():
    return {
        "geb_tank_metal": rvmat(TEX + "geb_tank_metal_nohq.paa", TEX + "geb_tank_metal_smdi.paa",
                                specular=(0.35, 0.35, 0.33), power=60),
        "geb_tank_trim": rvmat(TEX + "geb_tank_trim_nohq.paa", TEX + "geb_tank_trim_smdi.paa",
                               specular=(0.7, 0.7, 0.7), power=90, fresnel="fresnel(3.3,1.2)"),
        "geb_tank_hazard": rvmat(TEX + "geb_tank_hazard_nohq.paa", TEX + "geb_tank_hazard_smdi.paa",
                                 specular=(0.3, 0.3, 0.3), power=50),
        "geb_tank_hose": rvmat(TEX + "geb_tank_hose_nohq.paa", TEX + "geb_tank_hose_smdi.paa",
                               specular=(0.15, 0.15, 0.15), power=25, fresnel="fresnel(1.5,0.3)"),
        "geb_tank_grille": rvmat(TEX + "geb_tank_grille_nohq.paa", TEX + "geb_tank_grille_smdi.paa",
                                 emissive=(1, 1, 1, 2.5), specular=(0.3, 0.3, 0.3), power=50),
        "geb_tank_glow": rvmat(FLAT_NOHQ, "#(argb,8,8,3)color(1,0.4,0.6,1,SMDI)", emissive=(1, 1, 1, 3.5),
                               specular=(0.4, 0.4, 0.4), power=80),
        "geb_tank_screen": rvmat(FLAT_NOHQ, "#(argb,8,8,3)color(1,0.8,0.9,1,SMDI)", emissive=(1, 1, 1, 3),
                                 specular=(0.6, 0.6, 0.6), power=400),
        "geb_tank_label": rvmat(FLAT_NOHQ, TEX + "geb_tank_label_smdi.paa", specular=(0.35, 0.35, 0.35), power=60),
        "geb_tank_fluid": rvmat(TEX + "geb_tank_fluid_nohq.paa", TEX + "geb_tank_fluid_smdi.paa",
                                emissive=(1, 1, 1, 0.45), specular=(1, 1, 1), power=800,
                                fresnel="fresnel(0.81,1.26)", glass=True),
    }


# --------------------------------------------------------------------------------------------------------------
# particles
# --------------------------------------------------------------------------------------------------------------

def curve(*pairs):
    return " ".join("%g" % x for p in pairs for x in (p if isinstance(p, tuple) else (p,)))


def emitter(name, material, shape="Box", size=(0.01, 0, 0.01), offset=(0, 0, 0), cone=(360, 0, 6), max_num=40,
            rate=6, rate_rnd=2, size_mult=0.01, size_rnd=0.0, velocity=0.2, velocity_rnd=0.05, air=3.0,
            gravity=-0.2, spring=0.0, life=2.5, life_rnd=0.5, color=((0, 0.7, 1, 0.75), (1, 0.7, 1, 0.75)),
            alpha=((0, 0), (0.1, 0.8), (0.85, 0.8), (1, 0)), size_curve=((0, 1), (1, 1)), rot=500):
    lines = [
        "EmitorDef %s {" % name,
        " ShapeType %s" % shape,
        " ShapeSize %g %g %g" % size,
        " ConeAngle %g %g %g" % cone,
        " MaxNum %d" % max_num,
        " BirthRate %g" % rate,
        " BirthRateRND %g" % rate_rnd,
        " BirthRateVel 0",
        " Offset %g %g %g" % offset,
        " LocalTransform 0",
        " Material \"%s\"" % material,
        " CenterX -0.167",
        " CenterY -0.213",
        " ScaleX 1",
        " ScaleY 1",
        " SizeMultiplier %g" % size_mult,
        " SizeRND %g" % size_rnd,
        " RandomRotDir 1",
        " RotMultiplier %g" % rot,
        " StreakFullUV 0",
        " VelAngle 0",
        " RandomAngle 1",
        " Sort 0",
        " Velocity %g" % velocity,
        " VelocityRND %g" % velocity_rnd,
        " VelAffect 4",
        " AirResistance %g" % air,
        " GravityMultiply %g" % gravity,
        " Spring %g" % spring,
        " Wind 0",
        " AnimOnce 0",
        " AnimFPS 30",
        " LifetimeByAnim 0",
        " LifeTime %g" % life,
        " LifeTimeRND %g" % life_rnd,
        " Color {",
        "  %s" % curve(*[(t, r, g, b) for t, r, g, b in color]),
        " }",
        " Alpha {",
        "  %s" % curve(*alpha),
        " }",
        " RotationSpeed {",
        "  0 1 1 0",
        " }",
        " Size {",
        "  %s" % curve(*size_curve),
        " }",
        " EffectTime 10",
        " Repeat 1",
        "}",
    ]
    return lines


def effect(*emitters):
    out = ["EffectDef {", " Emitors {"]
    for e in emitters:
        out += ["  " + line for line in e]
    out += [" }", "}"]
    return "\n".join(out) + "\n"


def particles():
    bubble_col = ((0, 0.95, 1.0, 0.97), (1, 1.0, 1.0, 1.0))
    streams = []
    for i, deg in enumerate((20, 92, 164, 236, 308)):
        a = np.radians(deg)
        streams.append(emitter(
            "stream%d" % (i + 1), BUBBLES, size=(0.012, 0, 0.012), offset=(0.30 * np.cos(a), 0, 0.30 * np.sin(a)),
            cone=(360, 0, 2), rate=5 + i % 3, rate_rnd=2, max_num=60, size_mult=0.016, size_rnd=0.006,
            velocity=0.5, velocity_rnd=0.04, air=0.0, gravity=0.0, spring=0.02, life=4.7, life_rnd=0.2,
            color=bubble_col, alpha=((0, 0), (0.04, 1.0), (0.94, 1.0), (1, 0)), size_curve=((0, 0.6), (1, 1.25))))
    streams.append(emitter(
        "centre", BUBBLES, size=(0.02, 0, 0.02), offset=(0.0, 0, 0.05), cone=(360, 0, 2), rate=9, rate_rnd=3,
        max_num=70, size_mult=0.02, size_rnd=0.008, velocity=0.5, velocity_rnd=0.04, air=0.0, gravity=0.0,
        spring=0.03, life=4.7, life_rnd=0.2, color=bubble_col, alpha=((0, 0), (0.04, 1.0), (0.94, 1.0), (1, 0)),
        size_curve=((0, 0.6), (1, 1.3))))
    fizz = emitter(
        "fizz", BUBBLES, size=(0.33, 0, 0.33), cone=(360, 0, 3), rate=14, rate_rnd=4, max_num=110,
        size_mult=0.007, size_rnd=0.004, velocity=0.42, velocity_rnd=0.04, air=0.0, gravity=0.0, spring=0.04,
        life=5.4, life_rnd=0.2, color=bubble_col, alpha=((0, 0), (0.05, 0.8), (0.93, 0.8), (1, 0)))
    motes = emitter(
        "motes", GLOW, size=(0.36, 0.9, 0.36), cone=(360, 0, 180), rate=7, rate_rnd=2, max_num=80, size_mult=0.022,
        size_rnd=0.012, velocity=0.03, velocity_rnd=0.02, air=0.6, gravity=-0.004, life=6.0, life_rnd=3.0,
        color=((0, 0.5, 1.0, 0.6), (0.5, 0.7, 1.0, 0.75), (1, 0.45, 1.0, 0.55)),
        alpha=((0, 0), (0.2, 1.0), (0.75, 1.0), (1, 0)), size_curve=((0, 0.5), (0.5, 1), (1, 0.6)), rot=20)
    steam = emitter(
        "wisps", STEAM, shape="Point", size=(0, 0, 0), cone=(360, 0, 15), rate=2, rate_rnd=1, max_num=20,
        size_mult=0.06, size_rnd=0.03, velocity=0.25, velocity_rnd=0.1, air=1.5, gravity=-0.03, life=2.2,
        life_rnd=0.8, color=((0, 0.75, 0.95, 0.78), (1, 0.6, 0.8, 0.65)),
        alpha=((0, 0), (0.15, 0.45), (1, 0)), size_curve=((0, 0.3), (1, 1.6)), rot=40)
    return {"geb_tank_bubbles": effect(*streams, fizz), "geb_tank_motes": effect(motes),
            "geb_tank_steam": effect(steam)}


# --------------------------------------------------------------------------------------------------------------
# hum
# --------------------------------------------------------------------------------------------------------------

def hum(path, seconds=6, sr=44100):
    n = seconds * sr
    t = np.arange(n) / sr
    x = np.zeros(n)
    # 120 Hz machine drone with strong 2nd-4th harmonics (the first version sat at 50/100 Hz, which many
    # headsets and speakers barely play: Cole heard nothing), a little 60 Hz body underneath
    for f, a in ((60, 0.30), (120, 0.55), (240, 0.50), (360, 0.34), (480, 0.22), (600, 0.10), (720, 0.08),
                 (960, 0.04)):
        x += a * np.sin(2 * np.pi * f * t + f * 0.37)
    # faint electrical whine round 2 kHz, wavering in pitch every 3 s
    x += 0.035 * np.sin(2 * np.pi * 1990 * t + 6.0 * np.sin(2 * np.pi * t / 3.0))
    # electrical buzz: noise band-passed round 300-900 Hz, periodic over the loop
    rng = np.random.default_rng(7)
    spec = np.fft.rfft(rng.standard_normal(n))
    freqs = np.fft.rfftfreq(n, 1 / sr)
    spec *= np.exp(-((freqs - 600) / 300) ** 2)
    buzz = np.fft.irfft(spec, n)
    buzz /= np.abs(buzz).max()
    x += 0.08 * buzz * (0.6 + 0.4 * np.sin(2 * np.pi * 120 * t) ** 2)
    x *= 0.85 + 0.15 * np.sin(2 * np.pi * t / 3.0)            # slow swell, two per loop
    x *= 0.7 / np.abs(x).max()
    tmp = path + ".f32"
    x.astype(np.float32).tofile(tmp)
    # Vorbis' coding error differs at the file's two ends, so the loop point clicks unless that error is tiny:
    # at q 4 the seam jumped ~60x more than the signal ever does between samples, at q 10 it doesn't (24 KB).
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-f", "f32le", "-ar", str(sr), "-ac", "1", "-i", tmp,
                    "-c:a", "libvorbis", "-q:a", "10", path], check=True)
    os.remove(tmp)
    y = np.frombuffer(subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-f", "f32le", "-"], capture_output=True,
                                     check=True).stdout, np.float32).astype(float)
    inside = np.percentile(np.abs(2 * y[1:-1] - y[:-2] - y[2:]), 99.9)
    seam = abs(2 * y[-1] - y[-2] - y[0])
    assert len(y) == n and seam < 2 * inside, ("hum loop seam", len(y), seam, inside)


def build_all(repo, out_dir):
    for name, text in materials().items():
        path = os.path.join(out_dir, name + ".rvmat")
        with open(path, "w", encoding="ascii", newline="\r\n") as f:
            f.write(text)
    print("wrote %d materials in data/tank" % len(materials()))
    pdir = os.path.join(repo, "graphics", "particles")
    for name, text in particles().items():
        with open(os.path.join(pdir, name + ".ptc"), "w", encoding="ascii", newline="\n") as f:
            f.write(text)
        print("wrote graphics/particles/%s.ptc" % name)
    hum(os.path.join(out_dir, "geb_tank_hum.ogg"))
    print("wrote data/tank/geb_tank_hum.ogg")
