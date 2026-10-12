"""Render a contact sheet of key poses to tools/character/out/ for review.

Usage: python tools/character/preview.py        (needs: pip install pymupdf)
"""
from pathlib import Path

import pymupdf

from rig import H, W, Pose, svg

OUT = Path(__file__).parent / "out"
SCALE = 2

POSES = {
    "neutral": Pose(),
    "idle_a": Pose(bounce=5, head_tilt=-6, l_shoulder=220, l_elbow=300, r_shoulder=10,
                   r_elbow=-60, tail=-10),
    "idle_b": Pose(bounce=0, head_tilt=5, l_shoulder=180, l_elbow=250, r_shoulder=-40,
                   r_elbow=-120, tail=12, r_foot_lift=8),
    "shake_left": Pose(hip=-22, bounce=6, head_tilt=4, tail=18, l_shoulder=160, l_elbow=120,
                       r_shoulder=20, r_elbow=60),
    "shake_right": Pose(hip=22, bounce=6, head_tilt=-4, tail=-18, l_shoulder=160, l_elbow=120,
                        r_shoulder=20, r_elbow=60),
    "blink": Pose(blink=1),
}


def rasterize(svg_text, scale=SCALE):
    page = pymupdf.open(stream=svg_text.encode(), filetype="svg")[0]
    return page.get_pixmap(matrix=pymupdf.Matrix(scale, scale), alpha=False)


def main():
    OUT.mkdir(exist_ok=True)
    sheet = pymupdf.Pixmap(pymupdf.csRGB, pymupdf.IRect(0, 0, W * SCALE * 3, H * SCALE * 2), False)
    sheet.clear_with(0)
    for i, (name, pose) in enumerate(POSES.items()):
        text = svg(pose)
        (OUT / f"{name}.svg").write_text(text)
        pix = rasterize(text)
        pix.set_origin(W * SCALE * (i % 3), H * SCALE * (i // 3))
        sheet.copy(pix, pix.irect)
    sheet.save(OUT / "poses.png")
    print(f"wrote {OUT / 'poses.png'}")


if __name__ == "__main__":
    main()
