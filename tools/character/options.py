"""Render every candidate look side by side to tools/character/out/.

Writes options.png (all looks x three poses) and one option_<letter>_<name>.png
strip per look. Usage: python tools/character/options.py   (takes a few minutes)
"""
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import rig3d
from preview import POSES, rasterize
from rig import H, W, svg

OUT = Path(__file__).parent / "out"
SCALE = 2
SHOWN = ("neutral", "idle_b", "shake_left")
LOOKS = (("A", "flat", "Flat cartoon (the original)"),
         ("B", "vinyl", "3D - glossy vinyl toy"),
         ("C", "toon", "3D - toon shaded with outlines"),
         ("D", "clay", "3D - soft matte clay"))
LABEL_H = 44


def flat(pose):
    pix = rasterize(svg(pose), SCALE)
    return Image.frombytes("RGB", (pix.width, pix.height), pix.samples)


def main():
    OUT.mkdir(exist_ok=True)
    font = ImageFont.load_default(size=26)
    geos = {name: rig3d.geometry(POSES[name], scale=SCALE) for name in SHOWN}
    strips = []
    for letter, look, title in LOOKS:
        strip = Image.new("RGB", (W * SCALE * len(SHOWN), H * SCALE + LABEL_H), "#0b161d")
        ImageDraw.Draw(strip).text((14, 7), f"{letter}  {title}", fill="#ffffff", font=font)
        for i, name in enumerate(SHOWN):
            frame = flat(POSES[name]) if look == "flat" else Image.fromarray(
                rig3d.render(style=look, geo=geos[name]))
            strip.paste(frame, (W * SCALE * i, LABEL_H))
        strip.save(OUT / f"option_{letter}_{look}.png")
        strips.append(strip)
    sheet = Image.fromarray(np.vstack([np.asarray(s) for s in strips]))
    sheet.save(OUT / "options.png")
    print(f"wrote {OUT / 'options.png'}")


if __name__ == "__main__":
    main()
