// =============================================================================
//  Triskin case — BEZEL (the front body)
// =============================================================================
//
//  Open this file and press F6 to render just the bezel, or:
//      openscad -o bezel.stl bezel.scad
//
//  This is a thin wrapper: all the geometry genuinely lives in case.scad, which
//  holds the bezel, the plate and the easel, and case.scad in turn includes
//  board.scad for the module.  Setting CASE_LIB first stops case.scad rendering
//  its own default part.
//
//  PRINT ORIENTATION — this file renders the bezel in its working orientation.
//  For printing, put the OPEN BACK on the bed (rotate 180 deg about X).
// =============================================================================

CASE_LIB = true;
include <case.scad>

bezel();
