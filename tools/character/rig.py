"""The Triskin mascot as a posable vector rig.

svg(pose) returns one frame as an SVG string on a 240x320 canvas (the LCD's
size). Every moving part is driven by a field of Pose, so animations are just
functions from time to Pose.
"""
from dataclasses import dataclass
from math import atan2, cos, degrees, radians, sin, sqrt

W, H = 240, 320

OUTLINE = "#2b1040"
BODY = "#a455d6"
BODY_LIGHT = "#c183ec"
BODY_DARK = "#8238b4"
HORN = "#f7e9b8"
MOUTH = "#4b0f24"
TONGUE = "#ff7d9c"
BACKGROUND = "#10202b"
SPOT = "#1b3646"

LINE = 3.5        # outline thickness
LIMB = 15         # arm / leg thickness


@dataclass
class Pose:
    hip: float = 0.0          # sideways butt offset in px, + is right
    bounce: float = 0.0       # whole-body drop in px, + is down (knees bend)
    head_tilt: float = 0.0    # degrees, + is clockwise
    l_shoulder: float = 150   # arm angles in degrees, 0 = pointing right, 90 = down
    l_elbow: float = 55       # the defaults put both hands on the hips
    r_shoulder: float = 30
    r_elbow: float = 125
    l_foot_lift: float = 0.0  # px
    r_foot_lift: float = 0.0
    tail: float = 0.0         # tail tip swing in px, + is right
    blink: float = 0.0        # 0 = open, 1 = shut


def _ease(t):
    t = max(0.0, min(1.0, t))
    return t * t * (3 - 2 * t)


def _fmt(points):
    return " ".join(f"{x:.1f},{y:.1f}" for x, y in points)


def _outlined(d, fill, extra=""):
    return (f'<path d="{d}" fill="{fill}" stroke="{OUTLINE}" stroke-width="{LINE}" '
            f'stroke-linejoin="round" {extra}/>')


def _tube(d, width=LIMB, fill=BODY):
    """A limb: a fat round-capped stroke with an outline around it."""
    common = 'fill="none" stroke-linecap="round" stroke-linejoin="round"'
    return (f'<path d="{d}" stroke="{OUTLINE}" stroke-width="{width + 2 * LINE}" {common}/>'
            f'<path d="{d}" stroke="{fill}" stroke-width="{width}" {common}/>')


def _arm(shoulder, a_upper, a_fore):
    sx, sy = shoulder
    ex, ey = sx + 32 * cos(radians(a_upper)), sy + 32 * sin(radians(a_upper))
    hx, hy = ex + 30 * cos(radians(a_fore)), ey + 30 * sin(radians(a_fore))
    # Thumb sits on the inner side of the hand, towards the head.
    tx = hx + (7 if hx < W / 2 else -7)
    return (
        f'<circle cx="{tx:.1f}" cy="{hy - 9:.1f}" r="5.5" fill="{BODY}" stroke="{OUTLINE}" '
        f'stroke-width="{LINE}"/>'
        + _tube(f"M {sx:.1f},{sy:.1f} L {ex:.1f},{ey:.1f} L {hx:.1f},{hy:.1f}")
        + f'<circle cx="{hx:.1f}" cy="{hy:.1f}" r="11" fill="{BODY}" stroke="{OUTLINE}" '
          f'stroke-width="{LINE}"/>'
        # Hide the outline where the forearm runs into the hand.
        + f'<path d="M {ex:.1f},{ey:.1f} L {hx:.1f},{hy:.1f}" stroke="{BODY}" '
          f'stroke-width="{LIMB}" stroke-linecap="round" fill="none"/>'
    )


def _leg(hip, foot_x, lift, outward):
    hx, hy = hip
    fy = 284 - lift
    kx = (hx + foot_x) / 2 + outward * (6 + lift * 0.5)
    ky = (hy + fy) / 2
    foot = (f'<ellipse cx="{foot_x + outward * 7:.1f}" cy="{fy + 3:.1f}" rx="17" ry="9.5" '
            f'fill="{BODY_DARK}" stroke="{OUTLINE}" stroke-width="{LINE}"/>')
    return foot + _tube(f"M {hx:.1f},{hy:.1f} Q {kx:.1f},{ky:.1f} {foot_x:.1f},{fy:.1f}")


def _head(pose, dx, dy):
    blink = max(0.0, min(1.0, pose.blink))
    # The resting lid is already half shut: that's the mischief. The lid is the
    # slice of the eye ellipse above lid_y (drawn directly; the rasterizer
    # ignores clip paths).
    ecx, ecy, erx, ery = 104, 77, 10.5, 12.5
    lid_y = ecy - ery + 2 * ery * max(0.4, min(0.97, blink))
    half = erx * sqrt(1 - ((lid_y - ecy) / ery) ** 2)
    lid = (f"M {ecx - half:.1f},{lid_y:.1f} A {erx},{ery} 0 {int(lid_y > ecy)} 1 "
           f"{ecx + half:.1f},{lid_y:.1f} Z")
    parts = [
        # Horns and the far ear sit behind the skull.
        _outlined("M 97,58 C 90,40 95,27 107,21 C 105,34 110,45 117,52 Z", HORN),
        _outlined("M 143,58 C 150,40 145,27 133,21 C 135,34 130,45 123,52 Z", HORN),
        _outlined("M 148,72 L 181,56 L 158,96 Z", BODY_DARK),
        # Skull and snout share one silhouette: outlines first, fills on top.
        f'<circle cx="120" cy="84" r="38" fill="{BODY}" stroke="{OUTLINE}" stroke-width="{LINE * 2}"/>',
        f'<ellipse cx="88" cy="98" rx="26" ry="16" fill="{BODY}" stroke="{OUTLINE}" '
        f'stroke-width="{LINE * 2}"/>',
        f'<circle cx="120" cy="84" r="38" fill="{BODY}"/>',
        f'<ellipse cx="88" cy="98" rx="26" ry="16" fill="{BODY}"/>',
        f'<ellipse cx="134" cy="66" rx="14" ry="9" fill="{BODY_LIGHT}" opacity="0.55"/>',
        # Eye, looking back at the viewer.
        f'<ellipse cx="{ecx}" cy="{ecy}" rx="{erx}" ry="{ery}" fill="#fff"/>',
        f'<circle cx="106" cy="82" r="5" fill="{OUTLINE}"/>',
        f'<circle cx="104.5" cy="80.5" r="1.6" fill="#fff"/>',
        f'<path d="{lid}" fill="{BODY_DARK}" stroke="{OUTLINE}" stroke-width="2.5" '
        f'stroke-linejoin="round"/>',
        f'<ellipse cx="{ecx}" cy="{ecy}" rx="{erx}" ry="{ery}" fill="none" stroke="{OUTLINE}" '
        f'stroke-width="{LINE}"/>',
        f'<path d="M 90,69 L 117,59" stroke="{OUTLINE}" stroke-width="5" stroke-linecap="round"/>',
        # Grin with teeth and tongue.
        _outlined("M 66,93 C 80,99 98,100 112,92 C 111,113 78,117 66,93 Z", MOUTH),
        f'<path d="M 82,110 C 86,103 97,102 102,107 C 96,112 88,113 82,110 Z" fill="{TONGUE}"/>',
        f'<path d="M 70,95.5 C 82,100 97,101 110,94.5 C 108,101 80,104 70,95.5 Z" fill="#fff"/>',
        f'<path d="M 112,92 C 115,90 117,87 117,84" stroke="{OUTLINE}" stroke-width="3" '
        f'fill="none" stroke-linecap="round"/>',
        f'<circle cx="69" cy="88" r="1.8" fill="{OUTLINE}"/>',
    ]
    return (f'<g transform="translate({dx:.1f},{dy:.1f}) rotate({pose.head_tilt:.1f},120,116)">'
            + "".join(parts) + "</g>")


def svg(pose=None, background=True):
    pose = pose or Pose()
    b = pose.bounce

    def sk(x, y):
        """Shoulders stay put while the hips swing: shear the body by height."""
        return x + pose.hip * _ease((y - 135) / 95), y + b

    body_pts = [
        (120, 116), (97, 116), (91, 134), (87, 153), (81, 178), (57, 193), (57, 217),
        (57, 240), (80, 253), (104, 251), (112, 250), (118, 247), (120, 242), (122, 247),
        (128, 250), (136, 251), (160, 253), (183, 240), (183, 217), (183, 193), (159, 178),
        (153, 153), (149, 134), (143, 116), (120, 116),
    ]
    p = [sk(x, y) for x, y in body_pts]
    body = f"M {_fmt(p[:1])} " + " ".join(f"C {_fmt(p[i:i + 3])}" for i in range(1, len(p), 3)) + " Z"
    cleft = f"M {_fmt([sk(120, 204)])} C {_fmt([sk(116, 216), sk(117, 232), sk(120, 242)])}"

    tb = sk(120, 196)
    tip = (174 + pose.hip * 0.5 + pose.tail, 150 + b - abs(pose.tail) * 0.15)
    mid = (150 + pose.hip, 214 + b)
    ctl = (188 + pose.hip * 0.8 + pose.tail * 0.5, 190 + b)
    tail = f"M {_fmt([tb])} C {_fmt([mid, ctl, tip])}"
    # Arrowhead pointing along the direction the tail is travelling at its tip.
    ang = degrees(atan2(tip[1] - ctl[1], tip[0] - ctl[0]))
    spade = [(tip[0] + r * cos(radians(ang + a)), tip[1] + r * sin(radians(ang + a)))
             for r, a in ((22, 0), (13, 105), (3, 180), (13, 255))]

    out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}">']
    if background:
        out.append(f'<rect width="{W}" height="{H}" fill="{BACKGROUND}"/>')
        out.append(f'<ellipse cx="120" cy="170" rx="118" ry="150" fill="{SPOT}"/>')
    shadow_rx = 62 - b * 0.6
    out.append(f'<ellipse cx="{120 + pose.hip * 0.4:.1f}" cy="294" rx="{shadow_rx:.1f}" ry="8" '
               f'fill="#000" opacity="0.35"/>')
    out.append(_leg(sk(97, 240), 90, pose.l_foot_lift, -1))
    out.append(_leg(sk(143, 240), 150, pose.r_foot_lift, 1))
    out.append(_arm(sk(95, 138), pose.l_shoulder, pose.l_elbow))
    out.append(_arm(sk(145, 138), pose.r_shoulder, pose.r_elbow))
    out.append(_outlined(body, BODY))
    lx, ly = sk(84, 212)
    rx, ry = sk(150, 212)
    out.append(f'<ellipse cx="{lx:.1f}" cy="{ly:.1f}" rx="13" ry="17" fill="{BODY_LIGHT}" opacity="0.5" '
               f'transform="rotate(20,{lx:.1f},{ly:.1f})"/>')
    out.append(f'<ellipse cx="{rx:.1f}" cy="{ry:.1f}" rx="11" ry="15" fill="{BODY_LIGHT}" opacity="0.4" '
               f'transform="rotate(20,{rx:.1f},{ry:.1f})"/>')
    out.append(f'<path d="{cleft}" stroke="{OUTLINE}" stroke-width="{LINE}" fill="none" '
               f'stroke-linecap="round"/>')
    out.append(_tube(tail, width=5, fill=BODY_DARK))
    out.append(_outlined(f"M {_fmt(spade)} Z", BODY_DARK))
    out.append(_head(pose, pose.hip * -0.12, b * 1.15))
    out.append("</svg>")
    return "".join(out)
