r"""Green bloody hands for gutting aliens: data/blood/geb_alien_bloodyhands_mc.paa + one material per survivor.

    python tools/build_alien_hands.py [--heads P:\DZ\characters\heads\data]

Vanilla bloodies a survivor's bare hands by swapping their body material for its "_bloodyhands" copy, which
lays dz\characters\heads\data\bloodyhands_mc.paa (dark red blood with alpha) over the hands. This writes a
green version of that mask, and for every vanilla *_bloodyhands.rvmat a copy named *_alienblood.rvmat in
data/blood/hands/ that uses the green mask instead; everything else in the material stays vanilla's.
scripts/4_world/plugins/geb_PluginLifespan.c picks the copy by that name after gutting an alien.
Needs DayZ Tools' ImageToPAA (set DAYZ_TOOLS if it isn't under G:\SteamLibrary).
"""
import argparse
import glob
import os
import re
import subprocess
import tempfile

import numpy as np
from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DAYZ_TOOLS = os.environ.get("DAYZ_TOOLS", r"G:\SteamLibrary\steamapps\common\DayZ Tools")
IMAGE_TO_PAA = os.path.join(DAYZ_TOOLS, "Bin", "ImageToPAA", "ImageToPAA.exe")
VANILLA_MASK = r"dz\characters\heads\data\bloodyhands_mc.paa"
GREEN_MASK = r"alieninvasion\data\blood\geb_alien_bloodyhands_mc.paa"


def green_mask(src_paa, dst_paa, tmp):
    png = os.path.join(tmp, "mask.png")
    subprocess.run([IMAGE_TO_PAA, src_paa, png], check=True, capture_output=True)
    a = np.asarray(Image.open(png).convert("RGBA")).astype(float)
    v = a[..., 0:3].max(-1)                      # the blood's brightness (it's all red)
    out = a.copy()
    out[..., 0] = np.clip(v * 0.30, 0, 255)
    out[..., 1] = np.clip(v * 1.15, 0, 255)
    out[..., 2] = np.clip(v * 0.12, 0, 255)
    new_png = os.path.join(tmp, "geb_alien_bloodyhands_mc.png")
    Image.fromarray(out.astype(np.uint8), "RGBA").save(new_png)
    if os.path.exists(dst_paa):
        os.remove(dst_paa)
    subprocess.run([IMAGE_TO_PAA, new_png, dst_paa], check=True, capture_output=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--heads", default=r"P:\DZ\characters\heads\data")
    a = ap.parse_args()
    out_dir = os.path.join(REPO, "data", "blood")
    os.makedirs(os.path.join(out_dir, "hands"), exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        green_mask(os.path.join(a.heads, "bloodyhands_mc.paa"), os.path.join(out_dir, "geb_alien_bloodyhands_mc.paa"), tmp)
    print("wrote", GREEN_MASK)
    n = 0
    for src in sorted(glob.glob(os.path.join(a.heads, "*", "*_bloodyhands.rvmat"))):
        text = open(src, encoding="latin-1").read()
        new, count = re.subn(re.escape(VANILLA_MASK), lambda m: GREEN_MASK, text, flags=re.I)
        if not count:
            print("  no blood mask in", src)
            continue
        name = os.path.basename(src).lower().replace("_bloodyhands", "_alienblood")
        with open(os.path.join(out_dir, "hands", name), "w", encoding="latin-1", newline="\r\n") as f:
            f.write(new)
        n += 1
    print("wrote %d hand materials in data/blood/hands" % n)


if __name__ == "__main__":
    main()
