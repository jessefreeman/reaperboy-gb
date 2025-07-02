#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"

// ============================================================================
// ENEMY ENCODING SYSTEM
// ============================================================================

// Position alphabet (41 symbols): '0' = no enemy, then 1-40 for positions
const char POS41[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%";

// Base-32 alphabet for masks (32 symbols): 0-31 values
const char BASE32[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

// Tile mappings for display
const UBYTE POS41_TILE_MAP[] = {
    48,                                     // '0' (tile 48)
    49, 50, 51, 52, 53, 54, 55, 56, 57,     // '1'-'9' (tiles 49-57)
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // 'A'-'J' (tiles 58-67)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // 'K'-'T' (tiles 68-77)
    78, 79, 80, 81, 82, 83,                 // 'U'-'Z' (tiles 78-83)
    84, 85, 86, 87, 88                      // Extended characters
};

const UBYTE BASE32_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // '0'-'9' (tiles 48-57)
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // 'A'-'J' (tiles 58-67)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // 'K'-'T' (tiles 68-77)
    78, 79                                  // 'U'-'V' (tiles 78-79)
};

// Maps enemy index to row (0-3)
const UBYTE ENEMY_ROW_MAP[MAX_ENEMIES] = {0, 1, 2, 3, 0, 1};

// Level code enemy system constants
#define LEVEL_CODE_MAX_ENEMIES 5

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Find index of character in POS41 alphabet (returns 0-40, or 255 if not found)
UBYTE find_pos41_index(char c) BANKED
{
    for (UBYTE i = 0; i < 41; i++)
    {
        if (POS41[i] == c)
            return i;
    }
    return 255; // Not found
}

// Find index of character in BASE32 alphabet (returns 0-31, or 255 if not found)
UBYTE find_base32_index(char c) BANKED
{
    for (UBYTE i = 0; i < 32; i++)
    {
        if (BASE32[i] == c)
            return i;
    }
    return 255; // Not found
}

// Get the row (0-3) for an enemy at a given position index
UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES)
        return 0;
    return ENEMY_ROW_MAP[enemy_index];
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

// Encode enemy position using POS41 system
char encode_enemy_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= LEVEL_CODE_MAX_ENEMIES || current_level_code.enemy_positions[enemy_index] == 255)
    {
        return POS41[0]; // '0' = no enemy
    }

    UBYTE col = current_level_code.enemy_positions[enemy_index];
    UBYTE row = get_enemy_row_from_position(enemy_index);

    // Calculate index: 1 + row*10 + anchor (where anchor = col/2)
    UBYTE anchor = col / 2;
    UBYTE idx = 1 + row * 10 + anchor;

    if (idx > 40)
        return POS41[0]; // Safety check

    return POS41[idx];
}

// Encode odd-column parity mask for character 22
char encode_odd_mask(void) BANKED
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

    return BASE32[odd_mask & 0x1F];
}

// Encode direction mask for character 23
char encode_direction_mask(void) BANKED
{
    UBYTE dir_mask = current_level_code.enemy_directions & 0x1F;
    return BASE32[dir_mask];
}

// ============================================================================
// LEVEL CODE INTERFACE FUNCTIONS
// ============================================================================

// These functions are called by code_level_core.c to generate the level code display

UBYTE encode_enemy_positions(void) BANKED
{
    // Character 17: First enemy position
    char c = encode_enemy_position(0);
    UBYTE idx = find_pos41_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_enemy_details_1(void) BANKED
{
    // Character 18: Second enemy position
    char c = encode_enemy_position(1);
    UBYTE idx = find_pos41_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_enemy_details_2(void) BANKED
{
    // Character 19: Third enemy position
    char c = encode_enemy_position(2);
    UBYTE idx = find_pos41_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_enemy_position_4(void) BANKED
{
    // Character 20: Fourth enemy position
    char c = encode_enemy_position(3);
    UBYTE idx = find_pos41_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_enemy_position_5(void) BANKED
{
    // Character 21: Fifth enemy position
    char c = encode_enemy_position(4);
    UBYTE idx = find_pos41_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_odd_mask_value(void) BANKED
{
    // Character 22: Odd column parity mask
    char c = encode_odd_mask();
    UBYTE idx = find_base32_index(c);
    return (idx == 255) ? 0 : idx;
}

UBYTE encode_enemy_directions(void) BANKED
{
    // Character 23: Direction mask
    char c = encode_direction_mask();
    UBYTE idx = find_base32_index(c);
    return (idx == 255) ? 0 : idx;
}

// Compatibility alias
UBYTE encode_enemy_bitmask(void) BANKED
{
    return encode_enemy_positions();
}

// ============================================================================
// ENEMY DECODING FUNCTIONS (for level code editing)
// ============================================================================

// Decode enemy position from POS41 character
void decode_enemy_position(UBYTE enemy_index, char pos_char, UBYTE odd_bit, UBYTE dir_bit) BANKED
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

    UBYTE idx = find_pos41_index(pos_char);

    if (idx == 255 || idx == 0)
    {
        // Invalid character or '0' = no enemy
        current_level_code.enemy_positions[enemy_index] = 255;
        clear_enemy_actor(enemy_index);
        return;
    }

    // Decode: idx = 1 + row*10 + anchor
    UBYTE v = idx - 1;     // 0-39
    UBYTE row = v / 10;    // 0-3
    UBYTE anchor = v % 10; // 0-9

    // Calculate actual column: anchor*2 + odd_bit
    UBYTE col = anchor * 2 + odd_bit;

    if (col > 19 || row > 3) // Safety check
    {
        current_level_code.enemy_positions[enemy_index] = 255;
        clear_enemy_actor(enemy_index);
        return;
    }

    // Store the position
    current_level_code.enemy_positions[enemy_index] = col;

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

// Decode full enemy data from 7-character level code section
void decode_enemy_data_from_level_code(const char *enemy_chars) BANKED
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

    // Decode masks
    UBYTE odd_mask = find_base32_index(enemy_chars[5]); // Character 22
    UBYTE dir_mask = find_base32_index(enemy_chars[6]); // Character 23

    if (odd_mask == 255)
        odd_mask = 0;
    if (dir_mask == 255)
        dir_mask = 0;

    // Clear enemy data
    current_level_code.enemy_directions = 0;
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }

    // Decode each enemy position (characters 17-21 = indices 0-4)
    for (UBYTE k = 0; k < 5; k++)
    {
        UBYTE odd_bit = (odd_mask >> k) & 1;
        UBYTE dir_bit = (dir_mask >> k) & 1;
        decode_enemy_position(k, enemy_chars[k], odd_bit, dir_bit);
    }

    // Clear and re-place enemy actors based on new positions
    // clear_all_enemy_actors();
    // for (UBYTE k = 0; k < 5; k++)
    // {
    //     if (current_level_code.enemy_positions[k] != 255)
    //     {
    //         UBYTE row = get_enemy_row_from_position(k);
    //         UBYTE col = current_level_code.enemy_positions[k];
    //         UBYTE tilemap_x = PLATFORM_X_MIN + col;
    //         UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
    //         UBYTE dir_bit = (current_level_code.enemy_directions >> k) & 1;
    //         place_enemy_actor(k, tilemap_x, actual_y, dir_bit);
    //     }
    // }
}

// ============================================================================
// LEVEL CODE EDITING SUPPORT
// ============================================================================

// Handle enemy data edit from level code (called when user edits character 17-23)
void handle_enemy_data_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    if (char_index < 17 || char_index > 23)
        return; // Not an enemy character

    // Build current enemy character array for decoding
    char enemy_chars[7];

    // Get current encoded values
    enemy_chars[0] = POS41[encode_enemy_positions()];   // Character 17
    enemy_chars[1] = POS41[encode_enemy_details_1()];   // Character 18
    enemy_chars[2] = POS41[encode_enemy_details_2()];   // Character 19
    enemy_chars[3] = POS41[encode_enemy_position_4()];  // Character 20
    enemy_chars[4] = POS41[encode_enemy_position_5()];  // Character 21
    enemy_chars[5] = BASE32[encode_odd_mask_value()];   // Character 22
    enemy_chars[6] = BASE32[encode_enemy_directions()]; // Character 23

    // Update the edited character
    UBYTE rel_index = char_index - 17; // Convert to 0-6 range

    if (rel_index < 5) // Position characters (17-21)
    {
        if (new_value <= 40) // Valid POS41 range
        {
            enemy_chars[rel_index] = POS41[new_value];
        }
    }
    else // Mask characters (22-23)
    {
        if (new_value <= 31) // Valid BASE32 range
        {
            enemy_chars[rel_index] = BASE32[new_value];
        }
    }

    // Decode and apply the updated enemy data
    decode_enemy_data_from_level_code(enemy_chars);
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

// Get display tile for POS41 character
UBYTE get_pos41_display_tile(char c) BANKED
{
    UBYTE idx = find_pos41_index(c);
    if (idx == 255 || idx > 40)
        return 48; // Default to '0' tile
    return POS41_TILE_MAP[idx];
}

// Get display tile for BASE32 character
UBYTE get_base32_display_tile(char c) BANKED
{
    UBYTE idx = find_base32_index(c);
    if (idx == 255 || idx > 31)
        return 48; // Default to '0' tile
    return BASE32_TILE_MAP[idx];
}

// Convert character to display value for level code editing
UBYTE enemy_char_to_value(UBYTE tile_id) BANKED
{
    // Check POS41 tiles first
    for (UBYTE i = 0; i < 41; i++)
    {
        if (POS41_TILE_MAP[i] == tile_id)
            return i;
    }

    // Check BASE32 tiles
    for (UBYTE i = 0; i < 32; i++)
    {
        if (BASE32_TILE_MAP[i] == tile_id)
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

// Clear all enemy actors from the screen
// void clear_all_enemy_actors(void) BANKED
// {
//     for (UBYTE i = 0; i < MAX_ENEMIES; i++)
//     {
//         clear_enemy_actor(i);
//     }
// }
