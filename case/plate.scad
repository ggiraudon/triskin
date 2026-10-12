// =============================================================================
//  Triskin case — BACK PLATE
// =============================================================================
//
//  Open this file and press F6 to render just the back plate, or:
//      openscad -o plate.stl plate.scad
//
//  This is a thin wrapper: all the geometry genuinely lives in case.scad, which
//  holds the bezel, the plate and the easel, and case.scad in turn includes
//  board.scad for the module.  Setting CASE_LIB first stops case.scad rendering
//  its own default part.
//
//  Printed flat, either side down.  It is symmetric in Z apart from the pads
//  and the snap fingers, so put the rear face down: that leaves the fingers
//  pointing up and nothing hangs over the bed.
// =============================================================================

CASE_LIB = true;
include <case.scad>

plate();
