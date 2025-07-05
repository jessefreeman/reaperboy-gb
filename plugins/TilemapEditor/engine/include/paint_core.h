#ifndef PAINT_CORE_H
#define PAINT_CORE_H

#include <gbdk/platform.h>
#include "meta_tiles.h"

// ============================================================================
// CORE CONSTANTS
// ============================================================================

// Platform-specific constants not in core headers
#define PLATFORM_MIN_VERTICAL_GAP 1
#define PLATFORM_MAX_LENGTH 8

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Inline utility for repeated boundary checks
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y) BANKED;

// Cached tile access to reduce repeated lookups
UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED;

// Convert subpixels
#define TO_FP(n) ((INT16)((n) << 4))

// ============================================================================
// CORE PAINTING FUNCTIONS
// ============================================================================

// Main painting function - delegates to specific handlers
void paint(UBYTE x, UBYTE y) BANKED;

// Level code update based on painting
void update_level_code_for_paint(UBYTE x, UBYTE y) BANKED;

// Default level initialization
UBYTE is_map_empty(void) BANKED;
void init_default_level_code(void) BANKED;

#endif // PAINT_CORE_H
