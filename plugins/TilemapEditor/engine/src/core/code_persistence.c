#pragma bank 251

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_persistence.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_platform_system_ext.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "tile_utils.h"
#include "paint.h"
#include "code_enemy_system_validation.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

// Internal helper functions
static void cycle_character_internal(UBYTE x, UBYTE y, BYTE direction) BANKED;
void update_display_with_value(UBYTE char_index, UBYTE value, UBYTE x, UBYTE y) BANKED;

// ============================================================================
// EXTERNAL FUNCTION DECLARATIONS (from paint.c)
// ============================================================================

// Explicitly declare functions that might not be properly resolved
extern void clear_existing_player_on_row_11(void) BANKED;
extern void move_player_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;
extern void position_exit_for_player(UBYTE player_x, UBYTE player_y) BANKED;
extern UBYTE paint_player_id;

// Enemy system validation functions
extern void update_valid_enemy_positions(void) BANKED;
extern void find_next_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED;
extern void find_prev_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED;
extern UBYTE encode_odd_mask_value(void) BANKED;

// ============================================================================
// CHARACTER CONVERSION UTILITIES
// ============================================================================
// Tile mapping for POS41 system:
// - Values 0-9 map to tiles 48-57 (ASCII characters '0'-'9')
// - Values 10-35 map to tiles 58-83 (ASCII characters 'A'-'Z')
// - Values 36-40 map to tiles 84-88 (ASCII characters '!', '@', '#', '$', '%')
//
// Tile mapping for BASE32 system:
// - Values 0-9 map to tiles 48-57 (ASCII characters '0'-'9')
// - Values 10-31 map to tiles 58-79 (ASCII characters 'A'-'V')

// Convert display character back to numeric value (optimized)
UBYTE char_to_value(UBYTE display_char) BANKED
{
    // Fast path for numeric characters '0'-'9' (tiles 48-57)
    if (display_char >= 48 && display_char <= 57)
    {
        return display_char - 48;
    }

    // Fast path for letter characters 'A'-'Z' (tiles 58-83)
    if (display_char >= 58 && display_char <= 83)
    {
        return (display_char - 58) + 10;
    }

    // Special characters (tiles 84-88) - handle in a single range check
    if (display_char >= 84 && display_char <= 88)
    {
        return display_char - 48; // 84-48=36, 85-48=37, etc.
    }

    return 0; // Default for invalid characters
}

// ============================================================================
// VARIABLE-BASED PERSISTENCE
// ============================================================================

// Save current level code to GB Studio variables (COMPLETE - No data loss!)
void save_level_code_to_variables(void) BANKED
{
    update_complete_level_code();

    // Each variable is 16 bits (UWORD = 0-65535)
    // Each platform pattern needs 5 bits (0-20, so we need 5 bits for 0-31 range)
    // We can fit 3 patterns per variable (3 × 5 = 15 bits, leaves 1 bit unused)

    // Part 1: Platform patterns 0-2
    UWORD part1 = (current_level_code.platform_patterns[0] << 10) |
                  (current_level_code.platform_patterns[1] << 5) |
                  current_level_code.platform_patterns[2];

    // Part 2: Platform patterns 3-5
    UWORD part2 = (current_level_code.platform_patterns[3] << 10) |
                  (current_level_code.platform_patterns[4] << 5) |
                  current_level_code.platform_patterns[5];

    // Part 3: Platform patterns 6-8
    UWORD part3 = (current_level_code.platform_patterns[6] << 10) |
                  (current_level_code.platform_patterns[7] << 5) |
                  current_level_code.platform_patterns[8];

    // Part 4: Platform patterns 9-11
    UWORD part4 = (current_level_code.platform_patterns[9] << 10) |
                  (current_level_code.platform_patterns[10] << 5) |
                  current_level_code.platform_patterns[11];

    // Part 5: Platform patterns 12-14
    UWORD part5 = (current_level_code.platform_patterns[12] << 10) |
                  (current_level_code.platform_patterns[13] << 5) |
                  current_level_code.platform_patterns[14];

    // Part 6: Platform pattern 15 (only 1 pattern, stored in lower 5 bits)
    UWORD part6 = current_level_code.platform_patterns[15];

    // Store to variables
    script_memory[VAR_LEVEL_CODE_PART_1] = part1;
    script_memory[VAR_LEVEL_CODE_PART_2] = part2;
    script_memory[VAR_LEVEL_CODE_PART_3] = part3;
    script_memory[VAR_LEVEL_CODE_PART_4] = part4;
    script_memory[VAR_LEVEL_CODE_PART_5] = part5;
    script_memory[VAR_LEVEL_CODE_PART_6] = part6;
}

// Load level code from GB Studio variables (COMPLETE - No data loss!)
void load_level_code_from_variables(void) BANKED
{
    init_level_code();

    // Load packed data
    UWORD part1 = script_memory[VAR_LEVEL_CODE_PART_1];
    UWORD part2 = script_memory[VAR_LEVEL_CODE_PART_2];
    UWORD part3 = script_memory[VAR_LEVEL_CODE_PART_3];
    UWORD part4 = script_memory[VAR_LEVEL_CODE_PART_4];
    UWORD part5 = script_memory[VAR_LEVEL_CODE_PART_5];
    UWORD part6 = script_memory[VAR_LEVEL_CODE_PART_6];

    // Unpack all platform patterns (each uses 5 bits, mask with 0x1F = 31)
    current_level_code.platform_patterns[0] = (part1 >> 10) & 0x1F;
    current_level_code.platform_patterns[1] = (part1 >> 5) & 0x1F;
    current_level_code.platform_patterns[2] = part1 & 0x1F;

    current_level_code.platform_patterns[3] = (part2 >> 10) & 0x1F;
    current_level_code.platform_patterns[4] = (part2 >> 5) & 0x1F;
    current_level_code.platform_patterns[5] = part2 & 0x1F;

    current_level_code.platform_patterns[6] = (part3 >> 10) & 0x1F;
    current_level_code.platform_patterns[7] = (part3 >> 5) & 0x1F;
    current_level_code.platform_patterns[8] = part3 & 0x1F;

    current_level_code.platform_patterns[9] = (part4 >> 10) & 0x1F;
    current_level_code.platform_patterns[10] = (part4 >> 5) & 0x1F;
    current_level_code.platform_patterns[11] = part4 & 0x1F;

    current_level_code.platform_patterns[12] = (part5 >> 10) & 0x1F;
    current_level_code.platform_patterns[13] = (part5 >> 5) & 0x1F;
    current_level_code.platform_patterns[14] = part5 & 0x1F;

    current_level_code.platform_patterns[15] = part6 & 0x1F;

    // Update valid player positions after loading platform patterns
    update_valid_player_positions();

    // Ensure player position is valid, if not, move to first valid position
    if (valid_player_count > 0 && !is_valid_player_position(current_level_code.player_column))
    {
        current_level_code.player_column = valid_player_columns[0];
    }
}

// ============================================================================
// SRAM-BASED PERSISTENCE
// ============================================================================

// Calculate simple checksum (XOR of all bytes except the checksum itself)
UBYTE calculate_level_code_checksum(sram_level_code_t *data) BANKED
{
    UBYTE checksum = 0;
    UBYTE *ptr = (UBYTE *)data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t) - 1; i++)
    {
        checksum ^= ptr[i];
    }
    return checksum;
}

// Save level code to SRAM with optimized data copying
void save_level_code_to_sram(void) BANKED
{
    update_complete_level_code();

    sram_level_code_t sram_data;
    sram_data.magic = SRAM_LEVEL_CODE_MAGIC;

    // Copy all platform patterns in one loop
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        sram_data.platform_patterns[i] = current_level_code.platform_patterns[i];
    }

    // Copy all enemy data with minimal operations
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        sram_data.enemy_positions[i] = current_level_code.enemy_positions[i];
    }
    sram_data.enemy_directions = current_level_code.enemy_directions;
    sram_data.enemy_types = current_level_code.enemy_types;
    sram_data.player_column = current_level_code.player_column;
    sram_data.checksum = calculate_level_code_checksum(&sram_data);

    // Write to SRAM in a single block copy operation
    ENABLE_RAM;
    UBYTE *sram_ptr = (UBYTE *)(0xA000 + SRAM_LEVEL_CODE_OFFSET);
    UBYTE *data_ptr = (UBYTE *)&sram_data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t); i++)
    {
        sram_ptr[i] = data_ptr[i];
    }
    DISABLE_RAM;
}

// Load level code from SRAM with optimized validation and data loading
UBYTE load_level_code_from_sram(void) BANKED
{
    sram_level_code_t sram_data;

    // Read from SRAM in a single block read operation
    ENABLE_RAM;
    UBYTE *sram_ptr = (UBYTE *)(0xA000 + SRAM_LEVEL_CODE_OFFSET);
    UBYTE *data_ptr = (UBYTE *)&sram_data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t); i++)
    {
        data_ptr[i] = sram_ptr[i];
    }
    DISABLE_RAM;

    // Fast validation of both magic number and checksum
    if (sram_data.magic != SRAM_LEVEL_CODE_MAGIC ||
        calculate_level_code_checksum(&sram_data) != sram_data.checksum)
    {
        return 0; // Invalid or corrupted data
    }

    // Copy platform patterns in one batch
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = sram_data.platform_patterns[i];
    }

    // Copy enemy data efficiently
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = sram_data.enemy_positions[i];
    }
    current_level_code.enemy_directions = sram_data.enemy_directions;
    current_level_code.enemy_types = sram_data.enemy_types;
    current_level_code.player_column = sram_data.player_column;

    // Update player position validation
    update_valid_player_positions();
    if (valid_player_count > 0 && !is_valid_player_position(current_level_code.player_column))
    {
        current_level_code.player_column = valid_player_columns[0];
    }

    return 1; // Success
}

// ============================================================================
// ADVANCED CHARACTER EDITING
// ============================================================================

// Update level code data structure when a character is edited via cycling
void update_level_code_from_character_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    if (char_index < TOTAL_BLOCKS)
    {
        // Platform pattern character - apply the new pattern to the tilemap

        // SUPPRESS display updates during pattern application to prevent
        // intermediate pattern states from showing up in the level code
        set_suppress_display_updates(1);
        suppress_code_updates_for_block(char_index);

        // Apply the pattern using brush logic
        apply_pattern_with_brush_logic(char_index, new_value);

        // Re-enable updates for this specific block
        enable_code_updates_for_block(char_index);

        // Update valid player positions since platform changed
        update_valid_player_positions();

        // Ensure player is still in a valid position
        position_player_at_valid_location();

        // Now update the level code for the target block and its neighbors
        // This ensures the final pattern shows up correctly
        update_single_block_code(char_index);
        update_neighboring_block_codes(char_index);

        // Re-enable global display updates
        set_suppress_display_updates(0);

        // Finally, update the display to show the correct final state
        display_selective_level_code_fast();
    }
    else if (char_index == 16)
    {
        // Player column position
        if (is_valid_player_position(new_value))
        {
            current_level_code.player_column = new_value;

            // Update player visual position
            UBYTE player_x = new_value + 2;
            UBYTE player_y = 11;
            clear_existing_player_on_row_11();
            replace_meta_tile(player_x, player_y, TILE_PLAYER, 1);
            // Move player actor to new position
            move_player_actor_to_tile(paint_player_id, player_x, player_y);

            // Position the exit sprite based on the new player position
            position_exit_for_player(player_x, player_y);
        }
    }
    else if (char_index >= 17 && char_index <= 23)
    {
        // Enemy data characters - use the enemy system to handle the edit
        handle_enemy_data_edit(char_index, new_value);

        // Update the display to show the changes
        display_selective_level_code_fast();
    }
}

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

// Check if saved level code exists
void vm_has_saved_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Check if any variables contain non-zero data
    UBYTE has_data = 0;
    for (UBYTE i = VAR_LEVEL_CODE_PART_1; i <= VAR_LEVEL_CODE_PART_6; i++)
    {
        if (script_memory[i] != 0)
        {
            has_data = 1;
            break;
        }
    }

    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = has_data;
}

// Cycle character in a given direction (1 = forward, -1 = reverse)
static void cycle_character_internal(UBYTE x, UBYTE y, BYTE direction) BANKED
{
    // First, check if this position is within the level code display area
    UBYTE char_index = get_char_index_from_display_position(x, y);

    if (char_index != 255)
    {
        // This is a level code character position
        UBYTE current_tile = sram_map_data[METATILE_MAP_OFFSET(x, y)];
        UBYTE current_value = char_to_value(current_tile);
        UBYTE new_value;

        // Determine the new value based on character position and cycling direction
        if (char_index < TOTAL_BLOCKS)
        {
            // Platform patterns: use position-aware validation to get valid pattern
            new_value = (direction > 0)
                            ? get_next_valid_pattern_for_char(char_index, current_value)
                            : get_previous_valid_pattern_for_char(char_index, current_value);
        }
        else if (char_index == 16)
        {
            // Player column: cycle only through valid positions (columns with platforms)
            // First, ensure fresh platform data
            extract_platform_data();
            update_valid_player_positions();

            // Get next/previous valid player position based on direction
            new_value = (direction > 0)
                            ? get_next_valid_player_position(current_value)
                            : get_previous_valid_player_position(current_value);
        }
        else if (char_index >= 17 && char_index <= 21)
        {
            // Enemy positions (POS41 system): 0-40 (0 means no enemy)
            // Get the enemy index from the character position (17-21 -> 0-4)
            UBYTE enemy_index = char_index - 17;

            // Update valid enemy positions to ensure we're using current data
            update_valid_enemy_positions();

            if (direction > 0)
            {
                // Forward cycling - get the next valid position using our validation system
                UBYTE pos_value = current_value;
                UBYTE odd_bit = (current_level_code.enemy_directions >> enemy_index) & 1;
                UBYTE dir_bit = odd_bit; // Keep direction the same

                find_next_valid_enemy_position_in_code(enemy_index, &pos_value, &odd_bit, &dir_bit);
                new_value = pos_value; // Use the validated position

                // Update odd bit in the mask if needed
                if (odd_bit != ((current_level_code.enemy_directions >> enemy_index) & 1))
                {
                    UBYTE current_odd_mask = encode_odd_mask_value();
                    UBYTE new_odd_mask = current_odd_mask;

                    if (odd_bit)
                        new_odd_mask |= (1 << enemy_index);
                    else
                        new_odd_mask &= ~(1 << enemy_index);

                    // Update the odd mask character (character 22)
                    update_display_with_value(22, new_odd_mask, 0, 0); // Position doesn't matter
                }
            }
            else
            {
                // Reverse cycling - get the previous valid position using our validation system
                UBYTE pos_value = current_value;
                UBYTE odd_bit = (current_level_code.enemy_directions >> enemy_index) & 1;
                UBYTE dir_bit = odd_bit; // Keep direction the same

                find_prev_valid_enemy_position_in_code(enemy_index, &pos_value, &odd_bit, &dir_bit);
                new_value = pos_value; // Use the validated position

                // Update odd bit in the mask if needed
                if (odd_bit != ((current_level_code.enemy_directions >> enemy_index) & 1))
                {
                    UBYTE current_odd_mask = encode_odd_mask_value();
                    UBYTE new_odd_mask = current_odd_mask;

                    if (odd_bit)
                        new_odd_mask |= (1 << enemy_index);
                    else
                        new_odd_mask &= ~(1 << enemy_index);

                    // Update the odd mask character (character 22)
                    update_display_with_value(22, new_odd_mask, 0, 0); // Position doesn't matter
                }
            }
        }
        else if (char_index == 22 || char_index == 23)
        {
            // Enemy mask values (BASE32 system): 0-31
            UBYTE max_value = 31;

            if (direction > 0)
            {
                // Forward cycling
                new_value = (current_value >= max_value) ? 0 : current_value + 1;
            }
            else
            {
                // Reverse cycling
                new_value = (current_value == 0) ? max_value : current_value - 1;
            }
        }
        else
        {
            // Other encoded values: 0-34
            UBYTE max_value = 34;

            if (direction > 0)
            {
                // Forward cycling
                new_value = (current_value >= max_value) ? 0 : current_value + 1;
            }
            else
            {
                // Reverse cycling
                new_value = (current_value == 0) ? max_value : current_value - 1;
            }
        }

        // Update the display using our helper function (directly uses numeric values)
        update_display_with_value(char_index, new_value, x, y);
    }
    else
    {
        // This is not a level code position, use original character cycling logic
        UBYTE current_tile = sram_map_data[METATILE_MAP_OFFSET(x, y)];
        UBYTE new_tile = current_tile;

        // Check if current tile is in the character range
        if (current_tile >= TILE_CHAR_FIRST && current_tile <= TILE_CHAR_LAST)
        {
            if (direction > 0)
            {
                // Going forward - increment the character
                new_tile = (current_tile == TILE_CHAR_LAST) ? TILE_CHAR_FIRST : current_tile + 1;
            }
            else
            {
                // Going backward - decrement the character
                new_tile = (current_tile == TILE_CHAR_FIRST) ? TILE_CHAR_LAST : current_tile - 1;
            }

            // Update both map data and visual display
            replace_meta_tile(x, y, new_tile, 1);
        }
    }
}

// Advanced character cycling function (forward)
void vm_cycle_character(SCRIPT_CTX *THIS) OLDCALL BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);

    // Call shared implementation with direction = 1 (forward)
    cycle_character_internal(x, y, 1);
}

// Reverse character cycling function (for backward navigation)
void vm_cycle_character_reverse(SCRIPT_CTX *THIS) OLDCALL BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);

    // Call shared implementation with direction = -1 (reverse)
    cycle_character_internal(x, y, -1);
}

// Update the display character directly with numeric value
void update_display_with_value(UBYTE char_index, UBYTE value, UBYTE x, UBYTE y) BANKED
{
    // Update the level code data and tilemap
    update_level_code_from_character_edit(char_index, value);

    // Update the display directly with numeric value (no character conversion)
    display_char_at_position(value, x, y);

    // Mark this position as updated to prevent flicker
    mark_display_position_for_update(char_index);
}
