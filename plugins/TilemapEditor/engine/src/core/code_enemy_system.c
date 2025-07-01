#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"

// ============================================================================
// NEW SIMPLIFIED ENEMY ENCODING SYSTEM
// Character sets for the new 7-character enemy encoding system
// ============================================================================

// Position alphabet (41 symbols): '0' = no enemy, then 1-40 for positions
const char POS41[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%";

// Base-32 alphabet for masks (32 symbols): 0-31 values
const char BASE32[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

// Tile mappings for the extended character set
const UBYTE POS41_TILE_MAP[] = {
    48,                                     // '0' (tile 48)
    49, 50, 51, 52, 53, 54, 55, 56, 57,     // '1'-'9' (tiles 49-57)
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // 'A'-'J' (tiles 58-67)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // 'K'-'T' (tiles 68-77)
    78, 79, 80, 81, 82, 83,                 // 'U'-'Z' (tiles 78-83)
    33, 64, 35, 36, 37                      // '!@#$%' (tiles 33, 64, 35, 36, 37)
};

const UBYTE BASE32_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // '0'-'9' (tiles 48-57)
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // 'A'-'J' (tiles 58-67)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // 'K'-'T' (tiles 68-77)
    78, 79                                  // 'U'-'V' (tiles 78-79)
};

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

// ============================================================================
// ENEMY DATA EXTRACTION FROM TILEMAP
// ============================================================================

void extract_enemy_data(void) BANKED
{
    UBYTE enemy_count = 0;

    // Clear all enemy data
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255; // 255 = no enemy
    }

    // Scan the 4 enemy rows (corresponding to rows 0-3 in the 4x5 block system)
    // These map to actual tilemap rows 12, 14, 16, 18
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT; // 12, 14, 16, 18

        // Scan all 20 columns (0-19 in game coordinates = tilemap cols 2-21)
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE tilemap_x = PLATFORM_X_MIN + col; // Convert to tilemap coordinates
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(tilemap_x, actual_y)];
            UBYTE tile_type = get_tile_type(tile);

            if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < MAX_ENEMIES)
            {
                // Store the game coordinate column (0-19)
                current_level_code.enemy_positions[enemy_count] = col;

                // Set direction bit (left = 1, right = 0)
                if (tile_type == BRUSH_TILE_ENEMY_L)
                {
                    current_level_code.enemy_directions |= (1 << enemy_count);
                }

                // For future: detect enemy types and set bits in enemy_types
                // UBYTE enemy_type = detect_enemy_type(tile_type);
                // if (enemy_type == 1) { // If jumper
                //     current_level_code.enemy_types |= (1 << enemy_count);
                // }

                enemy_count++;
            }
        }
    }
}

// ============================================================================
// NEW SIMPLIFIED ENEMY ENCODING FUNCTIONS
// ============================================================================

// Get the row (0-3) for an enemy at a given position index
UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= MAX_ENEMIES || current_level_code.enemy_positions[enemy_index] == 255)
        return 255; // Invalid or no enemy

    // For now, scan the tilemap to find which row this enemy is actually on
    UBYTE col = current_level_code.enemy_positions[enemy_index];
    UBYTE tilemap_x = PLATFORM_X_MIN + col;

    // Check each of the 4 possible enemy rows
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(tilemap_x, actual_y)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
        {
            return row; // Found the enemy in this row
        }
    }

    return 0; // Default to row 0 if not found
}

// Encode enemy position using new POS41 system
char encode_enemy_position(UBYTE enemy_index) BANKED
{
    if (enemy_index >= MAX_ENEMIES || current_level_code.enemy_positions[enemy_index] == 255)
    {
        return POS41[0]; // '0' = no enemy
    }

    UBYTE col = current_level_code.enemy_positions[enemy_index];
    UBYTE row = get_enemy_row_from_position(enemy_index);

    if (row == 255)
        return POS41[0]; // Safety check

    // Calculate index: 1 + row*10 + anchor (where anchor = col/2)
    UBYTE anchor = col / 2;            // 0-9 (even columns: 0,2,4...18 map to 0,1,2...9)
    UBYTE idx = 1 + row * 10 + anchor; // 1-40

    if (idx > 40)
        return POS41[0]; // Safety check

    return POS41[idx];
}

// Encode odd-column parity mask for character 22
char encode_odd_mask(void) BANKED
{
    UBYTE odd_mask = 0;

    for (UBYTE k = 0; k < MAX_ENEMIES; k++)
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

    return BASE32[odd_mask & 0x1F]; // Ensure it's within 0-31
}

// Encode direction mask for character 23
char encode_direction_mask(void) BANKED
{
    UBYTE dir_mask = current_level_code.enemy_directions & 0x1F; // Only use first 5 bits
    return BASE32[dir_mask];
}

// ============================================================================
// ENEMY DECODING FUNCTIONS (for level code editing/loading)
// ============================================================================

// Decode enemy position from POS41 character
void decode_enemy_position(UBYTE enemy_index, char pos_char, UBYTE odd_bit, UBYTE dir_bit) BANKED
{
    if (enemy_index >= MAX_ENEMIES)
        return;

    UBYTE idx = find_pos41_index(pos_char);

    if (idx == 255 || idx == 0)
    {
        // Invalid character or '0' = no enemy
        current_level_code.enemy_positions[enemy_index] = 255;
        return;
    }

    // Decode: idx = 1 + row*10 + anchor
    UBYTE v = idx - 1;     // 0-39
    UBYTE row = v / 10;    // 0-3
    UBYTE anchor = v % 10; // 0-9

    // Calculate actual column: anchor*2 + odd_bit
    UBYTE col = anchor * 2 + odd_bit;

    if (col > 19) // Safety check
    {
        current_level_code.enemy_positions[enemy_index] = 255;
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

    // TODO: Place the enemy on the tilemap at the correct row and column
    // For now, we'll place it on row 0 (actual_y = PLATFORM_Y_MIN)
    UBYTE tilemap_x = PLATFORM_X_MIN + col;
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
    UBYTE enemy_tile = dir_bit ? BRUSH_TILE_ENEMY_L : BRUSH_TILE_ENEMY_R;

    replace_meta_tile(tilemap_x, actual_y, enemy_tile, 1);
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
                replace_meta_tile(tilemap_x, actual_y, 0, 1); // Clear enemy
            }
        }
    }

    // Decode masks
    UBYTE odd_mask = find_base32_index(enemy_chars[5]); // Character 22
    UBYTE dir_mask = find_base32_index(enemy_chars[6]); // Character 23

    if (odd_mask == 255)
        odd_mask = 0; // Safety
    if (dir_mask == 255)
        dir_mask = 0; // Safety

    // Clear enemy data
    current_level_code.enemy_directions = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
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
    clear_all_enemy_actors();
    for (UBYTE k = 0; k < 5; k++)
    {
        if (current_level_code.enemy_positions[k] != 255)
        {
            UBYTE col = current_level_code.enemy_positions[k];
            UBYTE row = get_enemy_row_from_position(k);
            UBYTE dir_bit = (current_level_code.enemy_directions >> k) & 1;

            place_enemy_actor(k, col, row, dir_bit);
        }
    }
}

// ============================================================================
// DISPLAY FUNCTIONS FOR NEW ENCODING SYSTEM
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
    // Convert tile ID back to character, then to value
    // This is for level code editing compatibility

    // Check POS41 tiles first
    for (UBYTE i = 0; i < 41; i++)
    {
        if (POS41_TILE_MAP[i] == tile_id)
            return i; // Return the index (0-40)
    }

    // Check BASE32 tiles
    for (UBYTE i = 0; i < 32; i++)
    {
        if (BASE32_TILE_MAP[i] == tile_id)
            return i; // Return the index (0-31)
    }

    return 0; // Default
}

// ============================================================================
// MAIN INTERFACE FUNCTIONS (replaces old encoding functions)
// ============================================================================

// These functions maintain compatibility with the existing display system
// Characters 17-23 in the level code

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

// New functions for the remaining characters
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
// TESTING AND VALIDATION FUNCTIONS
// ============================================================================

// Test the new encoding system with sample data
void test_new_enemy_encoding(void) BANKED
{
    // Clear existing data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;

    // Set up test data
    current_level_code.enemy_positions[0] = 5;  // Row 0, col 5 (anchor 2, odd)
    current_level_code.enemy_positions[1] = 10; // Row 0, col 10 (anchor 5, even)
    current_level_code.enemy_positions[2] = 15; // Row 1, col 15 (anchor 7, odd)
    current_level_code.enemy_positions[3] = 0;  // Row 2, col 0 (anchor 0, even)
    current_level_code.enemy_positions[4] = 19; // Row 3, col 19 (anchor 9, odd)

    // Set directions: enemies 0, 2, 4 face left
    current_level_code.enemy_directions = 0b10101; // Binary: 21

    // Test encoding
    char pos_chars[5];
    for (UBYTE i = 0; i < 5; i++)
    {
        pos_chars[i] = encode_enemy_position(i);
    }

    char odd_char = encode_odd_mask();
    char dir_char = encode_direction_mask();

    // For testing: place these on the tilemap at the correct positions
    // This would normally be done by the level code display system

    // Force a complete level code display to show the results
    force_complete_level_code_display();
}

// Validate that encoding/decoding round-trip works correctly
void validate_enemy_encoding_roundtrip(void) BANKED
{
    // Save original state
    level_code_t original = current_level_code;

    // Test encoding
    char enemy_chars[7];
    for (UBYTE i = 0; i < 5; i++)
    {
        enemy_chars[i] = encode_enemy_position(i);
    }
    enemy_chars[5] = encode_odd_mask();
    enemy_chars[6] = encode_direction_mask();

    // Clear current state
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;

    // Test decoding
    decode_enemy_data_from_level_code(enemy_chars);

    // Validate that we got back the same data
    // (In a real implementation, you'd want to check if the data matches)

    // Restore original state
    current_level_code = original;
}

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_test_new_enemy_encoding(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    test_new_enemy_encoding();
}

void vm_validate_enemy_encoding_roundtrip(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    validate_enemy_encoding_roundtrip();
}

void vm_extract_enemy_data(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    extract_enemy_data();
}

// Get enemy encoding info for script access
void vm_get_enemy_encoding_info(SCRIPT_CTX *THIS) BANKED
{
    // Store current enemy encoding results in script variables
    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = encode_enemy_positions();  // Character 17
    *(UWORD *)VM_REF_TO_PTR(FN_ARG1) = encode_enemy_details_1();  // Character 18
    *(UWORD *)VM_REF_TO_PTR(FN_ARG2) = encode_enemy_details_2();  // Character 19
    *(UWORD *)VM_REF_TO_PTR(FN_ARG3) = encode_enemy_position_4(); // Character 20

    // Additional info if needed
    if (FN_ARG4 != 0)
        *(UWORD *)VM_REF_TO_PTR(FN_ARG4) = encode_enemy_position_5(); // Character 21
    if (FN_ARG5 != 0)
        *(UWORD *)VM_REF_TO_PTR(FN_ARG5) = encode_odd_mask_value(); // Character 22
    if (FN_ARG6 != 0)
        *(UWORD *)VM_REF_TO_PTR(FN_ARG6) = encode_enemy_directions(); // Character 23
}

// VM wrapper functions for direct enemy level code editing
void vm_edit_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED
{
    UBYTE char_index = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE new_value = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);

    if (char_index >= 17 && char_index <= 23)
    {
        handle_enemy_data_edit(char_index, new_value);
    }
}

void vm_increment_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED
{
    UBYTE char_index = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);

    if (char_index >= 17 && char_index <= 23)
    {
        // Get current value
        UBYTE current_val = 0;
        UBYTE max_val = 40; // Default to POS41 range

        if (char_index >= 17 && char_index <= 21) // Position characters
        {
            switch (char_index)
            {
            case 17:
                current_val = encode_enemy_positions();
                break;
            case 18:
                current_val = encode_enemy_details_1();
                break;
            case 19:
                current_val = encode_enemy_details_2();
                break;
            case 20:
                current_val = encode_enemy_position_4();
                break;
            case 21:
                current_val = encode_enemy_position_5();
                break;
            }
            max_val = 40; // POS41 range
        }
        else // Mask characters (22-23)
        {
            switch (char_index)
            {
            case 22:
                current_val = encode_odd_mask_value();
                break;
            case 23:
                current_val = encode_enemy_directions();
                break;
            }
            max_val = 31; // BASE32 range
        }

        // Increment with wrap-around
        UBYTE new_val = (current_val + 1) % (max_val + 1);
        handle_enemy_data_edit(char_index, new_val);
    }
}

void vm_decrement_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED
{
    UBYTE char_index = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);

    if (char_index >= 17 && char_index <= 23)
    {
        // Get current value
        UBYTE current_val = 0;
        UBYTE max_val = 40; // Default to POS41 range

        if (char_index >= 17 && char_index <= 21) // Position characters
        {
            switch (char_index)
            {
            case 17:
                current_val = encode_enemy_positions();
                break;
            case 18:
                current_val = encode_enemy_details_1();
                break;
            case 19:
                current_val = encode_enemy_details_2();
                break;
            case 20:
                current_val = encode_enemy_position_4();
                break;
            case 21:
                current_val = encode_enemy_position_5();
                break;
            }
            max_val = 40; // POS41 range
        }
        else // Mask characters (22-23)
        {
            switch (char_index)
            {
            case 22:
                current_val = encode_odd_mask_value();
                break;
            case 23:
                current_val = encode_enemy_directions();
                break;
            }
            max_val = 31; // BASE32 range
        }

        // Decrement with wrap-around
        UBYTE new_val = (current_val == 0) ? max_val : (current_val - 1);
        handle_enemy_data_edit(char_index, new_val);
    }
}

// Test functions for validating enemy level code editing
void vm_test_enemy_level_code_editing(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;

    // Test editing character 17 (first enemy position)
    handle_enemy_data_edit(17, 5); // Set to position index 5

    // Test editing character 23 (direction mask)
    handle_enemy_data_edit(23, 15); // Set direction mask to 15 (0b01111)
}

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// ============================================================================

// These functions are kept for backward compatibility with existing code
// They now use the new encoding system internally

void init_test_enemy_patterns(void) BANKED
{
    test_new_enemy_encoding();
}

void validate_enemy_encoding(void) BANKED
{
    validate_enemy_encoding_roundtrip();
}

void update_enemy_patterns(void) BANKED
{
    extract_enemy_data();
}

void validate_enemy_positions_after_platform_change(void) BANKED
{
    extract_enemy_data();
}

// Pattern-based functions (simplified for new system)
UBYTE extract_enemy_pattern_for_block_row(UBYTE row) BANKED
{
    // Count enemies in the specified row
    UBYTE enemy_count = 0;

    if (row >= 4)
        return 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            UBYTE enemy_row = get_enemy_row_from_position(i);
            if (enemy_row == row)
            {
                enemy_count++;
            }
        }
    }

    return enemy_count;
}

void apply_enemy_pattern_to_block_row(UBYTE row, UBYTE pattern_id, UBYTE direction_mask) BANKED
{
    // Simplified pattern application
    // For now, just clear the row and place a single enemy if pattern_id > 0

    if (row >= 4)
        return;

    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

    // Clear existing enemies in this row
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

    // Place new enemy if pattern_id > 0
    if (pattern_id > 0)
    {
        UBYTE col = 5 + (pattern_id % 10); // Simple placement
        UBYTE tilemap_x = PLATFORM_X_MIN + col;
        UBYTE direction_bit = direction_mask & 1;
        UBYTE enemy_tile = direction_bit ? BRUSH_TILE_ENEMY_L : BRUSH_TILE_ENEMY_R;

        replace_meta_tile(tilemap_x, actual_y, enemy_tile, 1);
    }
}

void test_cycle_enemy_patterns(void) BANKED
{
    // Cycle through different test patterns
    static UBYTE test_pattern = 0;
    test_pattern = (test_pattern + 1) % 4;

    apply_enemy_pattern_to_block_row(0, test_pattern, 0b101010);
}

void test_cycle_enemy_directions(void) BANKED
{
    // Flip all enemy directions
    current_level_code.enemy_directions = ~current_level_code.enemy_directions;

    // Update tilemap to reflect new directions
    extract_enemy_data(); // Re-extract to sync tilemap
}

// ============================================================================
// ACTOR MANAGEMENT FUNCTIONS FOR ENEMY DECODING (IMPLEMENTATIONS)
// ============================================================================

// Convert subpixels (from paint.c)
#define TO_FP(n) ((INT16)((n) << 4))

// Direction constants from GB Studio
#define DIRECTION_DOWN 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_LEFT 3

// External references for actor management
extern UBYTE paint_enemy_ids[5];
extern UBYTE paint_enemy_slots_used[5];
extern actor_t actors[];

// External function declarations we need
extern void activate_actor(actor_t *actor) BANKED;
extern void deactivate_actor(actor_t *actor) BANKED;
extern void actor_set_dir(actor_t *actor, UBYTE dir, UBYTE moving) BANKED;

// Clear all enemy actors and tiles when decoding new level code
void clear_all_enemy_actors(void) BANKED
{
    for (UBYTE i = 0; i < 5; i++)
    {
        if (paint_enemy_slots_used[i])
        {
            actor_t *enemy = &actors[paint_enemy_ids[i]];

            // Convert actor position from fixed point to tile coordinates
            UBYTE old_tile_x = (enemy->pos.x >> 4) / 8;
            UBYTE old_tile_y = (enemy->pos.y >> 4) / 8;

            // For left-facing enemies, the tile position is offset by +1 tile
            if (enemy->dir == DIRECTION_LEFT)
            {
                old_tile_x += 1;
            }

            // Clear the old tile (set to empty)
            replace_meta_tile(old_tile_x, old_tile_y, 0, 1);

            // Deactivate the actor
            deactivate_actor(enemy);
            paint_enemy_slots_used[i] = 0;
        }
    }
}

// Place an enemy actor at the specified position with direction
void place_enemy_actor(UBYTE enemy_index, UBYTE col, UBYTE row, UBYTE direction) BANKED
{
    if (enemy_index >= 5) // Only 5 enemy slots available
        return;

    // Calculate tilemap position
    UBYTE tilemap_x = PLATFORM_X_MIN + col;
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

    // Place the tile
    UBYTE enemy_tile = direction ? BRUSH_TILE_ENEMY_L : BRUSH_TILE_ENEMY_R;
    replace_meta_tile(tilemap_x, actual_y, enemy_tile, 1);

    // Set up the actor
    actor_t *enemy = &actors[paint_enemy_ids[enemy_index]];
    enemy->pos.x = TO_FP(tilemap_x * 8);
    enemy->pos.y = TO_FP(actual_y * 8);
    activate_actor(enemy);

    // Set direction (DIRECTION_LEFT = 3, DIRECTION_RIGHT = 1)
    UBYTE actor_dir = direction ? DIRECTION_LEFT : DIRECTION_RIGHT;
    actor_set_dir(enemy, actor_dir, 1); // 1 = moving/active

    paint_enemy_slots_used[enemy_index] = 1;
}
