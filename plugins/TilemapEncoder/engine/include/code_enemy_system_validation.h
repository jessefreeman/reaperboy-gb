#ifndef CODE_ENEMY_SYSTEM_VALIDATION_H
#define CODE_ENEMY_SYSTEM_VALIDATION_H

#include <gbdk/platform.h>

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

// Check if a position is valid for enemy placement
UBYTE is_valid_enemy_position(UBYTE x, UBYTE y) BANKED;

// Check if a specific position is valid for a specific enemy (avoids checking itself)
UBYTE is_position_valid_for_enemy(UBYTE enemy_index, UBYTE x, UBYTE y) BANKED;

// ============================================================================
// POSITION TRACKING SYSTEM
// ============================================================================

// Initialize the valid enemy position tracking system
void init_valid_enemy_positions(void) BANKED;

// Update valid enemy positions by scanning the entire level
void update_valid_enemy_positions(void) BANKED;

// Update valid enemy positions affected by a platform change
void update_enemy_positions_for_platform(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert from level row (0-3) to actual y position (12, 14, 16, 18)
UBYTE get_enemy_y_from_row(UBYTE row) BANKED;

// Convert from actual y position to level row (0-3)
UBYTE get_enemy_row_from_y(UBYTE y) BANKED;

// Check if a position has a platform below
UBYTE has_platform_below_enemy(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// ENEMY POSITION CYCLING
// ============================================================================

// Find the next valid position for an enemy in the level code system
void find_next_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED;

// Find the previous valid position for an enemy in the level code system
void find_prev_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED;

// ============================================================================
// PLATFORM INTERACTION
// ============================================================================

// Remove enemies above a deleted platform
void remove_enemies_above_deleted_platform(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// INTEGRATION WITH LEVEL CODE SYSTEM
// ============================================================================

// Get a valid POS41 value for an enemy
UBYTE get_valid_enemy_pos41(UBYTE enemy_index, UBYTE current_value) BANKED;

#endif // CODE_ENEMY_SYSTEM_VALIDATION_H
