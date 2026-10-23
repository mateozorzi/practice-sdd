#ifndef STREETS_TEST_UTILS_H
#define STREETS_TEST_UTILS_H

// Street grids and bridge crossings of the milestone 5 Background, built in code (10 px cells,
// centers at multiples of 10). Separate from server_test_utils.h because it includes server
// headers.

#include "../server/streets_grid.h"

constexpr int TEST_CELL_SIZE = 10;

// Straight street, horizontal from (0, 0) to (200, 0)
StreetMap make_straight_street_map();

// L street: from (0, 0) to (200, 0) and from (200, 0) to (200, 200)
StreetMap make_l_street_map();

// The straight street plus another horizontal street from (0, 100) to (200, 100), not connected
StreetMap make_two_streets_map();

// Bridge in (100..160, 100..160). Top road: rows y = 110..150, x from 0 to 260, ON_BRIDGE sensor
// (40..220, 110..150) and rails at y = 100 and y = 160. Bottom road: columns x = 110..150, y from 0
// to 260, UNDER_BRIDGE sensor (110..150, 40..220). Detour: column x = 260 (y 110..260) and row
// y = 260 (x 110..260), outside the crossing.
StreetMap make_test_crossing_map();

// Bridge over the water in (100..160, 100..160): only the top road of the crossing, with its
// ON_BRIDGE sensor (40..220, 110..150) and rails. No UNDER_BRIDGE sensor and no bottom road.
StreetMap make_water_bridge_map();

#endif
