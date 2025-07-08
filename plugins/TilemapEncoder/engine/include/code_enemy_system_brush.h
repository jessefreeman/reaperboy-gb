#ifndef CODE_ENEMY_SYSTEM_BRUSH_H
#define CODE_ENEMY_SYSTEM_BRUSH_H

#include <gbdk/platform.h>
#include "code_enemy_system.h" // Include the main enemy system header

// ============================================================================
// BRUSH-BASED VALIDATION FUNCTIONS
// ============================================================================

// Test if a position would be valid according to the brush system
UBYTE is_pos41_value_brush_valid(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit) BANKED;

// Find next/previous valid positions using brush validation
UBYTE find_next_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED;
UBYTE find_prev_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED;

// Cycle through valid enemy codes
UBYTE cycle_to_next_valid_enemy_code(UBYTE char_index) BANKED;
UBYTE cycle_to_prev_valid_enemy_code(UBYTE char_index) BANKED;

// Check if a specific code value would be brush-valid
UBYTE is_enemy_code_brush_valid(UBYTE char_index, UBYTE test_value) BANKED;

// Note: Actor management functions are now defined in code_enemy_system.h

#endif // CODE_ENEMY_SYSTEM_BRUSH_H
