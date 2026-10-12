# Case for the Waveshare RP2350-Touch-LCD-2

Three 3D-printable parts. The case holds the module with the screen **flush** with
the front face and, in the easel, stands at **75° from the desk with the USB-C at
the top**.

## Files

Each printed part has its own `.scad` you can open and press **F6** on. The STLs
are generated from those — none of them is a one-off export.

| Open this | Gives you | Print it |
|---|---|---|
| `bezel.scad` | the front body | **open back on the bed**, 15 mm tall |
| `plate.scad` | the snap-on back cover | flat, rear face down |
| `easel.scad` | the stand — already in its print orientation | as it comes |
| `board.scad` | the board itself, for reference | not a printed part |

All the real geometry lives in one place, `case.scad`, which holds the bezel, the
plate and the easel; those first three files are four-line wrappers that include
it. `case.scad` also pulls in `board.scad` for the module, and can render assembly,
section and interference views — the `PART` variable at the top of it picks which.

Regenerate the STLs with:

```
openscad -o bezel.stl bezel.scad
openscad -o plate.stl plate.scad
openscad -o easel.stl easel.scad
```

Renders are in `renders/` if you want to see what you are printing first.

---

## Print settings

* 0.2 mm layers, 3 perimeters, 20–30 % infill — nothing here is structural.
* Neither part needs supports. The bezel is 15 mm tall in its print orientation,
  the easel about 20 mm.
* PLA or PETG both work. PETG is slightly better for the snaps (more give).
* The bezel's first layer is the rear rim (a closed 2.6 mm frame) plus the flat
  back of the foot, which is plenty for bed adhesion. A brim is not needed.

The bezel is printed **open-back-down** on purpose: in that orientation the pocket
is a converging cavity, the window chamfer becomes an up-facing 45° roof, and the
foot's surfaces are all vertical or up-facing. Printing it face-down is impossible
once the stand toe exists, because the toe sits *in front of* the screen plane.

---

## Assembly

1. **Drop the module in from behind.** Its cover glass lands on the 45° window
   chamfer, which traps it from the front, and the screen ends up coplanar with the
   case's front face.
2. The four pads on the plate press on the PCB's rear face. Each pad carries a short
   spigot that goes into a mounting hole, so the board cannot shift sideways.
3. **Push the plate on** until the four snaps click into the grooves in the side walls.
   The pads are 0.15 mm long, so the snapped plate bows very slightly and preloads the
   module instead of leaving it free to rattle.
4. **To open it**, hook the plate's edge out through the two pry reliefs in the side
   rims (the notches you can see near the back of each side wall). The barbs are
   two-way ramps, so it pops out about as easily as it snaps in.
5. **The easel is a separate part.** Drop the case's bottom end into it — the two
   cheeks grip the sides and a small rib makes the fit positive rather than sloppy.
   Lift the case straight out when you want it handheld.

---

## The mounting holes — what screws fit, and why they still can't hold the board

All four holes were measured on a calibrated product photo, across their centres:

| Hole | Ø (mm) |
|---|---|
| top-left | 1.57 |
| top-right | 1.57 |
| bottom-left | 1.62 |
| bottom-right | 1.51 |

**Ø1.55 mm, consistently, in a Ø3.65 mm plated pad.** That is exactly the ISO 273
*close* clearance hole for **M1.4**, and an M1.4 pan head (about Ø2.5) lands properly
on the pad. So:

| Screw | Close clearance | Passes a Ø1.55 hole? |
|---|---|---|
| M1.2 | 1.30 | yes, loosely |
| **M1.4** | **1.50** | **yes — this is the size** |
| M1.6 | 1.70 | no |
| M2 | 2.20 | no |
| M2.5 | 2.70 | no |
| M3 | 3.40 | no |

**M2, M2.5 and M3 screws will not even pass through the holes.**

That is not the whole problem, because even a correctly-sized screw cannot secure
the board here:

1. **The cover glass and LCD panel sit in front of the PCB and cover the holes.**
   Looking through a mounting hole in the product photo shows the dark LCD behind it,
   not daylight. So nothing can enter from the front, and the gap between the PCB and
   the LCD is only about 0.5 mm — there is no room for a nut either.
2. **From behind there is nothing to thread into.** The hole is just 1.1 mm deep (the
   PCB's own thickness) and then it is the LCD again.
3. **It would not even grip.** M1.4 threads are Ø1.4 — *smaller* than the Ø1.55 hole,
   so the screw slips straight through. You would need M1.6 to bite, and M1.6 will not
   pass.

So the holes keep their original job: **location only.** The two Ø1.2 spigots on the
plate drop into them (a pair, diagonally, so the board cannot spin) and stop it
sliding sideways. The module is held by clamping — the front lip traps the glass and
the plate pushes the PCB forward into it — which is why the plate needs a positive
snap rather than screws.

An alternative to the snaps would be to screw the plate down, but there is no room
for screw bosses inside the cavity (the glass fills it almost exactly). It would need
four bosses bulging about 2 mm out of the rear rim, visible on the outside. That
variant is not modelled.

---

## The easel — why the stand is a separate part

The case on its own **will not stand up**. At 75° it leans back far enough that the
module's mass ends up *in front of* the bottom-front edge, so it tips forward. Work
it out and the centre of mass projects about 2.5 mm ahead of the point where the
edge touches the desk.

Extending the base *backwards* does not fix this — that only adds rear contact.
Ballast would need roughly 20–30 g of extra plastic down there. What is actually
needed is base **in front of** the screen plane, and that is what `easel_toe` gives
you: 12 mm of it.

So the easel is a dock: a wedge the case's bottom end sits on, a forward toe for
stability, a front lip (2.6 mm, the tallest it can be before it would start
covering the glass), and two cheeks that grip the case's sides up to 18 mm. The
earlier revision had all of this built into the bezel with a fixed 8 mm toe — it
worked, but it made the case bulky to hold and the toe was short. Separate, the base
can be much larger.

The model prints the balance margin on every render — currently **+9.9 mm** with the
easel fitted, against +5.7 mm for the old built-in toe. It assumes a centre of mass
at `com_y = 2.6`, `com_z = 5.3` in the case frame: a reasonable guess for module
plus case, not a measured value.

* Tippy? Raise `easel_toe`.
* Smaller footprint? Lower it and accept a smaller margin.
* A USB-C cable hanging down in front adds forward load — route it back under the
  easel rather than over the front.

---

## Measured dimensions

Taken from Waveshare's dimension drawing for the board, calibrated against the
29.00 mm mounting-hole pitch, so these are consistent with each other rather than
absolute. The drawing and product photos they were measured from are Waveshare's and
are not included in this repository.

| Feature | Value |
|---|---|
| Cover glass / front module | 37.10 × 58.80 mm, corners R2.60, 1.10 mm thick |
| Active area | 31.00 × 41.20 mm, 3.06 mm from the left edge, 7.51 mm from the top edge |
| PCB | 35.00 × 48.20 × 1.10 mm; 4.80 mm below the glass top edge, 1.05 mm inboard each side |
| Mounting holes (×4) | centres 29.00 × 42.20 mm (3.00 mm from every PCB edge), **Ø1.50 hole in a Ø3.60 pad** |
| Headers | 2 × 14 at 2.54 mm, row centres 30.48 mm apart |
| Depth | glass front → PCB rear 7.20 mm, → header plastic 9.70 mm, → **pin tips 12.52 mm**, → USB-C shell 10.46 mm |
| USB-C | on the PCB's back at the board's bottom edge, ≈8.9 mm wide, ≈1.4 mm past the PCB edge |
| RP2350 | **RP2350A**, QFN-60, 7 × 7 mm (chip is marked `RP2350A0A2`) |
| BOOT / RST | tactile switches on the PCB's back either side of the USB-C, at y = −18.80 |

Two independent sources agree on these: Waveshare's dimension drawing, and a
1600 × 1200 product photo calibrated against the same 29.00 mm hole pitch. The two
scales they imply (18.8 and 17.86 px/mm) cross-check the pitch to better than 0.2 %,
which is what makes the absolute numbers trustworthy rather than merely self-consistent.

Derived case numbers (also echoed by `case.scad`):

| | |
|---|---|
| Case | 42.70 × 64.40 × **15.00** mm |
| Module pocket | 37.50 × 59.20 mm (0.20 mm clearance per side) |
| Front window | 36.40 × 58.10 mm (0.35 mm screen lip) |
| Back plate | 37.20 × 58.90 × 2.00 mm |
| Desk footprint | 23.81 mm deep |

### The two numbers that set the case's size

Neither is the USB-C, which is what you would expect:

* **The mounting holes are only Ø1.50 mm.** Modelling the board surfaced this. The
  earlier draft assumed Ø2.5 and sized its locating spigots at Ø2.20 — they would
  simply not have gone in. They are now Ø1.20. (`R3.00` in the drawing still doesn't
  reconcile with anything measurable: it is neither the hole, Ø6.00, nor the pad.)
* **The header pins are much longer than the “9.70” dimension suggests.** That 9.70
  is to the outer face of the header's *plastic body*; the bare pins carry on a
  further 2.82 mm, reaching **12.52 mm** from the screen. That is 5.32 mm behind the
  PCB, deeper than the USB-C at 3.26 mm. `cav_back` is sized from the pins.

The hole diameter, the pin length and the component layout were all read off the
calibrated photo.

---

## Firmware: the UI is rotated 180°

Because the USB-C has to end up at the top, the board goes in **rotated 180° from its
natural portrait orientation**. The firmware rotates the picture and the touch input
to match; this is `lcd::FLIPPED` in `src/board/pins.h`. Set it to `false` if you mount
the board the other way up.

---

## What is still a guess

Modelling the board removed most of the guesswork, but two things cannot be measured
off a photograph and are flagged as ESTIMATED in `board.scad`:

* **Component heights above the PCB** — taken from package types, not measured. The
  battery JST (modelled at 5.0 mm) is the tallest part after the pins, and it is the
  one to watch: the cavity leaves 5.8 mm behind the PCB, so it clears, but not by a lot.
* **The LCD panel outline behind the glass** (36.4 × 52 mm) — inferred, used for
  clearance only. It is well inside the pocket either way.

Everything else — the envelope, the hole, the headers, the USB-C, the switch
positions — is measured.

---

## Changing the design

Everything is parametric. Edit the sections at the top of `case.scad`:

* `[2. Case]` — walls, clearances, pocket depth
* `[3. Stand]` — `stand` (the screen angle) and `toe_lip`
* `[4. Plate retention]` — snap sizes and positions

The board's own dimensions are in `board.scad`, section `[1. Envelope]` onwards. If
your board measures differently, that is the only file to touch.

`PART` switches what is rendered: `bezel`, `plate`, `board` (the module on its own),
`assembly` (exploded, with a translucent module), `fit`, `section`, and `clash`.

**`clash` is the useful one.** It renders only the volume where the module and the
case overlap, so an interference is visible instead of having to be reasoned about.
Run it with `--render` (a CGAL render) rather than as a quick preview: the case's
front lip and the glass meet on exactly the same plane by design, and a preview
reports that zero-thickness contact as a large phantom sheet. A CGAL render discards
zero-volume intersections and shows only real clashes. What is legitimately left is
the four spigots inside the mounting holes and four thin discs where the pads are
preloaded into the PCB. Anything else is a genuine interference.
