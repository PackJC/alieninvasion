r"""Build the mod logo (data/logo.paa and data/logo_hover.paa, plus the README's .github/logo.png) from the
Gebsfish logo, in the same style.

    python tools/build_logo.py [--gebsfish P:\gebsfish\data] [--png <dir>]

Keeps the Gebsfish badge (black oval, moonlit sky, pine tree line, river) and its lettering (heavy italic,
white outline, black drop outline: Arial Black sheared like the Gebsfish wordmark), and swaps:
- the bird for a UFO with a faint light beam,
- "Gebsfish" for "Alien" / "Invasion" on two lines,
- the hover version's orange sunset for an alien lime-green glow (text stays Gebsfish's hover green) and its
  red frame for the mod's bright green one.
The normal version stays greyscale like Gebsfish's. Needs DayZ Tools' ImageToPAA (set DAYZ_TOOLS if it isn't
under G:\SteamLibrary) for the .paa <-> .png conversions. --png also writes the two PNGs for previews.
"""
import argparse
import os
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DAYZ_TOOLS = os.environ.get("DAYZ_TOOLS", r"G:\SteamLibrary\steamapps\common\DayZ Tools")
IMAGE_TO_PAA = os.path.join(DAYZ_TOOLS, "Bin", "ImageToPAA", "ImageToPAA.exe")
FONT = r"C:\Windows\Fonts\ariblk.ttf"
SHEAR = 0.22                                 # Arial Black leaned like the Gebsfish wordmark

# Measured on the 1024 x 1024 Gebsfish logo
TOP_CUT, BOT_CUT = 401, 582                  # rows between which its wordmark (with outlines) lies
OVAL = (495.6, 540.6, 326.7, 235.6)          # badge: centre x, y, semi-axes (fitted to the rim below the text)
BIRD = (398, 524, 328, 418)                  # box around the bird: x0, x1, y0, y1

UFO = (496, 326, 1.3)                        # centre x, y, scale
BEAM_TO = 420
LINES = [("Alien", 112, 496, 404), ("Invasion", 120, 496, 535)]   # text, cap height, centre x, top y
TEXT = {False: (88, 86, 88, 255), True: (38, 127, 0, 255)}         # Gebsfish's own fills
FRAME = {False: (0, 0, 0, 255), True: (77, 255, 0, 255)}


def paa_to_array(paa, tmp):
    png = os.path.join(tmp, os.path.basename(paa) + ".png")
    subprocess.run([IMAGE_TO_PAA, paa, png], check=True, capture_output=True)
    return np.asarray(Image.open(png).convert("RGBA")).astype(float)


def inpaint(img, mask, iters=150):
    """Fill masked pixels: row-wise interpolation between the nearest unmasked neighbours, then smoothing."""
    out = img.copy()
    xs = np.arange(mask.shape[1])
    for y in np.nonzero(mask.any(1))[0]:
        row = mask[y]
        for c in range(out.shape[2]):
            out[y, row, c] = np.interp(xs[row], xs[~row], out[y, ~row, c])
    for _ in range(iters):
        avg = (np.roll(out, 1, 0) + np.roll(out, -1, 0) + np.roll(out, 1, 1) + np.roll(out, -1, 1)) / 4
        out[mask] = avg[mask]
    return out


def rgb_to_hsv(rgb):
    r, g, b = rgb[..., 0], rgb[..., 1], rgb[..., 2]
    mx, mn = rgb.max(-1), rgb.min(-1)
    d = mx - mn
    h = np.zeros_like(mx)
    nz = d > 1e-9
    rm = nz & (mx == r)
    gm = nz & (mx == g) & ~rm
    bm = nz & ~rm & ~gm
    h[rm] = ((g - b)[rm] / d[rm]) % 6
    h[gm] = (b - r)[gm] / d[gm] + 2
    h[bm] = (r - g)[bm] / d[bm] + 4
    return h / 6, np.where(mx > 0, d / np.maximum(mx, 1e-9), 0), mx


def hsv_to_rgb(h, s, v):
    i = np.floor(h * 6).astype(int) % 6
    f = h * 6 - np.floor(h * 6)
    p, q, t = v * (1 - s), v * (1 - f * s), v * (1 - (1 - f) * s)
    out = np.zeros(h.shape + (3,))
    for k, c in enumerate(((v, t, p), (q, v, p), (p, v, t), (p, q, v), (t, p, v), (v, p, q))):
        out[i == k] = np.stack(c, -1)[i == k]
    return out


def sunset_to_alien_glow(a):
    """Gebsfish's orange/yellow sky and river -> lime green."""
    h, s, v = rgb_to_hsv(a[..., :3] / 255.0)
    warm = (s > 0.25) & ((h < 0.2) | (h > 0.95))
    h = np.where(warm, (h + 0.14) % 1.0, h)
    s = np.where(warm, s * 0.9, s)
    a = a.copy()
    a[..., :3] = hsv_to_rgb(h, s, v) * 255
    return a


def badge(src, hover):
    """The Gebsfish badge without its wordmark, frame and bird."""
    a = src
    H, W = a.shape[:2]
    yy, xx = np.mgrid[0:H, 0:W]
    cx, cy, ax, by = OVAL
    # Behind the text the badge is a little wider, so it meets the tree line at the cut (216..776 at row
    # 400) instead of leaving the overhanging trees with flat bottoms; "Invasion" covers where it rejoins
    # the real rim.
    band_oval = ((xx - cx) / (ax * 1.081)) ** 2 + ((yy - cy) / by) ** 2 <= 1.0
    out = a.copy()
    band = (yy >= TOP_CUT) & (yy <= BOT_CUT)
    out[band] = 0
    # The sky between the trees fades into the black forest floor over ~22 px, blurred sideways so it reads
    # as haze rather than stripes.
    edge = a[TOP_CUT - 1]
    k = 41
    pad = np.pad(edge[:, :3], ((k // 2, k // 2), (0, 0)), mode="edge")
    soft = np.stack([np.convolve(pad[:, c], np.ones(k) / k, mode="valid") for c in range(3)], -1)
    near = np.clip((yy - TOP_CUT) / 6.0, 0, 1)[..., None]
    fade = np.clip(1 - (yy - TOP_CUT) / 22.0, 0, 1)[..., None]
    fill = np.zeros_like(a)
    fill[..., :3] = (edge[None, :, :3] * (1 - near) + soft[None, :, :] * near) * fade
    fill[..., 3] = 255
    keep = band & band_oval
    out[keep] = fill[keep]
    out[:16, :] = out[-16:, :] = out[:, :16] = out[:, -16:] = 0          # old frame
    x0, x1, y0, y1 = BIRD
    m = np.zeros((H, W), bool)
    m[y0:y1, x0:x1] = out[y0:y1, x0:x1, :3].max(-1) < 70
    m = np.asarray(Image.fromarray(m.astype(np.uint8) * 255).filter(ImageFilter.MaxFilter(5))) > 0
    out = inpaint(out, m)
    return sunset_to_alien_glow(out) if hover else out


def beam(img, cx, top, bottom, w0, w1, hover):
    H, W = img.shape[:2]
    yy, xx = np.mgrid[0:H, 0:W]
    t = np.clip((yy - top) / float(bottom - top), 0, 1)
    half = w0 / 2 + (w1 - w0) / 2 * t
    inside = (yy >= top) & (yy <= bottom) & (np.abs(xx - cx) <= half)
    soft = np.clip(1 - np.abs(xx - cx) / np.maximum(half, 1), 0, 1) ** 0.6
    alpha = inside * (0.55 - 0.35 * t) * soft * (img[..., :3].max(-1) > 60)   # sky only: trees stay black
    col = np.array((150, 255, 90) if hover else (255, 255, 255), float)
    out = img.copy()
    out[..., :3] = img[..., :3] * (1 - alpha[..., None]) + col * alpha[..., None]
    return out


def ufo(draw, cx, cy, s, hover):
    black = (0, 0, 0, 255)
    draw.ellipse((cx - 36 * s, cy - 34 * s, cx + 36 * s, cy + 6 * s), fill=black)       # dome
    draw.ellipse((cx - 80 * s, cy - 12 * s, cx + 80 * s, cy + 16 * s), fill=black)      # saucer
    draw.ellipse((cx - 34 * s, cy + 4 * s, cx + 34 * s, cy + 24 * s), fill=black)       # belly
    light = (90, 255, 40, 255) if hover else (225, 225, 225, 255)
    for k in range(-3, 4):
        x = cx + k * 20 * s
        draw.ellipse((x - 4.5 * s, cy + 0.5 * s, x + 4.5 * s, cy + 9.5 * s), fill=light)


def wordmark(size, hover):
    layer = Image.new("RGBA", size, (0, 0, 0, 0))
    for text, cap, cx, top in LINES:
        probe = ImageFont.truetype(FONT, 400)
        bb = probe.getbbox("H")
        font = ImageFont.truetype(FONT, int(400 * cap / float(bb[3] - bb[1])))
        tmp = Image.new("RGBA", (int(font.getlength(text)) + 400, int(cap * 2.2) + 200), (0, 0, 0, 0))
        d = ImageDraw.Draw(tmp)
        ox, oy = 150, 100 - font.getbbox("H")[1]
        sw = max(6, int(cap * 0.058))
        for dx, dy in ((-sw * 0.9, sw * 0.9), (-sw * 0.5, sw * 0.5)):                 # black drop outline
            d.text((ox + dx, oy + dy), text, font=font, fill=(0, 0, 0, 255), stroke_width=int(sw * 2.1),
                   stroke_fill=(0, 0, 0, 255))
        d.text((ox, oy), text, font=font, fill=TEXT[hover], stroke_width=sw, stroke_fill=(255, 255, 255, 255))
        tmp = tmp.transform(tmp.size, Image.AFFINE, (1, SHEAR, -SHEAR * (100 + cap), 0, 1, 0), resample=Image.BICUBIC)
        tmp = tmp.crop(tmp.getbbox())
        layer.alpha_composite(tmp, (int(cx - tmp.size[0] / 2), int(top)))
    return layer


def build(src, hover, frame=True):
    img = badge(src, hover)
    H, W = img.shape[:2]
    ux, uy, us = UFO
    img = beam(img, ux, uy + 20 * us, BEAM_TO, 40 * us, 170 * us, hover)
    out = Image.fromarray(np.clip(img, 0, 255).astype(np.uint8), "RGBA")
    ufo(ImageDraw.Draw(out), ux, uy, us, hover)
    out.alpha_composite(wordmark((W, H), hover))
    if frame:
        d = ImageDraw.Draw(out)
        for i in range(15):
            d.rectangle((i, i, W - 1 - i, H - 1 - i), outline=FRAME[hover])
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gebsfish", default=r"P:\gebsfish\data")
    ap.add_argument("--png")
    a = ap.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        for hover, name in ((False, "logo"), (True, "logo_hover")):
            img = build(paa_to_array(os.path.join(a.gebsfish, name + ".paa"), tmp), hover)
            png = os.path.join(a.png or tmp, name + ".png")
            img.save(png)
            paa = os.path.join(REPO, "data", name + ".paa")
            if os.path.exists(paa):
                os.remove(paa)
            subprocess.run([IMAGE_TO_PAA, png, paa], check=True, capture_output=True)
            print("wrote", paa)
            if hover:                        # README header: the colour version without its frame, like Gebsfish's
                readme = os.path.join(REPO, ".github", "logo.png")
                build(paa_to_array(os.path.join(a.gebsfish, name + ".paa"), tmp), hover, frame=False).save(readme)
                print("wrote", readme)
    return 0


if __name__ == "__main__":
    sys.exit(main())
