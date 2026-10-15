r"""Green alien blood: write graphics/particles/geb_alien_blood_*.ptc from vanilla's red blood particles.

    python tools/build_alien_blood.py [--graphics P:\graphics]

Reads vanilla's hit_meatbones_ent_01 / hit_meatbones_ext_01 (bullet and melee hits on flesh) and
blood_surface_drops / blood_surface_chunks (the blood thrown onto the ground) from the P: drive extract and
writes copies where:
- every red colour key turns green (same brightness curve over the particle's life),
- the animated squirt sprites, which have the red baked into the texture, become vanilla's neutral liquid
  sprite (fluid_thick_01, the one vomit uses), so the green tint shows,
- the ground drops' material, which adds red through its ambient colour, becomes the white blood splat
  sprite (blood_01).
Everything else (sizes, timings, debris) stays as vanilla has it. scripts/3_game/effects/geb_AlienBlood.c
plays these instead of the red ones when the thing hit is an alien.
"""
import argparse
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(REPO, "graphics", "particles")

FILES = {
    "impacts/hit_meatbones_ent_01.ptc": "geb_alien_blood_enter.ptc",
    "impacts/hit_meatbones_ext_01.ptc": "geb_alien_blood_exit.ptc",
    "blood_surface_drops.ptc": "geb_alien_blood_drops.ptc",
    "blood_surface_chunks.ptc": "geb_alien_blood_chunks.ptc",
}
FLUID = '"{2884C61C9A35471E}Graphics/Particles/materials/character/fluid_thick_01.emat"'
SPLAT = '"{5A939336004F3C25}Graphics/Particles/materials/character/blood_01.emat"'
MATERIAL_SWAP = {
    "blood_anim_01.emat": FLUID,
    "blood_anim_02.emat": FLUID,
    "blood_anim_05.emat": FLUID,
    "blood_red_01.emat": SPLAT,
}


BAKED = ("blood_anim_01.emat", "blood_anim_02.emat", "blood_anim_05.emat")
BAKED_RED = 0.41 / 0.6        # the baked sprites' red level over the liquid sprite's grey level


def to_green(v):
    return [min(1.0, 0.6 * v), min(1.0, 2.6 * v), min(1.0, 0.25 * v)]


def recolour(colour_text, baked):
    """Colour keys (time r g b ...): red keys turn green. In an emitter whose red came from a baked sprite
    every key does, at the brightness the red sprite gave; other keys (grey mist, dirt) stay as they are."""
    nums = [float(x) for x in colour_text.split()]
    out = []
    for i in range(0, len(nums) - len(nums) % 4, 4):
        t, r, g, b = nums[i:i + 4]
        if baked:
            out += [t] + to_green(max(r, g, b) * BAKED_RED)
        elif r > 0.004 and r >= 1.5 * max(g, b):
            out += [t] + to_green(r)
        else:
            out += [t, r, g, b]
    return " ".join(("%.4f" % x).rstrip("0").rstrip(".") if x else "0" for x in out)


def convert(text):
    def emitter(m):
        block = m.group(0)
        mat = re.search(r'Material "([^"]*)"', block)
        name = mat.group(1).rsplit("/", 1)[-1] if mat else ""
        baked = name in BAKED
        if name in MATERIAL_SWAP:
            block = block.replace('"%s"' % mat.group(1), MATERIAL_SWAP[name])
        return re.sub(r"(Color \{\s*)([-0-9.\s]+?)(\s*\})",
                      lambda c: c.group(1) + recolour(c.group(2), baked) + c.group(3), block)
    # one EmitorDef per block: its body has no nested braces deeper than its Color/Alpha/Size curves
    return re.sub(r"EmitorDef \w+ \{(?:[^{}]|\{[^{}]*\})*\}", emitter, text)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--graphics", default=r"P:\graphics")
    a = ap.parse_args()
    for src, dst in FILES.items():
        text = open(os.path.join(a.graphics, "particles", src), encoding="utf-8").read()
        out = convert(text)
        with open(os.path.join(OUT, dst), "w", encoding="utf-8", newline="\n") as f:
            f.write(out)
        print("wrote", dst)


if __name__ == "__main__":
    main()
