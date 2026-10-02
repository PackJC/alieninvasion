# Turn Blender renders of the mod's models into the website's item pictures
# (docs/items/<classname>.webp), the same way the Gebsfish wiki gets its gear art.
#
#   0. Items on vanilla models (steak, hide, tanned leather, leather backpack, courier bag, hide backpack)
#      need vanilla's binarized p3d files (DZ\gear\food, DZ\gear\consumables, DZ\characters\backpacks),
#      which Blender can't import. Debinarize each with DeODOL6_2.exe <model.p3d> <out folder> (DeODOL53
#      crashes on most of them) and save the result as tools/debinned/_<name>.p3d, the names the manifest
#      uses. That folder is gitignored: those are Bohemia's models, so never commit them.
#   1. Render (Blender 4.x + the Arma 3 Object Builder add-on). render_items.py runs the gebsfish repo's
#      render_p3d.py after stripping the attachment proxies the vanilla backpacks carry:
#        blender --background --python tools/render_items.py -- \
#            --manifest tools/render_manifest.json --src data --texroot data --out <renders_dir>
#      Run it from this repo's root: the manifest's model paths are relative.
#   2. Crop and convert:
#        python tools/build_wiki_items.py <renders_dir>
#
# Renders are cropped to the subject and saved as WebP; the 1024px source PNGs are
# several MB each, which doesn't belong in a repo for a web page.

import glob
import os
import sys

from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS_IMG = os.path.join(REPO, "docs", "items")
MAX_W, MAX_H = 640, 420

# Degrees clockwise, applied after cropping. render_p3d.py's "roll" only works for side views,
# and the cartridge's three-quarter view stands it on end like a box rather than a magazine.
ROTATE = {"geb_PlasmaCartridge": 90}


def main():
    if len(sys.argv) < 2 or not os.path.isdir(sys.argv[1]):
        print("usage: python tools/build_wiki_items.py <renders_dir>")
        return 1
    os.makedirs(DOCS_IMG, exist_ok=True)

    written = 0
    for path in sorted(glob.glob(os.path.join(sys.argv[1], "*.png"))):
        cls = os.path.splitext(os.path.basename(path))[0]
        img = Image.open(path).convert("RGBA")
        bbox = img.getbbox()
        if bbox:
            img = img.crop(bbox)
        if cls in ROTATE:
            img = img.rotate(-ROTATE[cls], expand=True)
        img.thumbnail((MAX_W, MAX_H), Image.LANCZOS)
        out = os.path.join(DOCS_IMG, cls + ".webp")
        img.save(out, "WEBP", quality=86, method=6)
        print("%-24s %4dx%-4d %5.1f KB" % (cls, img.width, img.height, os.path.getsize(out) / 1024.0))
        written += 1

    total = sum(os.path.getsize(os.path.join(DOCS_IMG, f)) for f in os.listdir(DOCS_IMG))
    print("images written: %d (%.0f KB total in docs/items/)" % (written, total / 1024.0))
    return 0


if __name__ == "__main__":
    sys.exit(main())
