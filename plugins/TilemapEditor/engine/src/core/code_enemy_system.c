#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"

// ============================================================================
// ENEMY NUMERIC SYSTEM
// ============================================================================

// Level code enemy system constants
#define LEVEL_CODE_MAX_ENEMIES 5

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
    // Clear all enemy data
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255; // 255 = no enemy
        current_level_code.enemy_rows[i] = 255;      // 255 = no enemy
    }

    UBYTE enemy_count = 0;

    // Scan the 4 enemy rows
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

        // Scan all 20 columns
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE tilemap_x = PLATFORM_X_MIN + col;
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(tilemap_x, actual_y)];
            UBYTE tile_type = get_tile_type(tile);

            if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < LEVEL_CODE_MAX_ENEMIES)
            {
                current_level_code.enemy_positions[enemy_count] = col;
                current_level_code.enemy_rows[enemy_count] = row; // Store the row too!

                // Set direction bit (left = 1, right = 0)
                if (tile_type == BRUSH_TILE_ENEMY_L)
                {
                    current_level_code.enemy_directions |= (1 << enemy_count);
                }

                enemy_count++;
            }
        }
    }
}

// ============================================================================
// ENEMY ENCODING FUNCTIONS
// ============================================================================

// Encode enemy position using numeric POS41 system
UBYTE encode_enemy_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES || current_level_code.enemy_positions[enemy_index] == 255)
    {
        return 0; // 0 = no enemy
    }

    UBYTE col = current_level_code.enemy_positions[enemy_index];
    UBYTE row = get_enemy_row_from_position(enemy_index);

    // Calculate index: 1 + row*10 + anchor (where anchor = col/2)
    UBYTE anchor = col / 2;
    UBYTE idx = 1 + row * 10 + anchor;

    if (idx > 40)
        return 0; // Safety check

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
void decode_enemy_position(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit, UBYTE dir_bit) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES)
        return;

    // Clear previous enemy placement if it exists
    if (current_level_code.enemy_positions[enemy_index] != 255)
    {
        UBYTE old_row = get_enemy_row_from_position(enemy_index);
        UBYTE old_col = current_level_code.enemy_positions[enemy_index];
        UBYTE old_tilemap_x = PLATFORM_X_MIN + old_col;
        UBYTE old_actual_y = PLATFORM_Y_MIN + old_row * SEGMENT_HEIGHT;
        UBYTE old_tile = sram_map_data[METATILE_MAP_OFFSET(old_tilemap_x, old_actual_y)];
        UBYTE old_tile_type = get_tile_type(old_tile);

        if (old_tile_type == BRUSH_TILE_ENEMY_L || old_tile_type == BRUSH_TILE_ENEMY_R)
        {
            replace_meta_tile(old_tilemap_x, old_actual_y, 0, 1);
        }
    }

    if (pos_value == 0)
    {
        // 0 = no enemy
        current_level_code.enemy_positions[enemy_index] = 255;
        clear_enemy_actor(enemy_index);
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
        clear_enemy_actor(enemy_index);
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

    // Place the enemy on the tilemap
    UBYTE tilemap_x = PLATFORM_X_MIN + col;
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
    UBYTE enemy_tile = dir_bit ? TILE_LEFT_ENEMY : TILE_RIGHT_ENEMY;

    replace_meta_tile(tilemap_x, actual_y, enemy_tile, 1);

    // Update the enemy actor position
    place_enemy_actor(enemy_index, tilemap_x, actual_y, dir_bit);
}

// Decode full enemy data from numeric values array
void decode_enemy_data_from_values(const UBYTE *enemy_values) BANKED
{
    // Clear existing enemies from tilemap
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE tilemap_x = PLATFORM_X_MIN + col;
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(tilemap_x, actual_y)];
            UBYTE tile_type = get_tile_type(tile);

            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                replace_meta_tile(tilemap_x, actual_y, 0, 1);
            }
        }
    }

    // Get masks directly (no character conversion needed)
    UBYTE odd_mask = enemy_values[5] & 0x1F; // Character 22
    UBYTE dir_mask = enemy_values[6] & 0x1F; // Character 23

    // Clear enemy data
    current_level_code.enemy_directions = 0;
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
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
        // Allow values beyond 40 for debugging
        if (new_value < 41) // Valid POS41 range
        {
            enemy_values[rel_index] = new_value;
        }
        else
        {
            // Debug: For values beyond the POS41 range, use the maximum value
            enemy_values[rel_index] = 40;
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
    activate_actor(enemy);

    // Set direction: direction=1 means left-facing, direction=0 means right-facing
    UBYTE actor_dir = direction ? DIRECTION_LEFT : DIRECTION_RIGHT;
    actor_set_dir(enemy, actor_dir, 1); // 1 = moving/active

    paint_enemy_slots_used[enemy_index] = 1;
}
