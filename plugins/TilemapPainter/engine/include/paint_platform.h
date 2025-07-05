#ifndef PAINT_PLATFORM_H
#define PAINT_PLATFORM_H

#include <gbdk/platform.h>

// ============================================================================
// PLATFORM VALIDATION FUNCTIONS
// ============================================================================

// Check if a row is valid for platform placement
UBYTE is_valid_platform_row(UBYTE y) BANKED;

// Check if a platform exists below the specified position
UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED;

// Check if a platform exists directly below the specified position
UBYTE has_platform_directly_below(UBYTE x, UBYTE y) BANKED;

// Check for vertical conflicts with other platforms
UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y) BANKED;

// Check if a platform can be placed at the specified position
UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED;

// Determine the type of platform placement possible at a position
UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// PLATFORM LENGTH CALCULATION
// ============================================================================

// Calculate the length of a connected platform including the specified position
UBYTE count_connected_platform_length(UBYTE x, UBYTE y) BANKED;

// Check if placing a 2-tile platform would exceed the length limit after merging
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// PLATFORM MANIPULATION
// ============================================================================

// Rebuild a platform row after modifications
void rebuild_platform_row(UBYTE y) BANKED;

// Update player position tracking when a platform is painted
void update_column_platform_painted(UBYTE x, UBYTE y) BANKED;

// Update player position tracking when a platform is deleted
void update_column_platform_deleted(UBYTE x, UBYTE y) BANKED;

#endif // PAINT_PLATFORM_H
