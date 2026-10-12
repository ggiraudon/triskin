"""The Triskin mascot as a real 3D model, posed by the same Pose as rig.py.

The character is a signed-distance field built from rounded primitives, in the
2D rig's pixel coordinates plus a depth axis (z grows away from the viewer), and
rendered by sphere tracing with an orthographic camera. render(pose, style)
returns an RGB image; STYLES lists the available looks ("clay" is the chosen one).

Needs numpy.
"""
from math import cos, radians, sin

import numpy as np

from rig import H, W, Pose

F = np.float32

SKIN, DARK_SKIN, HORN, WHITE, INK, TONGUE, MOUTH = range(7)

ALBEDO = np.array([
    [0.64, 0.33, 0.84],   # SKIN
    [0.47, 0.20, 0.68],   # DARK_SKIN
    [0.97, 0.91, 0.72],   # HORN
    [1.00, 1.00, 1.00],   # WHITE
    [0.13, 0.05, 0.20],   # INK
    [1.00, 0.49, 0.61],   # TONGUE
    [0.50, 0.10, 0.22],   # MOUTH
], dtype=F)
GLOSS = np.array([1.0, 1.0, 0.6, 1.6, 0.3, 0.8, 0.0], dtype=F)

STYLES = ("vinyl", "toon", "clay")

# The backdrop: black, with a pool of light on the floor under the character.
FLOOR_Y = 291            # where the feet meet the floor
POOL_RADII = (104, 21)   # the pool is a circle seen from just above the floor
POOL_COLOUR = (0.40, 0.40, 0.43)

Z_NEAR, Z_FAR = -100.0, 70.0
LIGHT = np.array([-0.45, -0.65, -0.62], dtype=F)
LIGHT /= np.linalg.norm(LIGHT)
FILL = np.array([0.7, 0.35, -0.6], dtype=F)
FILL /= np.linalg.norm(FILL)
VIEW = np.array([0, 0, -1], dtype=F)
HALF = (LIGHT + VIEW) / np.linalg.norm(LIGHT + VIEW)


def _v(*xyz):
    return np.array(xyz, dtype=F)


def _len(v):
    return np.sqrt((v * v).sum(-1))


def _sphere(p, c, r):
    return _len(p - c) - r


def _ellipsoid(p, c, r):
    q = p - c
    k0 = _len(q / r)
    k1 = np.maximum(_len(q / (r * r)), 1e-6)
    return k0 * (k0 - 1.0) / k1


def _capsule(p, a, b, r):
    pa, ba = p - a, b - a
    h = np.clip((pa @ ba) / (ba @ ba), 0, 1)
    return _len(pa - h[:, None] * ba) - r


def _round_cone(p, a, b, r1, r2):
    ba = b - a
    l2 = float(ba @ ba)
    rr = r1 - r2
    a2 = l2 - rr * rr
    il2 = 1.0 / l2
    pa = p - a
    y = pa @ ba
    z = y - l2
    w = pa * l2 - y[:, None] * ba
    x2 = (w * w).sum(-1)
    y2 = y * y * l2
    z2 = z * z * l2
    k = np.sign(rr) * rr * rr * x2
    return np.where(
        np.sign(z) * a2 * z2 > k, np.sqrt(x2 + z2) * il2 - r2,
        np.where(np.sign(y) * a2 * y2 < k, np.sqrt(x2 + y2) * il2 - r1,
                 (np.sqrt(x2 * a2 * il2) + y * rr) * il2 - r1))


def _smin(a, b, k):
    h = np.clip(0.5 + 0.5 * (b - a) / k, 0, 1)
    return b + (a - b) * h - k * h * (1 - h)


def _ease(t):
    t = np.clip(t, 0, 1)
    return t * t * (3 - 2 * t)


class Model:
    """The posed character. dist() is its signed distance field."""

    def __init__(self, pose):
        self.pose = pose
        b = pose.bounce

        def sk(x, y, z=0.0):
            return _v(x + pose.hip * float(_ease((y - 135) / 95)), y + b, z)

        self.arms = []
        for (sx, sy), a_up, a_fore in (((95, 138), pose.l_shoulder, pose.l_elbow),
                                       ((145, 138), pose.r_shoulder, pose.r_elbow)):
            # Arms stay in the torso's plane (the 2D rig draws them behind it),
            # so a hand resting on a hip sinks into it instead of sitting on top.
            s = sk(sx, sy, 6)
            e = s + _v(32 * cos(radians(a_up)), 32 * sin(radians(a_up)), 0)
            h = e + _v(30 * cos(radians(a_fore)), 30 * sin(radians(a_fore)), -2)
            thumb = h + _v(7 if h[0] < W / 2 else -7, -9, -2)
            self.arms.append((s, e, h, thumb))

        self.legs = []
        for hip_x, foot_x, lift, outward in ((97, 90, pose.l_foot_lift, -1),
                                             (143, 150, pose.r_foot_lift, 1)):
            hip = sk(hip_x, 240)
            foot = _v(foot_x, 284 - lift, 0)
            knee = _v((hip[0] + foot_x) / 2 + outward * (6 + lift * 0.5), (hip[1] + foot[1]) / 2, 0)
            mid = 0.25 * hip + 0.5 * knee + 0.25 * foot
            self.legs.append((hip, mid, foot, _v(foot_x + outward * 7, foot[1] + 3, 0)))

        t0 = sk(120, 196, -30)
        t1 = _v(150 + pose.hip, 214 + b, -60)
        t2 = _v(188 + pose.hip * 0.8 + pose.tail * 0.5, 190 + b, -50)
        t3 = _v(174 + pose.hip * 0.5 + pose.tail, 150 + b - abs(pose.tail) * 0.15, -36)
        ts = np.linspace(0, 1, 13, dtype=F)[:, None]
        self.tail = ((1 - ts) ** 3 * t0 + 3 * (1 - ts) ** 2 * ts * t1
                     + 3 * (1 - ts) * ts ** 2 * t2 + ts ** 3 * t3)
        d = (t3 - t2) / np.linalg.norm(t3 - t2)
        self.tail_tip = (t3 - 3 * d, t3 + 21 * d)

        self.head_shift = _v(pose.hip * -0.12, b * 1.15, 0)
        self.head_cos = cos(radians(pose.head_tilt))
        self.head_sin = sin(radians(pose.head_tilt))
        blink = max(0.4, min(0.97, pose.blink))
        self.lid_y = 77 - 13.7 + 27.4 * blink

    def _head_space(self, p):
        q = p - self.head_shift
        x, y = q[:, 0] - 120, q[:, 1] - 116
        out = q.copy()
        out[:, 0] = 120 + x * self.head_cos + y * self.head_sin
        out[:, 1] = 116 - x * self.head_sin + y * self.head_cos
        return out

    def _body_space(self, p):
        out = p.copy()
        out[:, 1] -= self.pose.bounce
        out[:, 0] -= self.pose.hip * _ease((out[:, 1] - 135) / 95)
        return out

    def _mouth(self, ph):
        """2D smile on the snout: (inside the mouth opening, inside the lips)."""
        lower = np.hypot(ph[:, 0] - 87, ph[:, 1] - 86) - 24.5
        upper = np.hypot(ph[:, 0] - 89, ph[:, 1] - 62) - 38.2
        lips = np.maximum(lower, -upper)
        opening = np.maximum(lower, -(upper - 4.5))   # below the row of teeth
        return opening, lips

    def dist(self, p, materials=False):
        pb = self._body_space(p)
        ph = self._head_space(p)

        torso = _smin(_ellipsoid(pb, _v(120, 148, 2), _v(31, 36, 24)),
                      _ellipsoid(pb, _v(120, 206, 2), _v(56, 44, 36)), 18)
        cheeks = np.minimum(_ellipsoid(pb, _v(90, 216, -12), _v(34, 36, 36)),
                            _ellipsoid(pb, _v(150, 216, -12), _v(34, 36, 36)))
        body = _smin(torso, cheeks, 8)

        head = _smin(_sphere(ph, _v(120, 84, 0), 38),
                     _ellipsoid(ph, _v(88, 99, -12), _v(27, 18, 22)), 8)
        opening, lips = self._mouth(ph)
        head = np.maximum(head, -np.maximum(opening, ph[:, 2] + 23))

        limbs = np.full(len(p), 1e9, dtype=F)
        for s, e, h, thumb in self.arms:
            arm = np.minimum(_capsule(p, s, e, 7.5), _capsule(p, e, h, 7.5))
            hand = _smin(_sphere(p, h, 11), _sphere(p, thumb, 5.5), 3)
            limbs = np.minimum(limbs, _smin(arm, hand, 3))
        for hip, mid, foot, _ in self.legs:
            limbs = np.minimum(limbs, np.minimum(_capsule(p, hip, mid, 8), _capsule(p, mid, foot, 8)))

        skin = _smin(_smin(body, head, 6), limbs, 5)

        dark = np.full(len(p), 1e9, dtype=F)
        for _, _, _, foot_c in self.legs:
            dark = np.minimum(dark, _ellipsoid(p, foot_c, _v(17, 9.5, 15)))
        for a, c in zip(self.tail[:-1], self.tail[1:]):
            dark = np.minimum(dark, _capsule(p, a, c, 3.2))
        dark = np.minimum(dark, _round_cone(p, self.tail_tip[0], self.tail_tip[1], 11, 0.8))
        dark = np.minimum(dark, _round_cone(ph, _v(150, 80, 6), _v(181, 56, 10), 11, 1))
        eye_c = _v(104, 77, -30)
        lid = np.maximum(_ellipsoid(ph, eye_c, _v(11.7, 13.7, 8.2)), ph[:, 1] - self.lid_y)
        dark = np.minimum(dark, lid)

        horn = np.full(len(p), 1e9, dtype=F)
        for sign in (-1, 1):
            base, mid, tip = (_v(120 + sign * 15, 56, 0), _v(120 + sign * 23, 38, -1),
                              _v(120 + sign * 14, 20, -2))
            horn = np.minimum(horn, _smin(_round_cone(ph, base, mid, 9, 5.5),
                                          _round_cone(ph, mid, tip, 5.5, 1), 2))

        white = np.minimum(_ellipsoid(ph, eye_c, _v(10.5, 12.5, 7)),
                           _sphere(ph, _v(104, 79.5, -37.6), 1.5))
        ink = np.minimum(_ellipsoid(ph, _v(106, 82, -36.3), _v(5, 5, 1.8)),
                         _capsule(ph, _v(90, 69, -19.5), _v(117, 59, -28.5), 2.8))
        ink = np.minimum(ink, _sphere(ph, _v(67, 91, -19), 1.8))
        tongue = _ellipsoid(ph, _v(91, 109, -22), _v(10, 5, 7))

        groups = (skin, dark, horn, white, ink, tongue)
        d = np.minimum.reduce(groups)
        if not materials:
            return d

        mat = np.argmin(np.stack(groups), axis=0)
        on_head = (mat == SKIN) & (head <= np.minimum(body, limbs) + 0.5) & (ph[:, 2] < -8)
        mat[on_head & (lips < 0.4)] = WHITE       # teeth
        mat[on_head & (opening < 0.6)] = MOUTH
        return d, mat


def _trace(model, scale):
    """Sphere-trace one orthographic ray per pixel. Returns hit mask, points."""
    w, h = W * scale, H * scale
    xs = (np.arange(w, dtype=F) + 0.5) / scale
    ys = (np.arange(h, dtype=F) + 0.5) / scale
    px, py = (a.ravel() for a in np.meshgrid(xs, ys))
    z = np.full(px.shape, Z_NEAR, dtype=F)
    hit = np.zeros(px.shape, dtype=bool)
    live = np.arange(px.size)
    for _ in range(140):
        if live.size == 0:
            break
        p = np.stack([px[live], py[live], z[live]], axis=1)
        d = model.dist(p)
        z[live] += d * 0.7     # the field is only approximately a distance
        done = d < 0.06
        hit[live[done]] = True
        live = live[~done & (z[live] < Z_FAR)]
    points = np.stack([px[hit], py[hit], z[hit]], axis=1)
    return hit.reshape(h, w), points


def _normals(model, p):
    e = 0.6
    n = np.empty_like(p)
    for axis in range(3):
        off = np.zeros(3, dtype=F)
        off[axis] = e
        n[:, axis] = model.dist(p + off) - model.dist(p - off)
    return n / np.maximum(_len(n), 1e-6)[:, None]


def _occlusion(model, p, n):
    occ = np.zeros(len(p), dtype=F)
    for i, step in enumerate((2.0, 5.0, 9.0, 14.0)):
        occ += (step - model.dist(p + n * step)) / (1.6 ** i)
    return np.clip(1 - occ * 0.045, 0, 1)


def _shadow(model, p, n):
    """Soft shadow factor towards the key light: 0 = fully shadowed."""
    origin = p + n * 1.5
    t = np.full(len(p), 1.0, dtype=F)
    lit = np.ones(len(p), dtype=F)
    for _ in range(28):
        d = model.dist(origin + LIGHT * t[:, None])
        lit = np.minimum(lit, np.clip(7.0 * d / t, 0, 1))
        t += np.clip(d, 0.8, 12.0)
    return lit


def _hex(c):
    return np.array([int(c[i:i + 2], 16) for i in (1, 3, 5)], dtype=F) / 255


def _backdrop(scale, model):
    w, h = W * scale, H * scale
    xs = (np.arange(w, dtype=F) + 0.5) / scale
    ys = (np.arange(h, dtype=F) + 0.5) / scale
    x, y = np.meshgrid(xs, ys)
    # Evenly lit in the middle, fading out over the outer part of the pool.
    d = np.sqrt(((x - 120) / POOL_RADII[0]) ** 2 + ((y - FLOOR_Y) / POOL_RADII[1]) ** 2)
    edge = np.clip((d - 0.45) / 0.55, 0, 1)
    light = 1 - edge * edge * (3 - 2 * edge)
    img = light[:, :, None] * np.array(POOL_COLOUR, dtype=F)
    # The character's own shadow on the floor.
    cx = 120 + model.pose.hip * 0.4
    fall = np.clip(1 - ((x - cx) / (56 - model.pose.bounce * 0.5)) ** 2 - ((y - 293) / 8) ** 2, 0, 1)
    img *= (1 - 0.75 * fall ** 0.7)[:, :, None]
    return img.astype(F)


def _dilate(mask, radius):
    out = np.zeros_like(mask)
    h, w = mask.shape
    for dy in range(-radius, radius + 1):
        for dx in range(-radius, radius + 1):
            if dx * dx + dy * dy > radius * radius + 1:
                continue
            ys, yd = slice(max(0, -dy), h - max(0, dy)), slice(max(0, dy), h - max(0, -dy))
            xs, xd = slice(max(0, -dx), w - max(0, dx)), slice(max(0, dx), w - max(0, -dx))
            out[yd, xd] |= mask[ys, xs]
    return out


def _shade(style, n, mat, ao, lit):
    alb = ALBEDO[mat]
    gloss = GLOSS[mat][:, None]
    ndl = n @ LIGHT
    ndh = np.clip(n @ HALF, 0, 1)
    ndv = np.clip(n @ VIEW, 0, 1)
    sky = (0.5 - 0.5 * n[:, 1])[:, None]     # 1 where the surface faces up
    # The mouth is a cavity, so it would otherwise render almost black and the
    # grin would be lost at screen size.
    inside_mouth = (mat == MOUTH) | (mat == TONGUE)
    ao = np.where(inside_mouth, np.maximum(ao, 0.8), ao)[:, None]
    lit = np.where(inside_mouth, np.maximum(lit, 0.7), lit)[:, None]

    if style == "vinyl":
        wrap = np.clip((ndl + 0.35) / 1.35, 0, 1)[:, None]
        col = alb * ((0.30 + 0.20 * sky) * ao + 0.85 * wrap * (0.30 + 0.70 * lit))
        col += alb * _v(0.35, 0.45, 0.75) * (np.clip(n @ FILL, 0, 1)[:, None] * 0.35 * ao)
        col += gloss * lit * (0.42 * ndh ** 90 + 0.07 * ndh ** 12)[:, None]
        col += _v(0.55, 0.70, 1.0) * (((1 - ndv) ** 3)[:, None] * 0.45 * (0.4 + 0.6 * sky) * ao)
    elif style == "toon":
        t = ndl * (0.35 + 0.65 * lit[:, 0])
        band = np.where(t > 0.22, 1.08, np.where(t > -0.18, 0.84, 0.62))[:, None]
        col = alb * band * (0.88 + 0.12 * ao)
    elif style == "clay":
        grey = alb.mean(axis=1, keepdims=True)
        soft = alb * 0.88 + grey * 0.12
        col = soft * ((0.42 + 0.24 * sky) * ao ** 1.6
                      + 0.72 * np.clip(ndl, 0, 1)[:, None] * (0.25 + 0.75 * lit))
        col += gloss * lit * (0.06 * ndh ** 8)[:, None]
    else:
        raise ValueError(f"unknown style {style!r}; expected one of {STYLES}")
    return np.clip(col, 0, 1)


def geometry(pose=None, scale=1, supersample=2):
    """The expensive, style-independent part of a render."""
    model = Model(pose or Pose())
    s = scale * supersample
    hit, p = _trace(model, s)
    n = _normals(model, p)
    _, mat = model.dist(p, materials=True)
    return dict(model=model, s=s, supersample=supersample, hit=hit, depth=p[:, 2], n=n, mat=mat,
                ao=_occlusion(model, p, n), lit=_shadow(model, p, n))


def render(pose=None, style="clay", scale=1, supersample=2, geo=None):
    """Render one frame as a uint8 RGB array of (H*scale, W*scale, 3)."""
    g = geo or geometry(pose, scale, supersample)
    hit, s, ss = g["hit"], g["s"], g["supersample"]
    img = _backdrop(s, g["model"])
    img[hit] = _shade(style, g["n"], g["mat"], g["ao"], g["lit"])

    if style == "toon":
        depth = np.full(hit.shape, Z_FAR, dtype=F)
        depth[hit] = g["depth"]
        edge = np.zeros_like(hit)
        jump_y = np.abs(np.diff(depth, axis=0)) > 5.0
        jump_x = np.abs(np.diff(depth, axis=1)) > 5.0
        edge[:-1] |= jump_y
        edge[1:] |= jump_y
        edge[:, :-1] |= jump_x
        edge[:, 1:] |= jump_x
        # Creases, like the one between the cheeks, have no depth jump but the
        # surface direction changes abruptly across them.
        normal = np.zeros(hit.shape + (3,), dtype=F)
        normal[hit] = g["n"]
        crease_y = hit[1:] & hit[:-1] & ((normal[1:] * normal[:-1]).sum(-1) < 0.55)
        crease_x = hit[:, 1:] & hit[:, :-1] & ((normal[:, 1:] * normal[:, :-1]).sum(-1) < 0.55)
        edge[:-1] |= crease_y
        edge[:, :-1] |= crease_x
        # Parts lying on the body (the tail across the butt) differ in neither
        # depth nor direction, only in what they are made of.
        material = np.full(hit.shape, -1)
        material[hit] = g["mat"]
        edge[:-1] |= hit[1:] & hit[:-1] & (material[1:] != material[:-1])
        edge[:, :-1] |= hit[:, 1:] & hit[:, :-1] & (material[:, 1:] != material[:, :-1])
        img[_dilate(edge, max(1, round(0.9 * s)))] = ALBEDO[INK]

    h, w = H * (s // ss), W * (s // ss)
    img = img.reshape(h, ss, w, ss, 3).mean(axis=(1, 3))
    return (img * 255 + 0.5).astype(np.uint8)
