r"""Alien voices: many takes cut and varied from the mod's two alien recordings, for data/aliens/sounds/config.cpp.

    python tools/build_alien_sounds.py
    python tools/build_alien_sounds.py --measure "P:\DZ\sounds\ai\infected\voicefx\zmbm_skinny_attack_*.ogg"

Aliens used to play the same two recordings (aliensound.ogg, alienscream.ogg) for everything, so every growl was
the same growl. This keeps those two and builds data/aliens/sounds/voice/geb_alien_<event>_<n>.ogg from them:
the chirp, growl, burble, call and scream inside them are cut apart, re-joined, pitched, stretched and filtered
into several takes per infected voice event. Every take is mono (they're 3D sounds), faded, levelled to vanilla's
skinny infected voice for that event and kept under -1 dBFS (alienscream.ogg went over full scale).
config.cpp mixes these takes with vanilla's own skinny infected voice.

--measure prints the level and length this script levels by, to check takes against vanilla's.
Needs ffmpeg with rubberband and libvorbis on PATH.
"""
import argparse
import glob
import os
import subprocess

import numpy as np

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SOUNDS = os.path.join(REPO, "data", "aliens", "sounds")
OUT = os.path.join(SOUNDS, "voice")
SR = 44100
LIMIT = 0.89        # -1 dBFS

# Active level (dB RMS of the loud part, see active_db) of vanilla's zmbm_skinny voice per event, from --measure.
# Calm is a little above vanilla's (-38): vanilla's calm takes are breathy and carry further than a tonal chirp.
# Call to arms and death sit a little under vanilla's: they're screams, and louder only pushes them into the limiter.
LEVEL = {
    "calm": -36, "disturbed": -21, "alerted": -28, "chase": -18, "attack": -16, "lighthit": -21,
    "heavyhit": -18, "jump": -20.5, "land": -22.5, "calltoarms": -16.5, "death": -16,
}
# Takes in one event aren't all equally loud in vanilla either
SPREAD = [0, -1.5, 1, -0.5, -2, 0.5, -1, 1.5, -2.5]


def decode(path):
    raw = subprocess.run(["ffmpeg", "-v", "error", "-i", path, "-ac", "1", "-ar", str(SR), "-f", "f32le", "-"],
                         capture_output=True, check=True).stdout
    return np.frombuffer(raw, np.float32).astype(np.float64)


def ff(x, af):
    """x through an ffmpeg filter chain."""
    raw = subprocess.run(["ffmpeg", "-v", "error", "-f", "f32le", "-ar", str(SR), "-ac", "1", "-i", "-", "-af", af,
                          "-ar", str(SR), "-ac", "1", "-f", "f32le", "-"],
                         input=x.astype(np.float32).tobytes(), capture_output=True, check=True).stdout
    return np.frombuffer(raw, np.float32).astype(np.float64)


def encode(x, path):
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-f", "f32le", "-ar", str(SR), "-ac", "1", "-i", "-",
                    "-c:a", "libvorbis", "-q:a", "5", path], input=x.astype(np.float32).tobytes(), check=True)


def windows(x, seconds):
    n = max(1, len(x) // int(seconds * SR))
    w = len(x) // n
    return np.sqrt((x[:n * w].reshape(n, w) ** 2).mean(1) + 1e-12)


def active_db(x):
    """RMS of the 50 ms windows within 20 dB of the loudest: how loud the call is, ignoring its pauses."""
    w = windows(x, 0.05)
    loud = w[w > w.max() * 0.1]
    return 20 * np.log10(np.sqrt((loud ** 2).mean()))


# --- cutting -----------------------------------------------------------------------------------------------------

def cut(x, t0, t1, snap=0.03):
    """x between two times, each moved to the quietest 5 ms within `snap` so cuts fall between sounds."""
    hop = int(0.005 * SR)
    e = windows(x, 0.005)

    def quiet(t):
        i, r = int(t * SR / hop), int(snap * SR / hop)
        lo, hi = max(0, i - r), min(len(e), i + r + 1)
        return (lo + int(np.argmin(e[lo:hi]))) * hop

    return x[quiet(t0):quiet(t1)].copy()


def fade(x, fin=0.006, fout=0.05):
    x = x.copy()
    a, b = min(len(x) // 4, int(fin * SR)), min(len(x) // 3, int(fout * SR))
    if a:
        x[:a] *= np.linspace(0, 1, a)
    if b:
        x[-b:] *= np.linspace(1, 0, b) ** 1.5
    return x


def trim(x, seconds, fout=0.06):
    return fade(x[:int(seconds * SR)], fout=fout)


def strip(x, below_db=-45):
    """Drop silence before and after the sound."""
    hop = int(0.005 * SR)
    e = windows(x, 0.005)
    on = np.nonzero(e > e.max() * 10 ** (below_db / 20))[0]
    return x[on[0] * hop:(on[-1] + 1) * hop]


def join(*parts):
    """Parts in a row; a number between them is a pause in seconds."""
    out = []
    for p in parts:
        out.append(np.zeros(int(p * SR)) if isinstance(p, (int, float)) else fade(p, 0.004, 0.03))
    return np.concatenate(out)


# --- shaping -----------------------------------------------------------------------------------------------------

def tape(x, r):
    """Pitch and speed together by r, like a bigger (r < 1) or smaller throat."""
    return ff(x, "asetrate=%d,aresample=%d" % (round(SR * r), SR))


def pitch(x, r, keep_voice=False):
    """Pitch by r at the same length; keep_voice keeps the vowel (formants) so it's the same throat, other note."""
    return ff(x, "rubberband=pitch=%g:formant=%s:pitchq=quality" % (r, "preserved" if keep_voice else "shifted"))


def tempo(x, t):
    """Length / t at the same pitch."""
    return ff(x, "rubberband=tempo=%g" % t)


def lowpass(x, hz):
    return ff(x, "lowpass=f=%d:p=2" % hz)


def highpass(x, hz):
    return ff(x, "highpass=f=%d:p=2" % hz)


def rattle(x, hz, depth):
    """Throat flutter."""
    return ff(x, "tremolo=f=%g:d=%g" % (hz, depth))


def thicken(x, db=-12):
    """An octave below underneath, for a bigger chest."""
    low = pitch(x, 0.5)
    n = min(len(x), len(low))
    return x[:n] + low[:n] * 10 ** (db / 20)


def finish(x, level_db):
    x = fade(strip(x))
    x *= 10 ** ((level_db - active_db(x)) / 20)
    if np.abs(x).max() > LIMIT:
        x = ff(x, "alimiter=limit=%g:attack=1:release=60:level=0:latency=1" % (LIMIT * 0.97))
        x = np.clip(x, -LIMIT, LIMIT)
    return fade(x, 0.004, 0.03)


# --- the takes ---------------------------------------------------------------------------------------------------

def takes(A, S):
    """{event: [take, ...]} from aliensound (A) and alienscream (S)."""
    chirp = cut(A, 0.14, 0.33)      # short rising chirp
    growl = cut(A, 0.36, 0.76)      # the loud growl
    burble = cut(A, 0.76, 0.98)     # throaty burble
    call = cut(A, 0.98, 2.20)       # long rising call
    call1 = cut(A, 0.98, 1.56)
    call2 = cut(A, 1.54, 2.20)
    sound = cut(A, 0.06, 2.30)      # aliensound.ogg as it was
    scream = cut(S, 0.0, 0.80)      # the scream without its tail
    shriek = cut(S, 0.0, 0.46)      # up to the scream's peak
    onset = cut(S, 0.0, 0.30)       # the scream's first breath
    rasp = cut(S, 0.78, 1.98)       # the rasps and clicks after it
    screamall = S                   # alienscream.ogg as it was

    return {
        "calm": [
            rattle(lowpass(tape(call, 0.8), 2400), 14, 0.35),
            lowpass(tempo(tape(growl, 0.78), 0.55), 1800),
            lowpass(join(tape(chirp, 0.92), 0.35, tape(chirp, 1.06), 0.5, tape(burble, 0.9)), 3500),
            lowpass(pitch(rasp, 0.75), 3200),
            lowpass(tempo(pitch(call2, 0.84, True), 0.7), 2800),
            rattle(lowpass(tape(sound, 0.88), 3000), 9, 0.25),
            lowpass(sound, 3800),
            join(lowpass(tape(burble, 0.8), 2200), 0.2, lowpass(tempo(tape(call1, 0.86), 0.8), 2600)),
        ],
        "disturbed": [
            join(chirp, 0.08, tape(chirp, 1.1)),
            join(tape(chirp, 1.15), 0.05, tape(burble, 1.05)),
            join(tape(chirp, 0.92), 0.1, tape(chirp, 1.12), 0.08, tape(chirp, 1.2)),
            tape(call2, 1.12),
            tempo(pitch(growl, 1.08, True), 1.1),
            join(tape(burble, 1.2), 0.06, tape(chirp, 1.25)),
            tape(call1, 1.1),
        ],
        "alerted": [
            join(thicken(tape(growl, 0.86)), 0.25, tape(burble, 0.9)),
            tape(sound, 0.94),
            join(growl, 0.08, tape(call1, 0.95)),
            tempo(pitch(call, 0.8), 0.95),
            rattle(join(tape(burble, 0.88), 0.05, tape(growl, 0.88), 0.3, tape(chirp, 0.9)), 28, 0.3),
            lowpass(tempo(pitch(scream, 0.62), 0.6), 2400),
            join(tape(call2, 0.9), 0.15, chirp, 0.1, tape(chirp, 0.95)),
            join(thicken(tempo(pitch(growl, 0.9), 0.6), -14), 0.2, tape(call2, 0.95)),
        ],
        "chase": [
            join(scream, 0.15, tape(burble, 1.05)),
            join(tape(scream, 0.9), 0.1, tape(chirp, 1.0)),
            join(tape(growl, 1.05), 0.05, tape(scream, 1.1)),
            join(pitch(scream, 0.86, True), 0.2, tape(growl, 1.1)),
            join(growl, 0.03, tape(shriek, 1.05), 0.12, tape(burble, 1.1)),
            screamall,
            sound,
            join(tape(shriek, 0.93), 0.1, tape(burble, 1.1), 0.05, tape(chirp, 1.05)),
            thicken(tape(call, 1.08), -14),
        ],
        "attack": [
            shriek,
            tape(shriek, 1.15),
            tape(shriek, 0.88),
            tape(onset, 1.05),
            tempo(tape(growl, 1.12), 1.2),
            join(tape(chirp, 0.85), 0.02, tape(onset, 1.1)),
            tempo(pitch(shriek, 1.1, True), 1.15),
            join(tape(burble, 0.95), 0.03, tape(onset, 1.2)),
        ],
        "lighthit": [
            tape(onset, 1.2),
            onset,
            join(tape(chirp, 1.25), 0.02, tape(burble, 1.3)),
            highpass(join(tape(chirp, 1.05), 0.03, tape(chirp, 1.2)), 300),
            pitch(onset, 1.25, True),
            tape(burble, 1.3),
            trim(tape(shriek, 1.1), 0.3),
        ],
        "heavyhit": [
            trim(tape(scream, 0.95), 0.65),
            tape(shriek, 0.85),
            tempo(pitch(scream, 0.9), 1.1),
            join(tape(growl, 1.15), 0.02, tape(onset, 1.1)),
            join(tape(shriek, 1.08), 0.04, tape(burble, 0.9)),
            trim(tape(scream, 1.05), 0.7),
            tape(call1, 1.2),
        ],
        "jump": [
            trim(tape(growl, 1.05), 0.4),
            join(lowpass(tape(chirp, 0.9), 3000), 0.02, trim(tape(burble, 1.0), 0.15)),
            trim(tape(burble, 1.1), 0.25),
            lowpass(pitch(onset, 0.75), 2500),
            trim(tape(growl, 1.3), 0.3),
        ],
        "land": [
            trim(lowpass(tape(growl, 0.8), 2000), 0.3),
            trim(tape(burble, 0.8), 0.3),
            trim(tape(growl, 0.9), 0.3),
            trim(lowpass(pitch(onset, 0.6), 2000), 0.25),
            lowpass(tape(burble, 0.7), 2500),
        ],
        "calltoarms": [
            join(scream, 0.12, tape(call2, 1.05)),
            tape(screamall, 0.97),
            thicken(call, -14),
            join(tape(scream, 0.92), 0.15, tape(shriek, 1.08)),
            join(growl, 0.04, scream),
            join(pitch(scream, 0.85), 0.1, tape(chirp, 1.1)),
        ],
        "death": [
            screamall,
            fade(join(tape(scream, 0.85), 0.05, lowpass(tempo(tape(call, 0.75), 0.8), 2500)), fout=0.6),
            fade(join(tempo(pitch(scream, 0.8), 0.8), 0.05, lowpass(pitch(rasp, 0.7), 2500)), fout=0.4),
            fade(rattle(tape(sound, 0.8), 12, 0.4), fout=0.8),
            fade(join(tape(scream, 0.95), 0.08, lowpass(tempo(tape(growl, 0.7), 0.7), 2000)), fout=0.5),
        ],
    }


def describe(path, x):
    return "%-34s %5.2f s  active %6.1f dB  peak %6.1f dBFS" % (
        os.path.basename(path), len(x) / SR, active_db(x), 20 * np.log10(np.abs(x).max() + 1e-12))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--measure", nargs="+", metavar="GLOB")
    a = ap.parse_args()
    if a.measure:
        for g in a.measure:
            for f in sorted(glob.glob(g)):
                print(describe(f, decode(f)))
        return

    os.makedirs(OUT, exist_ok=True)
    for old in glob.glob(os.path.join(OUT, "geb_alien_*.ogg")):
        os.remove(old)
    events = takes(decode(os.path.join(SOUNDS, "aliensound.ogg")), decode(os.path.join(SOUNDS, "alienscream.ogg")))
    for event, xs in events.items():
        for i, x in enumerate(xs):
            x = finish(x, LEVEL[event] + SPREAD[i % len(SPREAD)])
            path = os.path.join(OUT, "geb_alien_%s_%d.ogg" % (event, i + 1))
            encode(x, path)
            print(describe(path, x))
    print("wrote %d takes in data/aliens/sounds/voice" % sum(len(xs) for xs in events.values()))


if __name__ == "__main__":
    main()
