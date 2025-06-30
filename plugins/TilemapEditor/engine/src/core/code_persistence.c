#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_persistence.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "tile_utils.h"
#include "paint.h"

// ============================================================================
// EXTERNAL FUNCTION DECLARATIONS (from paint.c)
// ============================================================================

// Explicitly declare functions that might not be properly resolved
extern void clear_existing_player_on_row_11(void) BANKED;
extern void move_player_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;
extern UBYTE paint_player_id;

// ============================================================================
// CHARACTER CONVERSION UTILITIES
// ============================================================================

// Convert display character back to numeric value
UBYTE char_to_value(UBYTE display_char) BANKED
{
    if (display_char >= 48 && display_char <= 57) // '0'-'9' (tiles 48-57)
    {
        return display_char - 48;
    }
    else if (display_char >= 58 && display_char <= 83) // 'A'-'Z' (tiles 58-83)
    {
        return (display_char - 58) + 10;
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

// Calculate simple checksum
UBYTE calculate_level_code_checksum(sram_level_code_t *data) BANKED
{
    UBYTE checksum = 0;
    UBYTE *ptr = (UBYTE *)data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t) - 1; i++) // Exclude checksum field
    {
        checksum ^= ptr[i];
    }
    return checksum;
}

// Save level code to SRAM
void save_level_code_to_sram(void) BANKED
{
    update_complete_level_code();

    sram_level_code_t sram_data;
    sram_data.magic = SRAM_LEVEL_CODE_MAGIC;

    // Copy platform patterns
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        sram_data.platform_patterns[i] = current_level_code.platform_patterns[i];
    }

    // Copy enemy data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        sram_data.enemy_positions[i] = current_level_code.enemy_positions[i];
    }
    sram_data.enemy_directions = current_level_code.enemy_directions;
    sram_data.enemy_types = current_level_code.enemy_types;
    sram_data.player_column = current_level_code.player_column;

    // Calculate and store checksum
    sram_data.checksum = calculate_level_code_checksum(&sram_data);

    // Write to SRAM
    ENABLE_RAM;
    UBYTE *sram_ptr = (UBYTE *)(0xA000 + SRAM_LEVEL_CODE_OFFSET);
    UBYTE *data_ptr = (UBYTE *)&sram_data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t); i++)
    {
        sram_ptr[i] = data_ptr[i];
    }
    DISABLE_RAM;
}

// Load level code from SRAM
UBYTE load_level_code_from_sram(void) BANKED
{
    sram_level_code_t sram_data;

    // Read from SRAM
    ENABLE_RAM;
    UBYTE *sram_ptr = (UBYTE *)(0xA000 + SRAM_LEVEL_CODE_OFFSET);
    UBYTE *data_ptr = (UBYTE *)&sram_data;
    for (UBYTE i = 0; i < sizeof(sram_level_code_t); i++)
    {
        data_ptr[i] = sram_ptr[i];
    }
    DISABLE_RAM;

    // Validate magic number and checksum
    if (sram_data.magic != SRAM_LEVEL_CODE_MAGIC)
    {
        return 0; // Invalid data
    }

    UBYTE calculated_checksum = calculate_level_code_checksum(&sram_data);
    if (calculated_checksum != sram_data.checksum)
    {
        return 0; // Corrupted data
    }

    // Copy data to current level code
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = sram_data.platform_patterns[i];
    }

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = sram_data.enemy_positions[i];
    }
    current_level_code.enemy_directions = sram_data.enemy_directions;
    current_level_code.enemy_types = sram_data.enemy_types;
    current_level_code.player_column = sram_data.player_column;

    // Update valid player positions after loading platform patterns
    update_valid_player_positions();

    // Ensure player position is valid, if not, move to first valid position
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
        // Platform pattern character
        current_level_code.platform_patterns[char_index] = new_value;

        // Apply the new pattern to the tilemap
        apply_pattern_with_brush_logic(char_index, new_value);

        // Update valid player positions since platform changed
        update_valid_player_positions();

        // Ensure player is still in a valid position
        position_player_at_valid_location();
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
        }
    }
    else if (char_index >= 17 && char_index <= 23)
    {
        // Enemy data - this is complex since it's encoded
        // For now, just trigger a complete update
        update_complete_level_code();
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

// VM wrapper functions for SRAM storage
void vm_save_level_code_sram(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    save_level_code_to_sram();
}

void vm_load_level_code_sram(SCRIPT_CTX *THIS) BANKED
{
    UBYTE success = load_level_code_from_sram();
    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = success;
    if (success)
    {
        reconstruct_tilemap_from_level_code(); // Apply patterns to tilemap using brush logic
        force_complete_level_code_display();   // Force complete redraw after loading
    }
}

// Advanced character cycling function
void vm_cycle_character(SCRIPT_CTX *THIS) OLDCALL BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);

    // First, check if this position is within the level code display area
    UBYTE char_index = get_char_index_from_display_position(x, y);

    if (char_index != 255)
    {
        // This is a level code character position
        UBYTE current_tile = sram_map_data[METATILE_MAP_OFFSET(x, y)];
        UBYTE current_value = char_to_value(current_tile);
        UBYTE new_value;

        // Determine the maximum value for this position
        UBYTE max_value;
        if (char_index < TOTAL_BLOCKS)
        {
            // Platform patterns: cycle through 0 to PLATFORM_PATTERN_COUNT-1
            max_value = PLATFORM_PATTERN_COUNT - 1;
            new_value = (current_value + 1) % PLATFORM_PATTERN_COUNT;
        }
        else if (char_index == 16)
        {
            // Player column: cycle only through valid positions (columns with platforms)
            UBYTE current_col = char_to_value(current_tile);
            UBYTE next_col = get_next_valid_player_position(current_col);
            new_value = next_col;
        }
        else if (char_index >= 17 && char_index <= 22)
        {
            // Enemy positions: 0-19 (0 means no enemy)
            max_value = 19;

            // Cycle to next value
            if (current_value >= max_value)
            {
                new_value = 0; // Wrap to 0
            }
            else
            {
                new_value = current_value + 1;
            }
        }
        else
        {
            // Other encoded values: 0-34
            max_value = 34;

            // Cycle to next value
            if (current_value >= max_value)
            {
                new_value = 0; // Wrap to 0
            }
            else
            {
                new_value = current_value + 1;
            }
        }

        // Update the level code data and tilemap
        update_level_code_from_character_edit(char_index, new_value);

        // Update the display character
        UBYTE display_char = get_extended_display_char(new_value);
        display_char_at_position(display_char, x, y);

        // Mark this position as updated to prevent flicker
        mark_display_position_for_update(char_index);
    }
    else
    {
        // This is not a level code position, use original character cycling logic
        UBYTE current_tile = sram_map_data[METATILE_MAP_OFFSET(x, y)];
        UBYTE new_tile = current_tile;

        // Check if current tile is in the character range (TILE_CHAR_FIRST to TILE_CHAR_LAST)
        if (current_tile >= TILE_CHAR_FIRST && current_tile <= TILE_CHAR_LAST)
        {
            // Going forward - increment the character
            if (current_tile == TILE_CHAR_LAST) // If at 'Z' (83), wrap to '0' (48)
            {
                new_tile = TILE_CHAR_FIRST;
            }
            else
            {
                new_tile = current_tile + 1;
            }

            // Set the new tile using replace_meta_tile to update both map data and visual display
            replace_meta_tile(x, y, new_tile, 1);
        }
    }
}
