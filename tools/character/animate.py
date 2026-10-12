"""The mascot's animation loops, and a renderer for their frames.

Each loop is a function from phase (0..1, wrapping) to a Pose. Running this
file renders every loop at the LCD's resolution into out/frames/<loop>/ and
writes an enlarged out/<loop>.gif preview.

Usage: python tools/character/animate.py [loop ...]
"""
import sys
from concurrent.futures import ProcessPoolExecutor
from math import cos, pi, sin
from os import cpu_count
from pathlib import Path

from PIL import Image

import rig3d
from rig import Pose

OUT = Path(__file__).parent / "out"
PREVIEW_SCALE = 2
WORKERS = 2   # each render holds several hundred MB while tracing


def idle(t):
    """Dancing on the spot: a bounce on each beat, arms pumping, feet stepping."""
    a = 2 * pi * t
    s = 0.5 + 0.5 * sin(a)
    return Pose(
        hip=6 * sin(a),
        bounce=2.5 * (1 - cos(2 * a)),
        head_tilt=5 * sin(a + 0.5),
        l_shoulder=220 - 40 * s, l_elbow=300 - 50 * s,
        r_shoulder=10 - 50 * s, r_elbow=-60 - 60 * s,
        l_foot_lift=7 * max(0.0, -sin(a)), r_foot_lift=7 * max(0.0, sin(a)),
        tail=12 * sin(a - 1.0),
    )


def shake(t):
    """The jiggle: butt swinging side to side, arms out for balance."""
    a = 2 * pi * t
    return Pose(
        hip=22 * sin(a),
        bounce=5 + 2 * cos(2 * a),
        head_tilt=-4 * sin(a),
        l_shoulder=160 + 8 * sin(a), l_elbow=120 + 10 * sin(a),
        r_shoulder=20 + 8 * sin(a), r_elbow=60 + 10 * sin(a),
        tail=-18 * sin(a - 0.6),
    )


# name -> (pose function, frame count, frames per second)
LOOPS = {
    "idle": (idle, 20, 15),
    "shake": (shake, 12, 15),
}


def _frame(job):
    name, index, count = job
    return index, rig3d.render(LOOPS[name][0](index / count))


def render_loop(name, pool):
    _, count, fps = LOOPS[name]
    folder = OUT / "frames" / name
    folder.mkdir(parents=True, exist_ok=True)
    frames = [None] * count
    for index, pixels in pool.map(_frame, [(name, i, count) for i in range(count)]):
        frames[index] = Image.fromarray(pixels)
        frames[index].save(folder / f"{index:03d}.png")
    big = [f.resize((f.width * PREVIEW_SCALE, f.height * PREVIEW_SCALE), Image.LANCZOS)
           for f in frames]
    big[0].save(OUT / f"{name}.gif", save_all=True, append_images=big[1:],
                duration=round(1000 / fps), loop=0)
    print(f"{name}: {count} frames -> {OUT / (name + '.gif')}")


def main():
    names = sys.argv[1:] or list(LOOPS)
    with ProcessPoolExecutor(max_workers=min(WORKERS, cpu_count() or 1)) as pool:
        for name in names:
            render_loop(name, pool)


if __name__ == "__main__":
    main()
