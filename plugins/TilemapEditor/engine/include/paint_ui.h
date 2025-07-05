#ifndef PAINT_UI_H
#define PAINT_UI_H

#include <gbdk/platform.h>

// ============================================================================
// SELECTOR STATE CONSTANTS
// ============================================================================

#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_RIGHT 2
#define SELECTOR_STATE_ENEMY_LEFT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLAYER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7

// ============================================================================
// BRUSH STATE FUNCTIONS
// ============================================================================

// Get the brush state for a position
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED;

#endif // PAINT_UI_H
