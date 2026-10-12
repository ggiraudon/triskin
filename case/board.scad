// =============================================================================
//  Waveshare RP2350-Touch-LCD-2 — model of the module itself
// =============================================================================
//
//  Library file: it defines parameters and modules and renders NOTHING at top
//  level.  Include it from case.scad, then call board() / board(true).
//
//  Frame of reference (the same one case.scad uses):
//      x   width,  0 at the module centre, + to the right seen from the front
//      y   height, 0 at the module centre, + AWAY from the USB-C end
//      z   depth,  0 at the screen's front surface, + backwards
//
//  The board is modelled as fitted, i.e. with its 2x14 pin headers soldered on
//  the component side, which is how Waveshare ships it.
//
//  What is measured and what is not
//  --------------------------------
//  MEASURED (Waveshare dimension drawing, cross-checked against a calibrated
//  product photo — the two agree to ~0.2%):
//      cover glass, PCB outline and thickness, mounting-hole pitch and pad size,
//      header pitch/row spacing/body/pin length, USB-C width and height,
//      the module's depth stack, and the positions of the main components.
//
//  ESTIMATED (cannot be measured from photographs):
//      LCD panel outline behind the glass, component heights above the PCB,
//      and the active-area decal.  These are marked below.
// =============================================================================

/* [1. Envelope] */
glass_w  = 37.10;   // cover glass width             (MEASURED)
glass_h  = 58.80;   // cover glass height            (MEASURED)
glass_t  =  1.10;   // cover glass thickness         (MEASURED)
glass_r  =  2.60;   // cover glass corner radius     (MEASURED)
pcb_w    = 35.00;   // PCB width                     (MEASURED)
pcb_h    = 48.20;   // PCB height                    (MEASURED)
pcb_t    =  1.10;   // PCB thickness                 (MEASURED)
pcb_r    =  1.20;   // PCB corner radius             (ESTIMATED)
glass_top_pcb = 4.80;   // glass top edge -> PCB top edge (MEASURED)
z_pcb_f  =  6.10;   // screen front -> PCB front face    (MEASURED)
z_pcb_b  =  7.20;   // screen front -> PCB rear face     (MEASURED)

/* [2. Display] */
aa_w     = 31.00;   // active area width             (MEASURED)
aa_h     = 41.20;   // active area height            (MEASURED)
aa_x     =  3.06;   // glass left edge -> active area (MEASURED)
aa_y     =  7.51;   // glass top edge -> active area  (MEASURED)
lcd_w    = 36.40;   // LCD panel outline             (ESTIMATED)
lcd_h    = 52.00;   //   "      "                     (ESTIMATED)

/* [3. Mounting holes] */
hole_px  = 29.00;   // hole pitch across             (MEASURED)
hole_py  = 42.20;   // hole pitch up the board       (MEASURED)
hole_d   =  1.50;   // hole diameter                 (MEASURED — small!)
hole_pad =  3.60;   // plated annulus outer diameter (MEASURED)

/* [4. Pin headers] */
hdr_n     = 14;     // pins per row                  (MEASURED)
hdr_pitch = 2.54;   // pin pitch                     (MEASURED)
hdr_row   = 30.48;  // row centre spacing            (MEASURED)
hdr_body  = 2.50;   // plastic body thickness        (MEASURED)
hdr_pin   = 2.82;   // pin protrusion past the body  (MEASURED)
hdr_pin_d = 0.64;   // pin cross-section             (ESTIMATED)
hdr_pad_d = 1.70;   // PCB pad diameter              (ESTIMATED)

/* [5. USB-C] */
usbc_w    =  8.94;  // shell width                   (MEASURED)
usbc_len  =  7.60;  // shell length, inboard from the PCB edge (MEASURED)
usbc_h    =  3.26;  // shell height above the PCB    (standard part)
usbc_proj =  1.42;  // protrusion past the PCB edge  (MEASURED)

/* [Hidden] */
$fn = 32;
board_eps = 0.01;
pad_fn = 16;                 // pads are tiny; keep the facet count down

// ------------------------------------------------------------ derived -------
aa_cy    = glass_h / 2 - aa_y - aa_h / 2;   // active-area centre   (+1.29)
aa_cx    = -glass_w / 2 + aa_x + aa_w / 2;  // active-area centre   (0.00)
pcb_cy   = -(glass_h / 2) + glass_top_pcb + pcb_h / 2;        // +0.50
z_pins   = z_pcb_b + hdr_body + hdr_pin;                      // 12.52
z_usbc_b = z_pcb_b + usbc_h;                                  // 10.46
pcb_y0   = pcb_cy - pcb_h / 2;              // PCB bottom edge      (-23.60)
pcb_y1   = pcb_cy + pcb_h / 2;              // PCB top edge         (+24.60)
hdr_y0   = pcb_cy - hdr_n * hdr_pitch / 2;  // header row start

// ============================================================================
//  Geometry helpers
// ============================================================================
module br_rrect(w, h, r) { offset(r = r) square([w - 2 * r, h - 2 * r], center = true); }

module br_slab(w, h, r, z0, z1) {
    translate([0, 0, z0]) linear_extrude(z1 - z0) br_rrect(w, h, r);
}

// a component: a chamfered box standing on the PCB's rear face at (x, y)
module br_part(x, y, w, d, h, cham = 0.25) {
    translate([x, y, z_pcb_b]) hull() {
        translate([0, 0, board_eps])      cube([w - 2 * cham, d - 2 * cham, board_eps], center = true);
        translate([0, 0, cham])           cube([w, d, board_eps], center = true);
        translate([0, 0, h - cham])       cube([w, d, board_eps], center = true);
        translate([0, 0, h - board_eps])  cube([w - 2 * cham, d - 2 * cham, board_eps], center = true);
    }
}

// ============================================================================
//  Glass + display
// ============================================================================
module board_glass() {
    br_slab(glass_w, glass_h, glass_r, 0, glass_t);
}

// The lit part of the screen.  Drawn as a decal 0.02 mm proud of the glass so
// it is visible in renders; it lies well inside the case's front window, so it
// can never register as a clash against the case.
module board_active_area() {
    translate([aa_cx, aa_cy, 0]) br_slab(aa_w, aa_h, 0.40, 0, glass_t + 0.02);
}

module board_lcd() {
    br_slab(lcd_w, lcd_h, 2.0, glass_t, z_pcb_f);
}

// ============================================================================
//  PCB with its holes and pads
// ============================================================================
module board_pcb() {
    difference() {
        br_slab(pcb_w, pcb_h, pcb_r, z_pcb_f, z_pcb_b);
        // mounting holes
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * hole_px / 2, pcb_cy + sy * hole_py / 2, z_pcb_f - 1])
                cylinder(d = hole_d, h = pcb_t + 2, $fn = pad_fn);
        // pin header holes
        for (s = [-1, 1], n = [0 : hdr_n - 1])
            translate([s * hdr_row / 2, hdr_y0 + n * hdr_pitch + hdr_pitch / 2, z_pcb_f - 1])
                cylinder(d = hdr_pin_d + 0.25, h = pcb_t + 2, $fn = 10);
    }
}

// gold plating: mounting-hole annuli and header pads, on both faces
module board_pads(t = 0.06) {
    for (z = [z_pcb_b - board_eps - t, z_pcb_f + board_eps]) {
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * hole_px / 2, pcb_cy + sy * hole_py / 2, z])
                cylinder(d = hole_pad, h = t, $fn = pad_fn);
        for (s = [-1, 1], n = [0 : hdr_n - 1])
            translate([s * hdr_row / 2, hdr_y0 + n * hdr_pitch + hdr_pitch / 2, z])
                cylinder(d = hdr_pad_d, h = t, $fn = pad_fn);
    }
}

// ============================================================================
//  Pin headers — plastic body on the PCB's rear face, pins through it
// ============================================================================
module board_headers() {
    hdr_len = hdr_n * hdr_pitch;                 // 35.56
    for (s = [-1, 1]) {
        // plastic body
        translate([s * hdr_row / 2, pcb_cy, z_pcb_b])
            linear_extrude(hdr_body)
                br_rrect(hdr_pitch, hdr_len, 0.5);
        // pins
        for (n = [0 : hdr_n - 1])
            translate([s * hdr_row / 2, hdr_y0 + n * hdr_pitch + hdr_pitch / 2,
                       z_pcb_b - board_eps])
                cylinder(d = hdr_pin_d, h = hdr_body + hdr_pin + board_eps, $fn = 6);
    }
}

// ============================================================================
//  USB-C receptacle — on the PCB's rear face, mouth flush with the board edge
// ============================================================================
module board_usbc() {
    y_mouth = pcb_y0 - usbc_proj;
    translate([0, y_mouth, z_pcb_b]) {
        difference() {
            // shell: full section at the board edge, chamfered at the mouth
            hull() {
                translate([0, usbc_len, usbc_h / 2]) cube([usbc_w, board_eps, usbc_h], center = true);
                translate([0, 1.40, usbc_h / 2])     cube([usbc_w - 0.60, board_eps, usbc_h - 0.30], center = true);
                translate([0, 0.10, usbc_h / 2])     cube([usbc_w - 1.40, board_eps, usbc_h - 1.40], center = true);
            }
            // the plug opening
            translate([0, 2.0, usbc_h / 2]) cube([usbc_w - 1.90, 8.0, usbc_h - 1.60], center = true);
        }
        // the tongue inside the mouth
        translate([0, 3.4, usbc_h / 2]) cube([usbc_w - 3.00, 4.0, 0.70], center = true);
    }
}

// ============================================================================
//  Everything else on the component side
//  [x, y, w, d, h] in the case frame — positions MEASURED off the calibrated
//  photo, heights ESTIMATED from the package type.
// ============================================================================
BOARD_PARTS = [
    ["RP2350A 7x7 QFN-60",   -0.20,  15.20,  7.00, 7.00, 0.90],
    ["W25Q128 flash SOIC-8",  7.90,   4.90,  5.30, 4.00, 1.20],
    ["QMI8658 IMU LGA-14",   -1.30,  -0.30,  2.50, 2.50, 0.90],
    ["battery JST-PH",       -8.32,  -0.86,  7.90, 4.90, 5.00],
    ["LCD FPC connector",    -0.34, -10.52, 16.10, 4.90, 2.00],
    ["touch / camera FPC",    2.32, -15.95, 19.50, 5.80, 3.00],
    ["microSD card socket",  -0.34,   6.76, 11.50, 5.70, 2.60],
    ["JST bottom left",       9.46, -20.68,  4.50, 2.70, 3.50],
    ["JST bottom right",     -9.41, -20.93,  4.50, 2.40, 3.50],
    ["crystal 1.4x1.4",      -6.70,  15.70,  1.40, 1.40, 0.55],
    ["regulator SOT23-5 a",   9.50,  -0.60,  3.00, 1.60, 1.20],
    ["regulator SOT23-5 b",   7.60,  -0.60,  3.00, 1.60, 1.20],
    ["IC SOT23-6",          -13.64, -12.14,  1.62, 2.46, 1.20],
    ["IC SOT23-6",          -10.14,  -8.86,  1.12, 2.63, 1.20],
    ["IC 3x3 DFN",           -8.49,  -5.09,  3.00, 3.00, 0.90],
    ["IC 3x3 DFN",          -14.09,  18.63,  3.00, 3.00, 0.90],
    ["IC + shadow",          10.91,  -9.37,  3.25, 5.00, 1.10],
    ["IC + shadow",          -7.31,  11.32,  3.53, 3.00, 1.10]
];

// BOOT / RST tactile switches, either side of the USB-C
BOARD_SWITCHES = [
    ["RST",   9.85, -18.80],
    ["BOOT", -9.19, -18.80]
];

// passives: [x, y, w, d] — scattered where the real board has them, so the
// model reads like the real thing without pretending to be a netlist
BOARD_PASSIVES = [
    [-3.4, 19.4, 1.0, 0.5], [-3.4, 21.0, 1.0, 0.5], [ 1.6, 20.4, 1.6, 0.8],
    [ 5.9, 20.8, 1.0, 0.5], [ 8.9, 20.6, 1.6, 0.8], [ 9.4, 18.6, 1.6, 0.8],
    [11.6, 17.4, 0.8, 0.5], [12.1, 12.0, 1.6, 0.8], [-4.6, 12.6, 0.8, 0.5],
    [-6.4,  9.6, 1.6, 0.8], [-9.0, 10.4, 1.0, 0.5], [ 6.5,  9.4, 1.6, 0.8],
    [ 4.4,  6.0, 1.0, 0.5], [ 6.2,  6.0, 1.0, 0.5], [ 8.4,  8.2, 1.0, 0.5],
    [ 6.0,  4.2, 1.6, 0.8], [11.4,  4.0, 1.0, 0.5], [12.4,  1.0, 1.6, 0.8],
    [-5.4,  3.4, 1.0, 0.5], [-6.6,  1.2, 1.6, 0.8], [ 3.0,  2.2, 1.0, 0.5],
    [ 5.2, -1.8, 1.0, 0.5], [ 6.8, -3.6, 1.6, 0.8], [-4.2, -2.4, 1.6, 0.8],
    [ 3.6,  8.0, 1.0, 0.5], [-11.8, -4.6, 1.0, 0.5], [11.8, -7.6, 1.6, 0.8],
    [12.0, -14.0, 1.0, 0.5], [-12.2, -16.6, 1.0, 0.5], [ 4.2, -13.6, 1.0, 0.5],
    [-4.4, -13.2, 1.0, 0.5], [ 0.6, -18.4, 1.6, 0.8], [-2.8, -18.6, 1.6, 0.8],
    [ 6.8, -19.4, 1.0, 0.5], [-6.2, -22.4, 1.6, 0.8], [ 2.6, -22.6, 1.0, 0.5]
];

module board_components() {
    for (p = BOARD_PARTS) br_part(p[1], p[2], p[3], p[4], p[5]);
    for (s = BOARD_SWITCHES) br_part(s[1], s[2], 3.50, 3.50, 1.60, 0.30);
    for (p = BOARD_PASSIVES) br_part(p[0], p[1], p[2], p[3], 0.35, 0.08);
}

// ============================================================================
//  The finished module
// ============================================================================
module board(ghost = false) {
    if (!ghost) {
        color("black")   board_glass();
        color("grey")    board_lcd();
        color("darkgreen") board_pcb();
        color("gold")    board_pads();
        color("silver")  board_headers();
        color("silver")  board_usbc();
        color("black")   board_components();
        color("white")   board_active_area();
    } else {
        color("white", 0.45)     board_glass();
        color("grey", 0.55)      board_lcd();
        color("darkgreen", 0.65) board_pcb();
        color("gold", 0.7)       board_pads();
        color("silver", 0.7)     board_headers();
        color("silver", 0.7)     board_usbc();
        color("black", 0.6)      board_components();
    }
}

// Just the parts that stick out behind the PCB — handy for clearance work.
module board_behind() {
    union() {
        board_headers();
        board_usbc();
        board_components();
    }
}

// ----------------------------------------------------------------------------
//  Preview the module when this file is opened on its own.  case.scad sets
//  BOARD_LIB = true before including it, which suppresses this.
// ----------------------------------------------------------------------------
if (is_undef(BOARD_LIB) || !BOARD_LIB) board();
