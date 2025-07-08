#ifndef ENEMY_POSITION_MANAGER_H
#define ENEMY_POSITION_MANAGER_H

#include <gbdk/platform.h>

// ============================================================================
// ENEMY POSITION MANAGER - UNIFIED VALIDATION SYSTEM
// ============================================================================

// Include necessary headers for constants and structures
#include "code_level_core.h"
#include "tile_utils.h"

// External level code structure
extern level_code_t current_level_code;

// Shared enemy position constants
extern const UBYTE ENEMY_ROWS[4];
extern const UBYTE PLATFORM_ROWS[4];

// ============================================================================
// PLATFORM TRACKING SYSTEM
// ============================================================================

// Update platform positions cache when platforms change
void update_platform_positions(void) BANKED;

// Check if there's a platform directly below an enemy position (cached)
UBYTE has_platform_below_cached(UBYTE enemy_row, UBYTE col) BANKED;

// ============================================================================
// ENEMY POSITION VALIDATION
// ============================================================================

// Check if a position is valid for enemy placement (unified logic)
UBYTE is_valid_enemy_position_unified(UBYTE x, UBYTE y) BANKED;

// Check for enemies at adjacent positions (not including same position)
UBYTE has_enemy_at_adjacent_positions(UBYTE x, UBYTE y) BANKED;

// Check for enemy at exact position (for replacement/flipping)
UBYTE has_enemy_at_exact_position(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// VALID POSITIONS SYSTEM
// ============================================================================

// Update the valid enemy positions matrix
void update_valid_enemy_positions_unified(void) BANKED;

// Get the next valid enemy position for cycling in level code editor
UBYTE get_next_valid_enemy_position(UBYTE current_row, UBYTE current_col, UBYTE *next_row, UBYTE *next_col) BANKED;

// Get the previous valid enemy position for cycling in level code editor
UBYTE get_prev_valid_enemy_position(UBYTE current_row, UBYTE current_col, UBYTE *prev_row, UBYTE *prev_col) BANKED;

// ============================================================================
// POSITION CONVERSION UTILITIES
// ============================================================================

// Convert tilemap coordinates to array indices
void tilemap_to_indices(UBYTE x, UBYTE y, UBYTE *row, UBYTE *col) BANKED;

// Convert array indices to tilemap coordinates
void indices_to_tilemap(UBYTE row, UBYTE col, UBYTE *x, UBYTE *y) BANKED;

// Convert row/col to POS41 value for level code
UBYTE indices_to_pos41(UBYTE row, UBYTE col, UBYTE odd_bit) BANKED;

// Convert POS41 value to row/col
void pos41_to_indices(UBYTE pos_value, UBYTE odd_bit, UBYTE *row, UBYTE *col) BANKED;

// ============================================================================
// INTEGRATION WITH PLATFORM SYSTEM
// ============================================================================

// Called when a platform is added or removed
void on_platform_changed(UBYTE x, UBYTE y) BANKED;

// Called when paint tool places/removes enemies - validates and corrects positions
UBYTE validate_enemy_placement(UBYTE x, UBYTE y) BANKED;

#endif // ENEMY_POSITION_MANAGER_H
