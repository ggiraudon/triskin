// =============================================================================
//  Triskin — 3D printable case for the Waveshare RP2350-Touch-LCD-2
// =============================================================================
//
//  Two printed parts, both support-free:
//
//    bezel   Front body.  Holds the module and carries the stand foot.
//            PRINT IT WITH ITS REAR FACE (the open back) ON THE BED.
//    plate   Back cover.  Snaps into the bezel and presses the module's PCB
//            forward against the bezel.  Print flat, either way up.
//
//  How the module is held
//  ----------------------
//  The module drops into the bezel from behind.  Its cover glass lands on the
//  45 deg window chamfer, so the screen ends up FLUSH with the case's front
//  face and is trapped from the front.  The plate presses on the PCB's rear
//  face through four pads sitting over the four mounting holes; short spigots
//  on those pads poke into the holes so the board cannot slide.  Four
//  cantilever snaps hold the plate on.
//
//  The board's own mounting holes CANNOT take a screw from behind: the LCD
//  panel fills the whole gap in front of the PCB, so there is nowhere for a
//  nut or a heat-set insert.  They are used for location only — see README.md.
//
//  What sets the case's depth
//  --------------------------
//  Not the USB-C.  The 2x14 pin headers are soldered on the component side and
//  their pins reach z = 12.52 — 5.32 mm behind the PCB — deeper than anything
//  else on the board (the USB-C stops at 10.46).  `cav_back` is sized from the
//  pins.  If you don't fit headers you can drop cav_back to about 11.0 and the
//  case becomes 2 mm slimmer.
//
//  The module's own dimensions live in board.scad, which also provides board()
//  — an accurate model of the board used for the assembly view and for the
//  `clash` clearance check.
//
//  Frame of reference (everything below is built in it)
//  ---------------------------------------------------
//    X   width,   0 at the module centre, + to the right
//    Y   height,  0 at the module centre, + towards the bottom of the standing
//                 case, i.e. AWAY from the USB-C connector
//    Z   depth,   0 at the screen's front surface, + backwards
//
//  The board is mounted ROTATED 180 deg from its natural portrait orientation
//  so the USB-C ends up at the TOP of the standing case — the firmware must
//  rotate the UI and the touch axes by 180 deg.
//
//  Standing at `stand` degrees — the easel
//  ---------------------------------------
//  The case on its own will NOT stand up.  At 75 deg the module's mass sits in
//  front of the bottom-front edge, so it tips forward.  That is why the stand is
//  a SEPARATE part: `easel()` is a dock that the case's bottom end drops into —
//  a wedge, a forward toe supplying the missing stability, a front lip, and two
//  cheeks that grip the sides.  Lift the case straight out to use it handheld.
//  Increase `easel_toe` if it ever feels tippy.  See README.md.
//
//  (An earlier revision had this wedge built into the bezel with a fixed 8 mm
//  toe.  It worked, but it made the case bulky to hold and the toe was short.
//  As a separate part the base can be much larger, and the case stays clean.)
// =============================================================================

// PART picks what gets rendered.  In the OpenSCAD GUI this appears as a dropdown
// in the Customizer (Design -> Customizer); from the command line use
//   openscad -o out.stl -D 'PART="plate"' case.scad
// You can also skip this file entirely and just open one of the per-part
// wrappers — bezel.scad, plate.scad, easel.scad, board.scad.
PART    = "bezel";   // bezel | plate | board | easel | easel_print | assembly | docked | fit | section | clash | easel_clash

EXPLODE = 22;        // plate offset for PART = "assembly"

// The module's own dimensions, and an accurate model of it, live in board.scad
// — measured off Waveshare's dimension drawing and cross-checked against a
// calibrated product photo.  board() is the module; this file uses it for the
// assembly view and for every clearance check below.
BOARD_LIB = true;            // stops board.scad rendering itself
include <board.scad>

/* [6. Case] */
wall      = 2.60;    // side wall thickness
outer_r   = 5.00;    // outer corner radius
fit       = 0.20;    // module pocket clearance, per side
overlap   = 0.35;    // front window overlap onto the glass, per side
cav_back  = 13.00;   // inner cavity rear.  This is set by the HEADER PINS,
                     // which reach z = 12.52 — deeper than the USB-C (10.46).
                     // It also clears the battery JST, which is ~5 mm tall.
rim_t     = 2.00;    // back plate thickness
pad_pre   = 0.15;    // pad interference -> bows the plate, preloads the module

/* [7. Easel — the stand, printed separately] */
stand      = 75;     // screen angle to the desk surface, degrees
easel_toe  = 12.00;  // base reaches this far forward of the screen plane
easel_lip  =  2.60;  // front lip height; ~2.8 is the most before it covers glass
easel_wall =  3.00;  // cheek thickness
easel_slip =  0.15;  // cheek-to-case clearance, per side
easel_top  = 14.00;  // Y of the cheeks' top edge — smaller Y is a taller cheek
easel_grip =  0.30;  // friction rib on each cheek
easel_rib  = 17.00;  // Y of that rib
easel_sit  =  0.60;  // the seating plane sits this far above the case's bottom
                     // face.  DO NOT SET THIS TO 0.  If the seating plane and the
                     // desk plane meet at z = 0 they are TANGENT there, so the
                     // cross-section pinches to a zero-thickness edge and OpenSCAD
                     // cannot make a valid solid out of it — you get
                     //   WARNING: mesh is not closed!
                     //   ERROR: Unauthorized intersections of constraints
                     //   WARNING: No top level geometry to render

/* [8. Plate retention] */
snap_n    = 2;       // snaps per side
snap_yoff = 18.00;   // snap positions, +/- from the centre
snap_t    = 1.00;    // finger thickness (radial)
snap_w    = 5.00;    // finger width (along Y)
snap_clr  = 0.10;    // finger to wall clearance
barb_eng  = 0.35;    // barb protrusion (radial)
barb_h    = 1.00;    // half the barb's axial length (ramp run)
z_barb    = 9.50;    // Z of the barb's apex
groove_d  = 0.45;    // groove depth  (= barb_eng + 0.10)
groove_h  = 2.30;    // groove height (= 2 * barb_h + 0.30)
lead_in   = 1.50;    // rear entry flare, axial
lead_amt  = 0.60;    // rear entry flare, radial

/* [9. Features] */
cable_slot_w = 12.00;   // USB-C cable slot width
cable_slot_z =  5.00;   // slot starts this far behind the screen
cable_in_y   = -24.00;  // slot's inner end (must clear the plug's shoulder)
btn_holes    = true;    // BOOT / RST access holes in the plate
btn_x        = [9.85, -9.19];   // RST, BOOT — measured off a calibrated photo
btn_y        = -18.80;
btn_d        = 4.00;            // access hole — a pin reaches the switch through this
vents        = true;    // vent slots in the plate
spigots      = true;    // locating spigots into the board's mounting holes
prize_w      = 10.00;   // pry relief in the side rims
prize_d      =  1.50;

/* [Hidden] */
$fn = 64;
eps = 0.01;

// ------------------------------------------------------------ derived -------
ow    = glass_w + 2 * fit + 2 * wall;   // 42.70 outer width
oh    = glass_h + 2 * fit + 2 * wall;   // 64.40 outer height
cav_w = glass_w + 2 * fit;              // 37.50 pocket width
cav_h = glass_h + 2 * fit;              // 59.20 pocket height
cav_r = glass_r + fit;                  // 2.80
win_w = glass_w - 2 * overlap;          // 36.40 window width
win_h = glass_h - 2 * overlap;          // 58.10 window height
win_r = glass_r - overlap;              // 2.25
cham  = (cav_w - win_w) / 2;            // 0.55  45 deg chamfer depth
case_z = cav_back + rim_t;              // 15.00 total case depth
Y0     = oh / 2;                        // 32.20 case bottom edge (case frame)
spin   = 90 - stand;                    // 15 deg lean back from vertical

plate_w = cav_w - 2 * 0.15;             // 37.20
plate_h = cav_h - 2 * 0.15;             // 58.90
plate_r = cav_r - 0.15;                 // 2.65
finger_x0 = cav_w / 2 - snap_clr - snap_t;       // 17.65 inner face (outer 18.65)
snap_len  = cav_back - (z_barb - barb_h);        // 4.20 finger body length

// flare: over-extrude past the rear face so the cut is never coplanar with it,
// and grow the total width to 2*lead_amt exactly at z = case_z
flare_over = 0.80;
flare_grow = 2 * lead_amt * (lead_in + flare_over) / lead_in;
easel_ow   = ow + 2 * (easel_slip + easel_wall);   // 49.00

// desk plane, case frame: +Z going back, +Y going down
function yd(z) = Y0 + z * tan(spin);

// snap positions: +/-, evenly spread (snap_n = 2 -> +/- snap_yoff)
function snap_y(n) = snap_n == 1 ? 0
                                 : snap_yoff * (2 * n - (snap_n - 1)) / (snap_n - 1);

// ---------------------------------------------------------- primitives ------
module rrect(w, h, r) { offset(r = r) square([w - 2 * r, h - 2 * r], center = true); }

// ============================================================================
//  Bezel
// ============================================================================
module bezel_body() {
    difference() {
        // solid outer prism
        linear_extrude(case_z) rrect(ow, oh, outer_r);

        // 45 deg window chamfer, z = 0 .. cham (window size up to cavity size)
        linear_extrude(cham, scale = [cav_w / win_w, cav_h / win_h])
            rrect(win_w, win_h, win_r);

        // straight module pocket, z = cham .. through the back.
        //   Starts a hair early so the two cuts overlap instead of merely
        //   touching — that keeps the CGAL solid free of coincident faces.
        translate([0, 0, cham - 0.02])
            linear_extrude(case_z - cham + 1.02) rrect(cav_w, cav_h, cav_r);

        // rear entry flare (lead-in for the plate and for the snap barbs)
        translate([0, 0, case_z - lead_in])
            linear_extrude(lead_in + flare_over,
                scale = [(cav_w + flare_grow) / cav_w, (cav_h + flare_grow) / cav_h])
                rrect(cav_w, cav_h, cav_r);

        // snap grooves in both long walls
        for (s = [1, -1], n = [0 : snap_n - 1])
            scale([s, 1, 1]) translate([cav_w / 2, snap_y(n), z_barb]) snap_groove();

        // USB-C cable slot: through the top wall and the rear rim
        translate([-cable_slot_w / 2, -(oh / 2 + 2), cable_slot_z])
            cube([cable_slot_w, (oh / 2 + 2) + cable_in_y, case_z + 2]);

        // pry reliefs in the side rims, so the plate's edge can be hooked out
        // with a fingernail (kept off the bottom rim, where the foot is)
        for (s = [1, -1]) scale([s, 1, 1])
            translate([cav_w / 2, -prize_w / 2, case_z - prize_d])
                cube([wall + 2, prize_w, prize_d + 4]);
    }
}

// groove sunk into the cavity wall for one snap finger
module snap_groove() {
    translate([-eps, -snap_w / 2, -groove_h / 2])
        cube([groove_d + eps, snap_w, groove_h]);
}

module easel_wedge() {
    // Profile in the (Z, Y) plane; the desk plane is the underside, and the
    // seating plane is what the case's bottom face rests on.
    //
    // The seating plane sits `easel_sit` ABOVE the case's bottom face.  That
    // offset is load-bearing, not cosmetic: with no offset the seating plane and
    // the desk plane are tangent at z = 0, the cross-section pinches to a
    // zero-thickness knife edge there, and the resulting solid is not manifold.
    p = [
        [-easel_toe, yd(-easel_toe)],              // desk plane, front
        [ case_z,    yd(case_z)],                  // desk plane, rear
        [ case_z,    Y0 - easel_sit],              // up the rear face
        [ 0,         Y0 - easel_sit],              // seating plane, back to z = 0
        [ 0,         Y0 - easel_lip],              // up the front lip's inner face
        [-easel_toe, yd(-easel_toe) - easel_lip]   // lip top, running forward
    ];
    // local x -> world Z, local y -> world Y, extruded along world X
    rotate([0, -90, 0]) linear_extrude(easel_ow, center = true) polygon(p);
}

// cheeks: clamp the case's lower sides so it cannot rock, slide or fall out.
// They stop at z = 0 (the screen plane) so nothing ever hangs in front of the
// glass — only the low lip below the screen does that.
module easel_cheek() {
    p = [
        [ 0,      Y0],                 // front edge, flush with the case's face
        [ case_z, yd(case_z)],         // desk plane, rear
        [ case_z, easel_top],
        [ 0,      easel_top]
    ];
    rotate([0, -90, 0]) linear_extrude(easel_wall, center = true) polygon(p);
}

module easel() {
    intersection() {
        union() {
            easel_wedge();
            for (s = [1, -1]) scale([s, 1, 1])
                translate([ow / 2 + easel_slip + easel_wall / 2, 0, 0]) easel_cheek();
            // A small rib on each cheek so the fit feels positive instead of
            // sloppy.  It reaches `easel_grip - easel_slip` INTO the case, which
            // is intended — that interference is where the grip comes from.
            for (s = [1, -1]) scale([s, 1, 1])
                translate([ow / 2 + easel_slip - easel_grip, easel_rib - 0.60, 1.60])
                    cube([easel_grip, 1.20, 2.80]);
        }
        // keep the sides straight and the width to easel_ow
        translate([0, 0, -easel_toe - 5])
            linear_extrude(case_z + easel_toe + 10) square([easel_ow, 130], center = true);
    }
}

module bezel() { bezel_body(); }

// ============================================================================
//  Back plate
// ============================================================================
module pad() {
    // boss pressing on the PCB's rear face.  It reaches `pad_pre` past the PCB
    // so the snapped plate is forced to bow slightly, which preloads the module
    // instead of leaving it free to rattle in the pocket.
    translate([0, 0, z_pcb_b - pad_pre])
        cylinder(d = 5.50, h = cav_back - z_pcb_b + pad_pre + 0.50);
    // locating spigot, kept inside the PCB's own thickness.  Deliberately short:
    // it only has to stop the board sliding, and the LCD panel sits just in
    // front of the PCB, so a long spigot risks hitting it.
    if (spigots)
        translate([0, 0, z_pcb_f + 0.40])
            cylinder(d = hole_d - 0.30, h = (z_pcb_b - z_pcb_f) - 0.40 + eps);
}

// one cantilever snap: root on the plate's front face, barb at the free end
module snap_finger() {
    // body
    translate([0, -snap_w / 2, z_barb - barb_h])
        cube([snap_t, snap_w, snap_len + eps]);
    // barb — a two-way ramp so the plate snaps in AND pops back out
    translate([snap_t, 0, z_barb]) rotate([90, 0, 0])
        linear_extrude(snap_w, center = true)
            polygon([[0, -barb_h], [barb_eng, 0], [0, barb_h]]);
}

module plate() {
    difference() {
        union() {
            translate([0, 0, cav_back]) linear_extrude(rim_t) rrect(plate_w, plate_h, plate_r);
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * hole_px / 2, pcb_cy + sy * hole_py / 2, 0]) pad();
            for (s = [1, -1], n = [0 : snap_n - 1])
                scale([s, 1, 1]) translate([finger_x0, snap_y(n), 0]) snap_finger();
        }
        // USB-C cable slot
        translate([-cable_slot_w / 2, -(oh / 2 + 2), cable_slot_z])
            cube([cable_slot_w, (oh / 2 + 2) + cable_in_y, case_z + 2]);
        // BOOT / RST access
        if (btn_holes)
            for (x = btn_x)
                translate([x, btn_y, cav_back - 1]) cylinder(d = btn_d, h = rim_t + 2);
        // vents
        if (vents)
            for (y = [10, 16, 22])
                translate([0, y, cav_back - 1]) hull() {
                    translate([-7.5, 0, 0]) cylinder(d = 2.0, h = rim_t + 2);
                    translate([ 7.5, 0, 0]) cylinder(d = 2.0, h = rim_t + 2);
                }
    }
}

// ============================================================================
//  The module itself lives in board.scad.  board(true) is the translucent
//  version used for the assembly view and the clearance checks.
// ============================================================================
module module_ghost() { board(true); }

// The easel in its print orientation: desk face horizontal and sitting exactly on
// Z = 0, so the exported STL drops straight onto the bed.
//   rotate([spin - 90, 0, 0]) puts the desk face's outward normal on -Z;
//   the translate then lifts it from -Y0*cos(spin) up to 0.
module easel_print() {
    translate([0, 0, Y0 * cos(spin)]) rotate([spin - 90, 0, 0]) easel();
}

// The case sits `easel_sit` further up once it is in the easel — see the note
// on that parameter.  Use this for the docked and clearance views so the case
// and the easel line up the way they will in your hand.
module case_in_easel() {
    translate([0, -easel_sit, 0]) { bezel(); plate(); }
}

// ============================================================================
//  Parameter report — echoed on every render so the numbers stay honest when
//  something in sections 1-4 is changed.
// ============================================================================
com_y = 2.60;    // ASSUMED centre of mass of the module plus the case together
com_z = 5.30;    //   "      "      "      "      "      "      "      "

s_com   = (com_y - Y0) * sin(spin) + com_z * cos(spin);   // along the desk plane
s_front = -easel_toe / cos(spin);                         // front contact point

for (t = [
    str("case outer       : ", ow, " x ", oh, " x ", case_z, " mm"),
    str("module pocket    : ", cav_w, " x ", cav_h, " mm   (glass ", glass_w, " x ", glass_h, ")"),
    str("front window     : ", win_w, " x ", win_h, " mm   (screen lip ", overlap, " mm)"),
    str("back plate       : ", plate_w, " x ", plate_h, " x ", rim_t, " mm"),
    str("screen angle     : ", stand, " deg from the desk"),
    str("easel footprint  : ", (easel_toe + case_z) / cos(spin), " mm deep"),
    str("easel lip        : ", easel_lip, " mm high, toe ", easel_toe, " mm forward"),
    str("print height     : ", case_z, " mm   (bezel, open back down)"),
    str("balance margin   : ", s_com - s_front,
        " mm   (easel fitted; the case alone does not stand up)")
]) echo(t);

// ============================================================================
//  Render — skipped when this file is included by a per-part wrapper
//  (bezel.scad / plate.scad / easel.scad), which set CASE_LIB first.
// ============================================================================
if (is_undef(CASE_LIB) || !CASE_LIB) {

if (PART == "bezel")       bezel();
if (PART == "plate")       plate();
if (PART == "board")       board();
if (PART == "easel")       easel();
if (PART == "easel_print") easel_print();   // desk face flat on the bed
if (PART == "assembly") { bezel(); translate([0, 0, EXPLODE]) plate(); module_ghost(); }
if (PART == "docked")   { easel(); case_in_easel(); module_ghost(); }
if (PART == "fit")      { bezel(); plate(); module_ghost(); }
if (PART == "section")  difference() {
    union() { bezel(); module_ghost(); }
    translate([-ow, -oh, -20]) cube([ow / 2, 2 * oh, case_z + 40]);
}
// FIT CHECK — renders only the volume where the module and the case overlap,
// so an interference is visible instead of having to be reasoned about.  Expect
// just the four spigots inside the mounting holes, four thin discs where the
// pads are preloaded into the PCB, and the zero-thickness contact where the
// glass seats on the front lip.  Anything else IS a real clash.
//   Run it with --render: in preview mode the coplanar glass/lip contact shows
//   up as a big phantom sheet that looks exactly like a disaster.
if (PART == "clash") intersection() {
    board();
    union() { bezel(); plate(); }
}
// The easel must clear the case too — this catches a slip fit that came out
// tight instead of loose, which would otherwise only show up at assembly.
// Expect only the two grip ribs, which are meant to interfere.
if (PART == "easel_clash") intersection() {
    easel();
    case_in_easel();
}

}
