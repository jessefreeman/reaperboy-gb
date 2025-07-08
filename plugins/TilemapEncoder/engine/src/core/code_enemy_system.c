#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"
#include "paint.h"                        // Add this to access the validation functions
#include "code_enemy_system_validation.h" // Add the validation header

// ============================================================================
// ENEMY NUMERIC SYSTEM
// ============================================================================

// Level code enemy system constants
#define LEVEL_CODE_MAX_ENEMIES 5

// ============================================================================
// INITIALIZATION
// ============================================================================

// Initialize the enemy system including position validation
void init_enemy_system(void) BANKED
{
    // Initialize the validation system
    init_valid_enemy_positions();

    // Any other enemy system initialization can be added here
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Get the row (0-3) for an enemy at a given position index
UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES)
        return 0;

    // Use the actual row from the level code
    if (current_level_code.enemy_rows[enemy_index] != 255)
        return current_level_code.enemy_rows[enemy_index];

    // Default row if none is set
    return enemy_index % 4; // Distribute across rows 0-3
}

// ============================================================================
// ENEMY DATA EXTRACTION FROM TILEMAP
// ============================================================================

void extract_enemy_data(void) BANKED
{
    // Enemy data is now stored in level code structure, not as background tiles
    // In both edit and play modes, preserve the existing level code data
    // The level code structure (current_level_code) is the single source of truth for enemy data
    
    // No extraction needed - enemy data is already in current_level_code structure
    // and is managed directly by the painting/editing system
    return;
}

// ============================================================================
// ENEMY ENCODING FUNCTIONS
// ============================================================================

// Encode enemy position using numeric POS41 system
UBYTE encode_enemy_position(UBYTE enemy_index) BANKED
{
    // First check if the enemy index is valid and the position is valid
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES)
        return 0; // 0 = no enemy

    if (current_level_code.enemy_positions[enemy_index] == 255)
        return 0; // 0 = no enemy

    // Get the column and row
    UBYTE col = current_level_code.enemy_positions[enemy_index];
    UBYTE row = get_enemy_row_from_position(enemy_index);

    // Calculate index: 1 + row*10 + anchor (where anchor = col/2)
    UBYTE anchor = col / 2;
    UBYTE idx = 1 + row * 10 + anchor;

    // Safety check - value must be in range 1-40
    if (idx > 40)
        return 0; // Invalid index

    return idx;
}

// Encode odd-column parity mask for character 22
UBYTE encode_odd_mask(void) BANKED
{
    UBYTE odd_mask = 0;

    for (UBYTE k = 0; k < LEVEL_CODE_MAX_ENEMIES; k++)
    {
        if (current_level_code.enemy_positions[k] != 255)
        {
            UBYTE col = current_level_code.enemy_positions[k];
            if (col & 1) // If column is odd
            {
                odd_mask |= (1 << k);
            }
        }
    }

    return odd_mask & 0x1F;
}

// Encode direction mask for character 23
UBYTE encode_direction_mask(void) BANKED
{
    return current_level_code.enemy_directions & 0x1F;
}

// ============================================================================
// LEVEL CODE INTERFACE FUNCTIONS
// ============================================================================

// These functions are called by code_level_core.c to generate the level code display

UBYTE encode_enemy_positions(void) BANKED
{
    // Character 17: First enemy position (0-40)
    return encode_enemy_position(0);
}

UBYTE encode_enemy_details_1(void) BANKED
{
    // Character 18: Second enemy position (0-40)
    return encode_enemy_position(1);
}

UBYTE encode_enemy_details_2(void) BANKED
{
    // Character 19: Third enemy position (0-40)
    return encode_enemy_position(2);
}

UBYTE encode_enemy_position_4(void) BANKED
{
    // Character 20: Fourth enemy position (0-40)
    return encode_enemy_position(3);
}

UBYTE encode_enemy_position_5(void) BANKED
{
    // Character 21: Fifth enemy position (0-40)
    return encode_enemy_position(4);
}

UBYTE encode_odd_mask_value(void) BANKED
{
    // Character 22: Odd column parity mask (0-31)
    return encode_odd_mask();
}

UBYTE encode_enemy_directions(void) BANKED
{
    // Character 23: Direction mask (0-31)
    return encode_direction_mask();
}

// Compatibility alias

// ============================================================================
// ENEMY DECODING FUNCTIONS (for level code editing)
// ============================================================================

// Decode enemy position from a numeric value (0-40)
// Used for level code editing - only updates data structures and actors, never modifies background tiles
void decode_enemy_position(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit, UBYTE dir_bit) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES)
        return;

    // Clear previous enemy actor if it exists
    if (current_level_code.enemy_positions[enemy_index] != 255)
    {
        clear_enemy_actor(enemy_index);
    }

    if (pos_value == 0)
    {
        // 0 = no enemy
        current_level_code.enemy_positions[enemy_index] = 255;
        current_level_code.enemy_rows[enemy_index] = 255;
        return;
    }

    // Decode: pos_value = 1 + row*10 + anchor
    UBYTE v = pos_value - 1; // 0-39
    UBYTE row = v / 10;      // 0-3
    UBYTE anchor = v % 10;   // 0-9

    // Calculate actual column: anchor*2 + odd_bit
    UBYTE col = anchor * 2 + odd_bit;

    if (col > 19 || row > 3) // Safety check
    {
        current_level_code.enemy_positions[enemy_index] = 255;
        current_level_code.enemy_rows[enemy_index] = 255;
        return;
    }

    // Store both row and column
    current_level_code.enemy_positions[enemy_index] = col;
    current_level_code.enemy_rows[enemy_index] = row;

    // Set direction bit
    if (dir_bit)
    {
        current_level_code.enemy_directions |= (1 << enemy_index);
    }
    else
    {
        current_level_code.enemy_directions &= ~(1 << enemy_index);
    }

    // Update the enemy actor position (no background tile manipulation)
    UBYTE tilemap_x = PLATFORM_X_MIN + col;
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

    // Update the enemy actor position
    place_enemy_actor(enemy_index, tilemap_x, actual_y, dir_bit);
}

// Decode full enemy data from numeric values array
// Used for level code editing - only updates data structures and actors, never modifies background tiles
void decode_enemy_data_from_values(const UBYTE *enemy_values) BANKED
{
    // Clear all enemy actors but don't touch background tiles
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        clear_enemy_actor(i);
    }

    // Get masks directly (no character conversion needed)
    UBYTE odd_mask = enemy_values[5] & 0x1F; // Character 22
    UBYTE dir_mask = enemy_values[6] & 0x1F; // Character 23

    // Clear enemy data
    current_level_code.enemy_directions = 0;
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
        current_level_code.enemy_rows[i] = 255;
    }

    // Decode each enemy position (values 0-4 = positions 0-4)
    for (UBYTE k = 0; k < 5; k++)
    {
        UBYTE odd_bit = (odd_mask >> k) & 1;
        UBYTE dir_bit = (dir_mask >> k) & 1;
        decode_enemy_position(k, enemy_values[k], odd_bit, dir_bit);
    }
}

// ============================================================================
// LEVEL CODE EDITING SUPPORT
// ============================================================================

// Handle enemy data edit from level code (called when user edits character 17-23)
void handle_enemy_data_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    if (char_index < 17 || char_index > 23)
        return; // Not an enemy character

    // Build current enemy values array for decoding
    UBYTE enemy_values[7];

    // Get current encoded values directly
    enemy_values[0] = encode_enemy_positions();  // Character 17
    enemy_values[1] = encode_enemy_details_1();  // Character 18
    enemy_values[2] = encode_enemy_details_2();  // Character 19
    enemy_values[3] = encode_enemy_position_4(); // Character 20
    enemy_values[4] = encode_enemy_position_5(); // Character 21
    enemy_values[5] = encode_odd_mask_value();   // Character 22
    enemy_values[6] = encode_enemy_directions(); // Character 23

    // Update the edited value
    UBYTE rel_index = char_index - 17; // Convert to 0-6 range

    if (rel_index < 5) // Position characters (17-21)
    {
        // Position character (0-4) - validate position value
        if (new_value == 0)
        {
            // 0 = no enemy is always valid
            enemy_values[rel_index] = 0;
        }
        else if (new_value <= 40) // Valid POS41 range
        {
            // For position values, we need to check if this is a valid position
            // Convert POS41 value to row/column
            UBYTE v = new_value - 1; // 0-39
            UBYTE row = v / 10;      // 0-3
            UBYTE anchor = v % 10;   // 0-9

            // Get the odd bit for this enemy
            UBYTE odd_bit = (enemy_values[5] >> rel_index) & 1;

            // Calculate actual column: anchor*2 + odd_bit
            UBYTE col = anchor * 2 + odd_bit;

            if (row < 4 && col < 20)
            {
                // Temporarily set this position to see if it's valid
                UBYTE old_pos = current_level_code.enemy_positions[rel_index];
                UBYTE old_row = current_level_code.enemy_rows[rel_index];

                // Set temporary position to check validity
                current_level_code.enemy_positions[rel_index] = col;
                current_level_code.enemy_rows[rel_index] = row;

                // Convert to tilemap coordinates
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = 0;

                // Map row to actual y coordinate
                if (row == 0)
                    y = 12;
                else if (row == 1)
                    y = 14;
                else if (row == 2)
                    y = 16;
                else
                    y = 18;

                // Use our strict enemy position validation system instead
                if (is_valid_enemy_position(x, y))
                {
                    // Position is valid, use it
                    enemy_values[rel_index] = new_value;
                }
                else
                {
                    // Invalid position, use our validation system to find a valid position
                    enemy_values[rel_index] = get_valid_enemy_pos41(rel_index, new_value);
                }

                // Restore original position
                current_level_code.enemy_positions[rel_index] = old_pos;
                current_level_code.enemy_rows[rel_index] = old_row;
            }
            else
            {
                // Invalid row/column, keep old value
                enemy_values[rel_index] = encode_enemy_position(rel_index);
            }
        }
    }
    else // Mask characters (22-23)
    {
        if (new_value <= 31) // Valid BASE32 range
        {
            enemy_values[rel_index] = new_value;
        }
    }

    // Decode and apply the updated enemy data
    decode_enemy_data_from_values(enemy_values);
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

// Get tile ID for a POS41 value (0-40)
UBYTE get_pos41_display_tile(UBYTE value) BANKED
{
    // Use central mapping function from level core
    return pos41_value_to_tile_id(value);
}

// Get tile ID for a BASE32 value (0-31)
UBYTE get_base32_display_tile(UBYTE value) BANKED
{
    // Use central mapping function from level core
    return base32_value_to_tile_id(value);
}

// Convert tile ID to numeric value for level code editing
UBYTE enemy_char_to_value(UBYTE tile_id) BANKED
{
    // Check POS41 values (0-40)
    for (UBYTE i = 0; i < 41; i++)
    {
        if (pos41_value_to_tile_id(i) == tile_id)
            return i;
    }

    // Check BASE32 values (0-31)
    for (UBYTE i = 0; i < 32; i++)
    {
        if (base32_value_to_tile_id(i) == tile_id)
            return i;
    }

    return 0; // Default
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

// External reference to script memory for mode checking
extern UWORD script_memory[];

// Direction constants
#define DIRECTION_DOWN 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_LEFT 3

// Convert to fixed point (from paint.c)
#define TO_FP(n) ((INT16)((n) << 4))

// Clear a specific enemy actor
void clear_enemy_actor(UBYTE enemy_index) BANKED
{
    if (enemy_index >= MAX_ENEMIES)
        return;

    if (paint_enemy_slots_used[enemy_index])
    {
        actor_t *enemy = &actors[paint_enemy_ids[enemy_index]];
        deactivate_actor(enemy);
        paint_enemy_slots_used[enemy_index] = 0;
    }
}

// Place an enemy actor at a specific position
void place_enemy_actor(UBYTE enemy_index, UBYTE tilemap_x, UBYTE tilemap_y, UBYTE direction) BANKED
{
    if (enemy_index >= MAX_ENEMIES)
        return;

    // Set up the enemy actor
    actor_t *enemy = &actors[paint_enemy_ids[enemy_index]];
    enemy->pos.x = TO_FP(tilemap_x * 8);
    enemy->pos.y = TO_FP(tilemap_y * 8);
    
    // Check edit mode: script_memory[0] = 0 means play mode, 1 means edit mode
    UBYTE is_edit_mode = (script_memory[0] != 0);
    
    if (is_edit_mode) {
        // Edit mode: Position enemy actors but keep them deactivated (visible but inactive)
        // The actors will be visible for editing but won't move or collide
        deactivate_actor(enemy);

        // Set direction: direction=1 means left-facing, direction=0 means right-facing  
        UBYTE actor_dir = direction ? DIRECTION_LEFT : DIRECTION_RIGHT;
        actor_set_dir(enemy, actor_dir, 0); // 0 = not moving (edit mode)

        // Handle left-facing enemy positioning offset
        if (actor_dir == DIRECTION_LEFT) {
            enemy->pos.x = TO_FP(tilemap_x * 8 - 8);
        }
    } else {
        // Play mode: Activate enemy actors for gameplay
        activate_actor(enemy);

        // Set direction: direction=1 means left-facing, direction=0 means right-facing  
        UBYTE actor_dir = direction ? DIRECTION_LEFT : DIRECTION_RIGHT;
        actor_set_dir(enemy, actor_dir, 1); // 1 = moving (play mode)

        // Handle left-facing enemy positioning offset
        if (actor_dir == DIRECTION_LEFT) {
            enemy->pos.x = TO_FP(tilemap_x * 8 - 8);
        }
    }

    paint_enemy_slots_used[enemy_index] = 1;
}

// Restore all enemy actors from current level code data
void restore_enemy_actors_from_level_code(void) BANKED
{
    // First clear all existing enemy actors
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        clear_enemy_actor(i);
    }
    
    // Now place enemies based on current level code data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            // Enemy exists at this position
            UBYTE col = current_level_code.enemy_positions[i];
            UBYTE row = current_level_code.enemy_rows[i];
            
            // Convert to tilemap coordinates
            UBYTE tilemap_x = PLATFORM_X_MIN + col;  // Convert column to tilemap X
            UBYTE tilemap_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;  // Convert row to tilemap Y
            
            // Get direction for this enemy (bit i in enemy_directions)
            UBYTE direction = (current_level_code.enemy_directions & (1 << i)) ? 1 : 0;
            
            // Place the enemy actor
            place_enemy_actor(i, tilemap_x, tilemap_y, direction);
        }
    }
}
