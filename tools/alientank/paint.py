r"""Procedural textures for the specimen tank, written as PNG and converted to PAA with DayZ Tools' ImageToPAA.

Every surface texture tiles (noise is made periodic through the FFT), so the model maps them by metres (build.py
T_* tiles). Colour maps are sRGB; _nohq normal maps are DayZ's X+ Y- (green down), bright = raised; _smdi is
R 1, G specular, B gloss. Text uses Windows' Bahnschrift and Consolas.
"""
import os
import subprocess
import tempfile

import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

DAYZ_TOOLS = os.environ.get("DAYZ_TOOLS", r"G:\SteamLibrary\steamapps\common\DayZ Tools")
IMAGE_TO_PAA = os.path.join(DAYZ_TOOLS, "Bin", "ImageToPAA", "ImageToPAA.exe")
FONTS = os.path.join(os.environ.get("WINDIR", r"C:\Windows"), "Fonts")


def font(name, size):
    return ImageFont.truetype(os.path.join(FONTS, name), size)


# --------------------------------------------------------------------------------------------------------------
# building blocks
# --------------------------------------------------------------------------------------------------------------

def noise(h, w, cycles, seed, octaves=4, persistence=0.5, aspect=(1.0, 1.0)):
    """Periodic fractal noise in 0..1. cycles: blobs across the image at the first octave; aspect stretches the
    blobs (u, v) - (1, 8) gives vertical streaks."""
    rng = np.random.default_rng(seed)
    fy = np.fft.fftfreq(h)[:, None] * h
    fx = np.fft.fftfreq(w)[None, :] * w
    out = np.zeros((h, w))
    amp, total = 1.0, 0.0
    for o in range(octaves):
        c = cycles * 2 ** o
        r = np.sqrt((fx * aspect[0]) ** 2 + (fy * aspect[1]) ** 2)
        layer = np.real(np.fft.ifft2(np.fft.fft2(rng.standard_normal((h, w))) * np.exp(-(r / c) ** 2)))
        out += amp * layer / (layer.std() + 1e-12)
        total += amp
        amp *= persistence
    out /= total
    return (out - out.min()) / (out.max() - out.min() + 1e-12)


def normal_map(height, strength):
    """DayZ normal map from a periodic height map (0..1)."""
    dx = (np.roll(height, -1, 1) - np.roll(height, 1, 1)) * 0.5 * strength
    dy = (np.roll(height, -1, 0) - np.roll(height, 1, 0)) * 0.5 * strength     # image rows grow downward
    n = np.stack([-dx, -dy, np.ones_like(height)], -1)
    n /= np.linalg.norm(n, axis=-1, keepdims=True)
    rgb = np.clip((n * 0.5 + 0.5) * 255, 0, 255).astype(np.uint8)
    return Image.fromarray(np.dstack([rgb, np.full(height.shape, 255, np.uint8)]), "RGBA")


def smdi(spec, gloss):
    spec, gloss = np.broadcast_arrays(np.clip(spec, 0, 1), np.clip(gloss, 0, 1))
    rgb = np.stack([np.ones_like(spec), spec, gloss], -1)
    return Image.fromarray((rgb * 255).astype(np.uint8), "RGB")


def rgb_image(col):
    return Image.fromarray(np.clip(col * 255, 0, 255).astype(np.uint8), "RGB")


def tint(base, *layers):
    """base RGB (3,) times per-pixel factors."""
    f = np.ones(layers[0].shape) if layers else 1
    for layer in layers:
        f = f * layer
    return np.asarray(base, float)[None, None, :] * f[..., None]


def disc(h, w, cx, cy, r, soft=1.0):
    """Anti-aliased filled disc mask, wrapping at the edges."""
    y = np.arange(h)[:, None]
    x = np.arange(w)[None, :]
    dx = (x - cx + w / 2) % w - w / 2
    dy = (y - cy + h / 2) % h - h / 2
    d = np.sqrt(dx ** 2 + dy ** 2)
    return np.clip((r - d) / soft + 0.5, 0, 1), d


def scratches(h, w, count, seed, length=(20, 120), angle_spread=0.6, base_angle=0.0):
    rng = np.random.default_rng(seed)
    im = Image.new("L", (w * 3, h * 3), 0)
    d = ImageDraw.Draw(im)
    for _ in range(count):
        x, y = rng.uniform(w, 2 * w), rng.uniform(h, 2 * h)
        a = base_angle + rng.normal(0, angle_spread)
        ln = rng.uniform(*length)
        d.line([(x, y), (x + np.cos(a) * ln, y + np.sin(a) * ln)], fill=int(rng.uniform(90, 255)), width=1)
    arr = np.asarray(im, float) / 255
    # fold the 3x canvas back so lines crossing an edge wrap
    out = np.zeros((h, w))
    for i in range(3):
        for j in range(3):
            out = np.maximum(out, arr[i * h:(i + 1) * h, j * w:(j + 1) * w])
    return out


# --------------------------------------------------------------------------------------------------------------
# textures (each returns dict suffix -> PIL image)
# --------------------------------------------------------------------------------------------------------------

def metal(n=1024):
    """Painted dark steel panels, 1 m per repeat: seams every 0.5 m, rivet rows, scratches, grime streaks."""
    big = noise(n, n, 3, 1)
    fine = noise(n, n, 40, 2, 3)
    streak = noise(n, n, 6, 3, 4, 0.55, aspect=(1.0, 0.12))       # stretched along v: vertical streaks
    scr = scratches(n, n, 350, 4, (8, 45), 0.9) * 0.6
    h = 0.5 + 0.08 * (fine - 0.5)
    seam = np.zeros((n, n))
    x = np.arange(n)
    for pos in (0, n // 2):
        d = np.abs((x - pos + n / 2) % n - n / 2)
        groove = np.clip(1 - d / 3.0, 0, 1)
        seam = np.maximum(seam, groove[None, :])
    d = np.abs((x - 0 + n / 2) % n - n / 2)
    seam = np.maximum(seam, np.clip(1 - d / 3.0, 0, 1)[:, None])
    h -= 0.35 * seam
    rivets = np.zeros((n, n))
    step = n // 16
    for line_x in (0, n // 2):
        for k in range(16):
            for side in (-1, 1):
                m, dd = disc(n, n, line_x + side * 14, k * step + step / 2, 5.5)
                rivets = np.maximum(rivets, m * np.sqrt(np.clip(1 - (dd / 5.5) ** 2, 0, 1)))
    for k in range(32):
        for side in (-1, 1):
            m, dd = disc(n, n, k * step / 2 + step / 4, side * 14, 5.5)
            rivets = np.maximum(rivets, m * np.sqrt(np.clip(1 - (dd / 5.5) ** 2, 0, 1)))
    h += 0.45 * rivets
    shade = (0.86 + 0.22 * big) * (0.93 + 0.14 * fine) * (0.78 + 0.25 * streak)
    col = tint((0.24, 0.27, 0.255), shade)
    col *= (1 - 0.55 * seam)[..., None]
    col += (0.10 * rivets)[..., None]
    col = col * (1 - 0.4 * scr[..., None]) + np.array([0.36, 0.37, 0.36]) * 0.4 * scr[..., None]
    grime = np.clip((streak - 0.55) * 2.5, 0, 1) * 0.35
    col = col * (1 - grime[..., None]) + np.array([0.10, 0.09, 0.07]) * grime[..., None]
    spec = 0.22 + 0.45 * scr + 0.15 * rivets - 0.1 * grime
    gloss = 0.32 + 0.25 * scr - 0.15 * grime
    return {"co": rgb_image(col), "nohq": normal_map(h + 0.04 * scr, 6.0), "smdi": smdi(spec, gloss)}


def trim(n=512):
    """Brushed steel, 0.5 m per repeat, a little grime."""
    brush = noise(n, n, 6, 11, 5, 0.65, aspect=(0.04, 1.0))        # streaks along u
    big = noise(n, n, 2, 12)
    grime = noise(n, n, 5, 13, 4, 0.5, aspect=(1.0, 0.2))
    scr = scratches(n, n, 300, 14, (8, 40), 0.3)
    shade = (0.82 + 0.25 * brush) * (0.9 + 0.15 * big)
    col = tint((0.47, 0.49, 0.50), shade)
    g = np.clip((grime - 0.6) * 2.0, 0, 1) * 0.4
    col = col * (1 - g[..., None]) + np.array([0.13, 0.12, 0.10]) * g[..., None]
    col = col * (1 - 0.4 * scr[..., None]) + 0.62 * 0.4 * scr[..., None]
    return {"co": rgb_image(col), "nohq": normal_map(0.5 + 0.06 * brush + 0.03 * scr, 4.0),
            "smdi": smdi(0.55 + 0.2 * brush - 0.2 * g, 0.55 + 0.1 * brush - 0.25 * g)}


def hazard(w=1024, h=128):
    """0.8 m x 0.1 m band: worn yellow/black stripes with a BIOHAZARD plate in each repeat."""
    y, x = np.mgrid[0:h, 0:w]
    stripe = (((x + y) // 48) % 2 == 0)                        # 45 degrees, ~3.8 cm stripes
    col = np.where(stripe[..., None], np.array([0.86, 0.66, 0.06]), np.array([0.07, 0.07, 0.07]))
    img = rgb_image(col)
    d = ImageDraw.Draw(img)
    plate = (int(w * 0.30), 14, int(w * 0.70), h - 14)
    d.rectangle(plate, fill=(18, 18, 18))
    f = font("bahnschrift.ttf", 58)
    text = "BIOHAZARD"
    tw = d.textlength(text, font=f)
    d.text(((w - tw) / 2, h / 2 - 30), text, fill=(222, 170, 20), font=f)
    col = np.asarray(img, float) / 255
    wear = noise(h, w, 10, 21, 5, 0.6)
    chips = np.clip((wear - 0.72) * 6, 0, 1)
    edge = np.clip(1 - np.minimum(y, h - 1 - y) / 10.0, 0, 1) * np.clip((wear - 0.45) * 3, 0, 1)
    worn = np.maximum(chips, edge)
    steel = tint((0.38, 0.39, 0.39), 0.8 + 0.3 * noise(h, w, 30, 22, 2))
    col = col * (1 - worn[..., None]) + steel * worn[..., None]
    dirt = noise(h, w, 4, 23, 4, 0.5, aspect=(1.0, 0.3))
    col *= (0.78 + 0.25 * dirt)[..., None]
    height = 0.5 - 0.15 * worn
    return {"co": rgb_image(col), "nohq": normal_map(height, 5.0), "smdi": smdi(0.25 + 0.35 * worn, 0.35 + 0.2 * worn)}


def hose(n=256):
    """Corrugated black rubber, 0.3 m per repeat; ribs run round the hose (v is along it)."""
    y = np.arange(n)[:, None]
    rib = 0.5 + 0.5 * np.sin(2 * np.pi * y * 20 / n) * np.ones((1, n))
    dust = noise(n, n, 4, 31, 4)
    col = tint((0.075, 0.078, 0.082), (0.8 + 0.35 * rib) * (0.85 + 0.3 * dust))
    return {"co": rgb_image(col), "nohq": normal_map(rib * 0.6, 9.0), "smdi": smdi(0.12 + 0.1 * rib, 0.3 + 0.1 * rib)}


def grille(n=512):
    """Perforated dark plate, 0.4 m per repeat; the holes are lit green from below (the material glows)."""
    holes = np.zeros((n, n))
    pitch = 32
    for row in range(n // pitch * 2):
        cy = row * pitch / 2 * np.sqrt(3) / 1.0
        if cy >= n:
            break
        off = (pitch / 2) * (row % 2)
        for k in range(n // pitch):
            m, _ = disc(n, n, k * pitch + off, cy, 9.5)
            holes = np.maximum(holes, m)
    fine = noise(n, n, 20, 41, 3)
    plate = tint((0.07, 0.08, 0.075), 0.8 + 0.4 * fine)
    glow = np.array([0.35, 1.0, 0.5])
    col = plate * (1 - holes[..., None]) + glow * holes[..., None]
    return {"co": rgb_image(col), "nohq": normal_map(0.5 - 0.5 * holes, 4.0), "smdi": smdi(0.3 * (1 - holes), 0.4)}


def glow(n=256):
    """Lamp colours in the patches build.py's GLOW_* rects point at, brightest in the middle."""
    col = np.zeros((n, n, 3))
    rects = {(0.02, 0.02, 0.48, 0.48): (0.45, 1.0, 0.55), (0.52, 0.02, 0.73, 0.23): (1.0, 0.22, 0.14),
             (0.77, 0.02, 0.98, 0.23): (1.0, 0.62, 0.12), (0.52, 0.27, 0.73, 0.48): (0.85, 1.0, 0.88)}
    y, x = np.mgrid[0:n, 0:n] / n
    for (u0, v0, u1, v1), c in rects.items():
        inside = (x >= u0) & (x <= u1) & (y >= v0) & (y <= v1)
        cx, cy = (u0 + u1) / 2, (v0 + v1) / 2
        r = np.sqrt(((x - cx) / (u1 - u0)) ** 2 + ((y - cy) / (v1 - v0)) ** 2) * 2
        lvl = 0.75 + 0.25 * np.clip(1 - r, 0, 1)
        col[inside] = np.array(c) * lvl[inside][:, None]
    return {"co": rgb_image(col)}


def screen(w=820, h=512):
    """The console's monitor: specimen read-outs over a heartbeat trace, scanlines; squashed to 512 x 512."""
    img = Image.new("RGB", (w, h), (4, 18, 11))
    d = ImageDraw.Draw(img)
    green, dim = (90, 255, 140), (30, 110, 60)
    for gx in range(0, w, 41):
        d.line([(gx, 0), (gx, h)], fill=(10, 40, 24))
    for gy in range(0, h, 41):
        d.line([(0, gy), (w, gy)], fill=(10, 40, 24))
    fb = font("consolab.ttf", 38)
    f = font("consola.ttf", 30)
    fs = font("consola.ttf", 24)
    d.text((24, 16), "S-4 CONTAINMENT", fill=green, font=fb)
    d.text((24, 60), "SPECIMEN 07  //  EBE-2", fill=dim, font=f)
    d.rectangle((w - 210, 18, w - 24, 58), outline=green, width=2)
    d.text((w - 196, 22), "DORMANT", fill=green, font=fb)
    # heartbeat trace
    xs = np.arange(24, w - 24)
    ys = []
    for x in xs:
        t = (x - 24) / 260.0
        ph = t - np.floor(t)
        v = 0.0
        if 0.30 < ph < 0.34:
            v = (ph - 0.30) / 0.04 * 1.0
        elif 0.34 <= ph < 0.38:
            v = 1.0 - (ph - 0.34) / 0.04 * 1.6
        elif 0.38 <= ph < 0.42:
            v = -0.6 + (ph - 0.38) / 0.04 * 0.6
        elif 0.55 < ph < 0.70:
            v = 0.18 * np.sin((ph - 0.55) / 0.15 * np.pi)
        ys.append(200 - v * 70)
    pts = list(zip(xs, ys))
    d.line(pts, fill=green, width=3)
    rows = [("PULSE", "12 BPM"), ("RESP", "0.4 /MIN"), ("TEMP", "4.1 C"), ("FLUID", "98.6 %"), ("NEURAL", "")]
    for i, (k, v) in enumerate(rows):
        yy = 290 + i * 42
        d.text((24, yy), k, fill=dim, font=f)
        d.text((180, yy), v, fill=green, font=f)
    for i in range(9):                              # neural activity bars
        bh = [10, 18, 6, 26, 14, 8, 20, 12, 5][i]
        d.rectangle((180 + i * 22, 300 + 4 * 42 + 30 - bh, 194 + i * 22, 300 + 4 * 42 + 30), fill=green)
    d.text((w - 330, 290), "LID SEALED", fill=green, font=f)
    d.text((w - 330, 332), "O2 SAT  91 %", fill=dim, font=f)
    d.text((w - 330, 374), "!! DO NOT OPEN", fill=(255, 90, 60), font=f)
    d.text((24, h - 34), "SYS 4.22  UPLINK OK  CYCLE 117 432", fill=dim, font=fs)
    arr = np.asarray(img, float) / 255
    arr *= (0.82 + 0.18 * (np.arange(h) % 4 < 2))[:, None, None]          # scanlines
    yy, xx = np.mgrid[0:h, 0:w]
    vign = 1 - 0.35 * (((xx / w - 0.5) * 1.6) ** 2 + ((yy / h - 0.5) * 1.6) ** 2)
    arr *= np.clip(vign, 0.5, 1)[..., None]
    glowimg = Image.fromarray((np.clip(arr, 0, 1) * 255).astype(np.uint8)).filter(ImageFilter.GaussianBlur(2))
    arr = np.clip(arr + 0.5 * np.asarray(glowimg, float) / 255, 0, 1)
    return {"co": rgb_image(arr).resize((512, 512), Image.LANCZOS)}


def biohazard(d, cx, cy, r, fill, bg):
    """Simplified biohazard trefoil."""
    for k in range(3):
        a = np.radians(90 + 120 * k)
        ox, oy = cx + np.cos(a) * r * 0.42, cy - np.sin(a) * r * 0.42
        d.ellipse((ox - r * 0.52, oy - r * 0.52, ox + r * 0.52, oy + r * 0.52), fill=fill)
    for k in range(3):
        a = np.radians(90 + 120 * k)
        ox, oy = cx + np.cos(a) * r * 0.58, cy - np.sin(a) * r * 0.58
        d.ellipse((ox - r * 0.34, oy - r * 0.34, ox + r * 0.34, oy + r * 0.34), fill=bg)
    d.ellipse((cx - r * 0.24, cy - r * 0.24, cx + r * 0.24, cy + r * 0.24), outline=fill, width=max(2, int(r * 0.07)))
    d.ellipse((cx - r * 0.12, cy - r * 0.12, cx + r * 0.12, cy + r * 0.12), fill=bg)


def label(w=1024, h=606):
    """Warning plate on the console front; squashed to 512 x 256 (the plate is 0.44 x 0.26 m)."""
    base = trim(512)["co"].resize((w, h))
    img = Image.blend(base, Image.new("RGB", (w, h), (205, 205, 200)), 0.55)
    d = ImageDraw.Draw(img)
    d.rectangle((10, 10, w - 10, h - 10), outline=(30, 30, 30), width=6)
    d.rectangle((10, 10, w - 10, 150), fill=(170, 28, 22))
    fb = font("bahnschrift.ttf", 112)
    tw = d.textlength("WARNING", font=fb)
    d.text(((w - tw) / 2, 18), "WARNING", fill=(245, 240, 230), font=fb)
    biohazard(d, 150, 330, 110, (25, 25, 25), (200, 200, 195))
    f = font("bahnschrift.ttf", 52)
    fs = font("bahnschrift.ttf", 40)
    d.text((290, 200), "LIVE BIOLOGICAL", fill=(25, 25, 25), font=f)
    d.text((290, 260), "SPECIMEN", fill=(25, 25, 25), font=f)
    d.text((290, 340), "DO NOT BREACH", fill=(150, 25, 20), font=fs)
    d.text((290, 386), "CONTAINMENT", fill=(150, 25, 20), font=fs)
    d.line((30, 470, w - 30, 470), fill=(30, 30, 30), width=4)
    d.text((40, 495), "S-4 / SPECIMEN 07", fill=(40, 40, 40), font=fs)
    d.text((w - 330, 495), "EBE-2  CLASS IV", fill=(40, 40, 40), font=fs)
    for cx, cy in ((32, 32), (w - 32, 32), (32, h - 32), (w - 32, h - 32)):      # screws
        d.ellipse((cx - 12, cy - 12, cx + 12, cy + 12), fill=(120, 120, 118), outline=(40, 40, 40), width=2)
        d.line((cx - 8, cy, cx + 8, cy), fill=(40, 40, 40), width=3)
    arr = np.asarray(img, float) / 255
    dirt = noise(h, w, 4, 51, 4, 0.5)
    arr *= (0.85 + 0.2 * dirt)[..., None]
    out = rgb_image(arr).resize((512, 256), Image.LANCZOS)
    return {"co": out, "smdi": smdi(np.full((256, 512), 0.35), np.full((256, 512), 0.45))}


def fluid(w=512, h=1024):
    """Green fluid seen through the glass: u is a quarter of the way round, v the whole height (top at the
    image's top). Alpha is the fluid's opacity: thicker toward the floor, light streaks and suspended specks."""
    streak = noise(h, w, 5, 61, 5, 0.6, aspect=(1.0, 0.08))
    swirl = noise(h, w, 3, 62, 4, 0.5)
    specks = np.clip((noise(h, w, 120, 63, 1) - 0.86) * 9, 0, 1)
    v = np.linspace(0, 1, h)[:, None] * np.ones((1, w))                 # 0 top .. 1 bottom
    ends = np.exp(-((v - 0.0) / 0.10) ** 2) + np.exp(-((1 - v) / 0.10) ** 2)    # brighter at the light rings
    bright = 0.62 + 0.28 * streak + 0.12 * swirl + 0.35 * ends
    col = tint((0.16, 0.88, 0.30), bright)
    col = col * (1 - specks[..., None] * 0.5) + np.array([0.75, 1.0, 0.7]) * specks[..., None] * 0.5
    alpha = 0.48 + 0.10 * v + 0.08 * swirl + 0.05 * streak + 0.25 * specks
    rgba = np.dstack([np.clip(col, 0, 1), np.clip(alpha, 0, 1)])
    img = Image.fromarray((rgba * 255).astype(np.uint8), "RGBA")
    return {"ca": img, "smdi": smdi(np.full((64, 64), 0.9), np.full((64, 64), 0.9)),
            "nohq": normal_map(0.5 + 0.02 * swirl[::4, ::4], 2.0)}


TEXTURES = {
    "geb_tank_metal": metal, "geb_tank_trim": trim, "geb_tank_hazard": hazard, "geb_tank_hose": hose,
    "geb_tank_grille": grille, "geb_tank_glow": glow, "geb_tank_screen": screen, "geb_tank_label": label,
    "geb_tank_fluid": fluid,
}


def to_paa(img, path, tmp):
    png = os.path.join(tmp, os.path.basename(path)[:-4] + ".png")
    img.save(png)
    if os.path.exists(path):
        os.remove(path)
    subprocess.run([IMAGE_TO_PAA, png, path], check=True, capture_output=True)


def build_all(out_dir, names=None):
    os.makedirs(out_dir, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        for name, fn in TEXTURES.items():
            if names and name not in names:
                continue
            for suffix, img in fn().items():
                path = os.path.join(out_dir, "%s_%s.paa" % (name, suffix))
                to_paa(img, path, tmp)
                print("wrote", os.path.relpath(path, os.path.join(out_dir, "..", "..")))


if __name__ == "__main__":
    import sys
    build_all(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "..", "..", "data",
                                                                    "tank"))
