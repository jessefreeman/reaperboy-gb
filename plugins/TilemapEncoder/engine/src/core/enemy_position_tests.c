#pragma bank 253

#include <gbdk/platform.h>
#include "enemy_position_manager.h"
#include "code_level_core.h"

// ============================================================================
// ENEMY POSITION MANAGER TESTS
// ============================================================================

// External level code structure
extern level_code_t current_level_code;

// Test function to verify enemy position validation
void test_enemy_position_validation(void) BANKED
{
    // Clear level code for clean test
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
        current_level_code.enemy_rows[i] = 255;
    }
    
    // Test 1: Valid position with platform below
    // Simulate platform at (2, 13) - should make (2, 12) valid for enemy
    // Note: In real usage, platform_positions would be set by update_platform_positions()
    
    // Test basic validation logic
    UBYTE test_x = 5;  // Column 3 in level code (5 - 2 = 3)
    UBYTE test_y = 12; // Enemy row 0
    
    // This should pass basic checks (position, row validation)
    // Platform check would depend on actual platform data
    UBYTE is_valid = is_valid_enemy_position_unified(test_x, test_y);
    
    // Test 2: Invalid position - wrong row
    UBYTE invalid_y = 11; // Not an enemy row
    UBYTE is_invalid = is_valid_enemy_position_unified(test_x, invalid_y);
    
    // Test 3: Position conversion utilities
    UBYTE row, col;
    tilemap_to_indices(test_x, test_y, &row, &col);
    
    UBYTE converted_x, converted_y;
    indices_to_tilemap(row, col, &converted_x, &converted_y);
    
    // converted_x should equal test_x, converted_y should equal test_y
    
    // Test 4: POS41 conversion
    UBYTE pos41 = indices_to_pos41(row, col, 0); // Even position
    
    UBYTE decoded_row, decoded_col;
    pos41_to_indices(pos41, 0, &decoded_row, &decoded_col);
    
    // decoded_row should equal row, decoded_col should equal col
}

// Test function to verify adjacent enemy detection
void test_adjacent_enemy_detection(void) BANKED
{
    // Clear level code
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
        current_level_code.enemy_rows[i] = 255;
    }
    
    // Place an enemy at position (5, 12) - column 3, row 0
    current_level_code.enemy_positions[0] = 3; // Column 3
    current_level_code.enemy_rows[0] = 0;      // Row 0
    
    // Test adjacent positions
    UBYTE has_adjacent_left = has_enemy_at_adjacent_positions(4, 12);  // Column 2 - should detect adjacent enemy
    UBYTE has_adjacent_right = has_enemy_at_adjacent_positions(6, 12); // Column 4 - should detect adjacent enemy
    UBYTE has_adjacent_same = has_enemy_at_adjacent_positions(5, 12);  // Column 3 - should NOT detect (same position)
    UBYTE has_adjacent_far = has_enemy_at_adjacent_positions(8, 12);   // Column 6 - should NOT detect (too far)
    
    // Test exact position detection
    UBYTE has_exact = has_enemy_at_exact_position(5, 12); // Should return 1
    UBYTE has_exact_wrong = has_enemy_at_exact_position(4, 12); // Should return 0
}

// Test function to verify position cycling
void test_position_cycling(void) BANKED
{
    // This test would require actual platform data to be meaningful
    // For now, just test the interface
    
    UBYTE next_row, next_col;
    UBYTE has_next = get_next_valid_enemy_position(0, 5, &next_row, &next_col);
    
    UBYTE prev_row, prev_col;
    UBYTE has_prev = get_prev_valid_enemy_position(0, 5, &prev_row, &prev_col);
    
    // Results depend on actual valid positions in the level
}

// Main test runner
void run_enemy_position_tests(void) BANKED
{
    test_enemy_position_validation();
    test_adjacent_enemy_detection();
    test_position_cycling();
}
