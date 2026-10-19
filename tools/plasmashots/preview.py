r"""Browser preview of the plasma muzzle effects: the original geb_plasma_shot next to shots.py's four colours.

    python tools/plasmashots/preview.py out.html

Reads the .ptc files as written (so the preview shows the files, not shots.py's intentions), resolves each
emitter's material to its vanilla .emat and sprite on the P: drive (the folder holding this repo, or
%P_DRIVE%), decodes the sprites with edds.py and bakes everything into one self-contained page
(preview_template.html). The page simulates every emitter in WebGL: birth rate and MaxNum, the cone, velocity
with exponential AirResistance, gravity, lifetime, the Color/Alpha/Size/RotationSpeed curves and their *Mast
curves over EffectTime, Full / LockedAxis / None billboards, VelAngle, tiles, additive or alpha blending, then
HDR bloom. Where the engine's meaning of a value isn't documented (ConeAngle's three numbers, how the *RND
spreads apply, StretchMultiplier) it follows the reading in the template's comments, so Workbench and the game
remain the final word.
"""
import base64
import io
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import edds    # noqa: E402
import shots   # noqa: E402

P_DRIVE = os.environ.get("P_DRIVE", os.path.normpath(os.path.join(shots.REPO, "..")))
TEMPLATE = os.path.join(HERE, "preview_template.html")

CURRENT = {"id": "shot", "name": "Original shot", "colour": {"main": (0.2, 1, 0.25)}, "current": True,
           "desc": "geb_plasma_shot, used before the four colours: one electric sprite thrown forward at up to "
                   "100 m/s, plus two clouds of electric sprites, the larger up to 7 m across."}


def parse_ptc(text):
    """EmitorDef blocks as dicts: scalars as floats, vectors as lists, curves as flat lists, strings as is."""
    emitters, stack, cur, curve = [], [], None, None
    for raw in text.splitlines():
        s = raw.strip()
        if not s:
            continue
        if s.endswith("{"):
            head = s[:-1].strip()
            if head.startswith("EmitorDef"):
                cur = {"name": head.split()[1]}
                emitters.append(cur)
            elif cur is not None:
                curve = head
                cur[curve] = []
            stack.append(head)
        elif s == "}":
            head = stack.pop()
            if head == curve:
                curve = None
            elif head.startswith("EmitorDef"):
                cur = None
        elif curve:
            cur[curve] += [float(x) for x in s.split()]
        elif cur is not None:
            key, _, val = s.partition(" ")
            val = val.strip()
            if val.startswith('"'):
                cur[key] = val.strip('"')
                continue
            try:
                nums = [float(x) for x in val.split()]
                cur[key] = nums if len(nums) > 1 else nums[0]
            except ValueError:
                cur[key] = val
    return emitters


def _resource(path):
    return os.path.join(P_DRIVE, re.sub(r"^\{[0-9A-Fa-f]+\}", "", path).replace("/", os.sep))


def read_material(path, sprites):
    text = open(_resource(path), encoding="latin-1").read()

    def vec(key, default):
        m = re.search(r'\b%s\s+"?(-?[\d.]+(?:\s+-?[\d.]+)*)"?' % key, text)
        return [float(x) for x in m.group(1).split()] if m else default

    albedo = re.search(r'AlbedoMap\s+"\{[0-9A-Fa-f]+\}([^"]+)"', text).group(1)
    blend = re.search(r"\bBlendMode\s+(\S+)", text)
    additive = bool(blend) and blend.group(1) in ("2", "Additive", "Additive_AlphaModulate")
    sprite = os.path.splitext(os.path.basename(albedo))[0].lower()
    if sprite not in sprites:
        img = edds.read(_resource(albedo)).convert("RGBA")
        if max(img.size) > 256:
            img.thumbnail((256, 256))
        buf = io.BytesIO()
        img.save(buf, "PNG", optimize=True)
        sprites[sprite] = "data:image/png;base64," + base64.b64encode(buf.getvalue()).decode("ascii")
    return {"sprite": sprite, "additive": additive,
            "gain": max(vec("Emissive", [0])[:3]) + 0.5 * max(vec("Color", [0])[:3]),
            "diffuse": max(vec("Diffuse", [0])[:3]), "ambient": max(vec("Ambient", [0])[:3]),
            "tiles": [int(vec("TileU", [1])[0]), int(vec("TileV", [1])[0])],
            "short": os.path.splitext(os.path.basename(path))[0]}


def css(rgb):
    return "rgb(%d %d %d)" % tuple(round(255 * min(1, c) ** (1 / 1.6)) for c in rgb)


def build(out):
    effects, materials, sprites = [], {}, {}
    for v in [CURRENT] + shots.VARIANTS:
        name = "geb_plasma_%s" % v["id"]
        text = open(os.path.join(shots.OUT, name + ".ptc"), encoding="ascii").read()
        emitters = parse_ptc(text)
        for e in emitters:
            if e["Material"] not in materials:
                materials[e["Material"]] = read_material(e["Material"], sprites)
        main = v["colour"]["main"]
        effects.append({"id": v["id"], "name": v["name"], "file": name, "desc": v["desc"],
                        "current": v.get("current", False), "cartridge": v.get("cartridge", ""),
                        "accent": css(main), "light": list(main), "emitters": emitters, "ptc": text})
    data = json.dumps({"effects": effects, "materials": materials, "sprites": sprites}, separators=(",", ":"))
    page = open(TEMPLATE, encoding="utf-8").read().replace("/*__DATA__*/null", data.replace("</", "<\\/"))
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write(page)
    print("wrote %s (%d effects, %d materials, %d sprites, %d KB)"
          % (out, len(effects), len(materials), len(sprites), len(page) // 1024))


if __name__ == "__main__":
    build(sys.argv[1])
