// =============================================================================
//  Triskin case — EASEL (the stand)
// =============================================================================
//
//  Open this file and press F6 to render just the easel, or:
//      openscad -o easel.stl easel.scad
//
//  This is a thin wrapper: all the geometry genuinely lives in case.scad, which
//  holds the bezel, the plate and the easel, and case.scad in turn includes
//  board.scad for the module.  Setting CASE_LIB first stops case.scad rendering
//  its own default part.
//
//  PRINT ORIENTATION — already applied below, so what you see is what you
//  slice: the desk face is flat on the bed, exactly at Z = 0, and the cheeks
//  rise almost vertically.  Support-free.
//
//  In its WORKING orientation the easel holds the screen at `stand` degrees,
//  with the toe forward for stability.  To see that, comment out the rotate()
//  and the translate() on the last line.
// =============================================================================

CASE_LIB = true;
include <case.scad>

easel_print();
