#pragma bank 253

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"
#include "paint.h"

// ============================================================================
// BRUSH-BASED VALIDATION FUNCTIONS
// ============================================================================

// External references to brush validation functions
extern UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED;

// Test if the brush would allow painting an enemy at coordinates corresponding to a POS41 value
UBYTE is_pos41_value_brush_valid(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit) BANKED
{
    if (pos_value == 0)
        return 1; // 0 = no enemy is always valid

    if (pos_value > 40)
        return 0; // Invalid POS41 range

    // Convert POS41 value to row and column
    UBYTE v = pos_value - 1; // 0-39
    UBYTE row = v / 10;      // 0-3
    UBYTE anchor = v % 10;   // 0-9

    // Calculate actual column: anchor*2 + odd_bit
    UBYTE col = anchor * 2 + odd_bit;

    if (row >= 4 || col >= 20)
        return 0; // Invalid coordinates

    // Convert to tilemap coordinates
    UBYTE x = PLATFORM_X_MIN + col;
    UBYTE y;
    
    // Use the same row-to-Y mapping as the paint system
    if (row == 0)
        y = 12;
    else if (row == 1)
        y = 14;
    else if (row == 2)
        y = 16;
    else
        y = 18;

    // Temporarily clear this enemy's position to avoid self-conflict during validation
    UBYTE old_pos = current_level_code.enemy_positions[enemy_index];
    UBYTE old_row = current_level_code.enemy_rows[enemy_index];
    current_level_code.enemy_positions[enemy_index] = 255;
    current_level_code.enemy_rows[enemy_index] = 255;

    // Test if the brush would allow painting an enemy here
    UBYTE result = can_paint_enemy_right(x, y);

    // Restore the enemy's original position
    current_level_code.enemy_positions[enemy_index] = old_pos;
    current_level_code.enemy_rows[enemy_index] = old_row;

    return result;
}

// Find the next valid POS41 value by testing brush validation
UBYTE find_next_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED
{
    // First check if the current position is valid (needed for when users try to enter an invalid position)
    if (current_pos > 0 && current_pos <= 40)
    {
        if (is_pos41_value_brush_valid(enemy_index, current_pos, odd_bit))
            return current_pos;
    }
    
    // Start searching from the next position after current
    UBYTE start_pos = (current_pos == 0) ? 1 : current_pos + 1;
    
    // Search forward from start_pos to 40
    for (UBYTE pos = start_pos; pos <= 40; pos++)
    {
        if (is_pos41_value_brush_valid(enemy_index, pos, odd_bit))
            return pos;
    }
    
    // If nothing found forward, search from 1 to current_pos
    for (UBYTE pos = 1; pos < start_pos; pos++)
    {
        if (is_pos41_value_brush_valid(enemy_index, pos, odd_bit))
            return pos;
    }
    
    // If no valid position found, return 0 (no enemy)
    return 0;
}

// Find the previous valid POS41 value by testing brush validation
UBYTE find_prev_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED
{
    // First check if the current position is valid (needed for when users try to enter an invalid position)
    if (current_pos > 0 && current_pos <= 40)
    {
        if (is_pos41_value_brush_valid(enemy_index, current_pos, odd_bit))
            return current_pos;
    }
    
    // Start searching from the previous position before current
    UBYTE start_pos = (current_pos <= 1) ? 40 : current_pos - 1;
    
    // Search backward from start_pos to 1
    for (UBYTE pos = start_pos; pos >= 1; pos--)
    {
        if (is_pos41_value_brush_valid(enemy_index, pos, odd_bit))
            return pos;
    }
    
    // If nothing found backward, search from 40 down to current_pos
    for (UBYTE pos = 40; pos > start_pos; pos--)
    {
        if (is_pos41_value_brush_valid(enemy_index, pos, odd_bit))
            return pos;
    }
    
    // If no valid position found, return 0 (no enemy)
    return 0;
}

// ============================================================================
// BRUSH-BASED CODE CYCLING FUNCTIONS
// ============================================================================

// Cycle to the next valid enemy code for a specific character position
// Used when user presses right arrow or cycles through codes
UBYTE cycle_to_next_valid_enemy_code(UBYTE char_index) BANKED
{
    if (char_index < 17 || char_index > 23)
        return 0; // Not an enemy character

    UBYTE rel_index = char_index - 17; // Convert to 0-6 range

    if (rel_index < 5) // Position characters (17-21)
    {
        // Get current values
        UBYTE current_pos = encode_enemy_position(rel_index);
        UBYTE odd_mask = encode_odd_mask_value();
        UBYTE odd_bit = (odd_mask >> rel_index) & 1;

        // Find next valid position using brush validation
        UBYTE next_pos = find_next_brush_valid_pos41(rel_index, current_pos, odd_bit);
        return next_pos;
    }
    else if (rel_index == 5) // Odd mask (character 22)
    {
        // For odd mask, we need to find the next valid mask value
        // This is more complex as changing odd bits affects all enemy positions
        UBYTE current_mask = encode_odd_mask_value();
        
        // Try all possible mask values from current+1 to 31, then 0 to current
        for (UBYTE mask = (current_mask + 1) & 0x1F; mask != current_mask; mask = (mask + 1) & 0x1F)
        {
            // Test if this mask would make all current enemies valid
            UBYTE all_valid = 1;
            for (UBYTE i = 0; i < 5; i++)
            {
                UBYTE pos_value = encode_enemy_position(i);
                if (pos_value > 0) // If this enemy exists
                {
                    UBYTE new_odd_bit = (mask >> i) & 1;
                    if (!is_pos41_value_brush_valid(i, pos_value, new_odd_bit))
                    {
                        all_valid = 0;
                        break;
                    }
                }
            }
            
            if (all_valid)
                return mask;
        }
        
        // If no valid mask found, return current
        return current_mask;
    }
    else // Direction mask (character 23)
    {
        // Direction changes don't affect brush validation, so just cycle through values
        UBYTE current_dir = encode_enemy_directions();
        return (current_dir + 1) & 0x1F; // Cycle 0-31
    }
}

// Cycle to the previous valid enemy code for a specific character position
// Used when user presses left arrow or cycles backwards through codes
UBYTE cycle_to_prev_valid_enemy_code(UBYTE char_index) BANKED
{
    if (char_index < 17 || char_index > 23)
        return 0; // Not an enemy character

    UBYTE rel_index = char_index - 17; // Convert to 0-6 range

    if (rel_index < 5) // Position characters (17-21)
    {
        // Get current values
        UBYTE current_pos = encode_enemy_position(rel_index);
        UBYTE odd_mask = encode_odd_mask_value();
        UBYTE odd_bit = (odd_mask >> rel_index) & 1;

        // Find previous valid position using brush validation
        UBYTE prev_pos = find_prev_brush_valid_pos41(rel_index, current_pos, odd_bit);
        return prev_pos;
    }
    else if (rel_index == 5) // Odd mask (character 22)
    {
        // For odd mask, find the previous valid mask value
        UBYTE current_mask = encode_odd_mask_value();
        
        // Try all possible mask values from current-1 down to 0, then 31 down to current
        for (UBYTE mask = (current_mask - 1) & 0x1F; mask != current_mask; mask = (mask - 1) & 0x1F)
        {
            // Test if this mask would make all current enemies valid
            UBYTE all_valid = 1;
            for (UBYTE i = 0; i < 5; i++)
            {
                UBYTE pos_value = encode_enemy_position(i);
                if (pos_value > 0) // If this enemy exists
                {
                    UBYTE new_odd_bit = (mask >> i) & 1;
                    if (!is_pos41_value_brush_valid(i, pos_value, new_odd_bit))
                    {
                        all_valid = 0;
                        break;
                    }
                }
            }
            
            if (all_valid)
                return mask;
        }
        
        // If no valid mask found, return current
        return current_mask;
    }
    else // Direction mask (character 23)
    {
        // Direction changes don't affect brush validation, so just cycle through values
        UBYTE current_dir = encode_enemy_directions();
        return (current_dir - 1) & 0x1F; // Cycle 0-31
    }
}

// Check if a specific enemy code value would be valid according to brush system
UBYTE is_enemy_code_brush_valid(UBYTE char_index, UBYTE test_value) BANKED
{
    if (char_index < 17 || char_index > 23)
        return 0; // Not an enemy character

    UBYTE rel_index = char_index - 17; // Convert to 0-6 range

    if (rel_index < 5) // Position characters (17-21)
    {
        if (test_value > 40)
            return 0; // Invalid POS41 range

        // Get the odd bit for this enemy
        UBYTE odd_mask = encode_odd_mask_value();
        UBYTE odd_bit = (odd_mask >> rel_index) & 1;

        // Test if the brush would allow this position
        return is_pos41_value_brush_valid(rel_index, test_value, odd_bit);
    }
    else if (rel_index == 5) // Odd mask (character 22)
    {
        if (test_value > 31)
            return 0; // Invalid BASE32 range

        // Test if this mask would make all current enemies valid
        for (UBYTE i = 0; i < 5; i++)
        {
            UBYTE pos_value = encode_enemy_position(i);
            if (pos_value > 0) // If this enemy exists
            {
                UBYTE new_odd_bit = (test_value >> i) & 1;
                if (!is_pos41_value_brush_valid(i, pos_value, new_odd_bit))
                    return 0;
            }
        }
        return 1;
    }
    else // Direction mask (character 23)
    {
        // Direction changes don't affect brush validation
        return (test_value <= 31);
    }
}

// ============================================================================
// ACTOR MANAGEMENT FUNCTIONS
// ============================================================================

// External references from paint.c
extern UBYTE paint_enemy_ids[5];
extern UBYTE paint_enemy_slots_used[5];
extern actor_t actors[];
extern void activate_actor(actor_t *actor) BANKED;
extern void deactivate_actor(actor_t *actor) BANKED;
extern void actor_set_dir(actor_t *actor, UBYTE dir, UBYTE moving) BANKED;

// External references from painting system
extern UBYTE get_next_enemy_slot_from_pool(void) BANKED;
extern void reset_enemy_pool(void) BANKED;

// External reference to script memory for mode checking
extern UWORD script_memory[];

// Direction constants
#define DIRECTION_DOWN 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_LEFT 3

// Convert to fixed point (from paint.c)
#define TO_FP(n) ((INT16)((n) << 4))

// Note: The following functions were moved to code_enemy_system.c to avoid duplication:
// - clear_enemy_actor()
// - place_enemy_actor() 
// - restore_enemy_actors_from_level_code()

// External function declarations from code_enemy_system.c
extern void clear_enemy_actor(UBYTE enemy_index) BANKED;
extern void place_enemy_actor(UBYTE enemy_index, UBYTE tilemap_x, UBYTE tilemap_y, UBYTE direction) BANKED;
extern void restore_enemy_actors_from_level_code(void) BANKED;
