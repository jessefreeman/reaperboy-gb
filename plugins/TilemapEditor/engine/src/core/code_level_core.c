#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "tile_utils.h"

// External data declarations for cross-bank access
extern const UBYTE PATTERN_TILE_MAP[];
extern const UBYTE EXTENDED_PATTERN_TILE_MAP[];

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================

// Main level code data structure instance
level_code_t current_level_code;

// ============================================================================
// SELECTIVE UPDATE SYSTEM - Prevents flicker by only updating changed chars
// ============================================================================

level_code_t previous_level_code;
UBYTE level_code_initialized = 0;

// Cache for encoded values to avoid recalculation
UBYTE previous_encoded_enemy_data[4];
UBYTE current_encoded_enemy_data[4];

// Bitmask to track which display positions need updating (24 bits max)
UWORD display_update_mask_low = 0;  // Bits 0-15
UBYTE display_update_mask_high = 0; // Bits 16-23

// Mark a display position for update
void mark_display_position_for_update(UBYTE position) BANKED
{
    if (position < 16)
    {
        display_update_mask_low |= (1 << position);
    }
    else if (position < 24)
    {
        display_update_mask_high |= (1 << (position - 16));
    }
}

// Check if a display position needs updating
UBYTE display_position_needs_update(UBYTE position) BANKED
{
    if (position < 16)
    {
        return (display_update_mask_low & (1 << position)) != 0;
    }
    else if (position < 24)
    {
        return (display_update_mask_high & (1 << (position - 16))) != 0;
    }
    return 0;
}

// Clear all update flags
void clear_display_update_flags(void) BANKED
{
    display_update_mask_low = 0;
    display_update_mask_high = 0;
}

// Calculate display position for a given character index
void get_display_position(UBYTE char_index, UBYTE *x, UBYTE *y) BANKED
{
    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;

    // Layout: 3 blocks of 4 characters per row, with spaces between blocks
    // Row 0: 0000 0000 0000 (characters 0-11)
    // Row 1: 0000 0000 0000 (characters 12-23)
    // Display format: "0000 0000 0000" per row

    UBYTE row = char_index / 12; // 12 characters per row
    UBYTE col = char_index % 12; // Position within row

    // Calculate column position with spaces between blocks
    UBYTE block = col / 4;        // Which block (0, 1, or 2)
    UBYTE pos_in_block = col % 4; // Position within block (0-3)

    display_x += block * 5 + pos_in_block; // 5 = 4 chars + 1 space
    display_y += row;

    *x = display_x;
    *y = display_y;
}

// Compare current level code with previous and mark changes
void detect_level_code_changes(void) BANKED
{
    if (!level_code_initialized)
    {
        // First time - mark everything for update
        for (UBYTE i = 0; i < LEVEL_CODE_CHARS_TOTAL; i++)
        {
            mark_display_position_for_update(i);
        }

        // Cache initial encoded enemy values
        current_encoded_enemy_data[0] = encode_enemy_positions();
        current_encoded_enemy_data[1] = encode_enemy_details_1();
        current_encoded_enemy_data[2] = encode_enemy_details_2();
        current_encoded_enemy_data[3] = current_level_code.player_column;

        // Copy to previous cache
        for (UBYTE i = 0; i < 4; i++)
        {
            previous_encoded_enemy_data[i] = current_encoded_enemy_data[i];
        }

        level_code_initialized = 1;
    }
    else
    {
        // Check platform patterns (positions 0-15)
        for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
        {
            if (current_level_code.platform_patterns[i] != previous_level_code.platform_patterns[i])
            {
                mark_display_position_for_update(i);
            }
        }

        // Update current encoded enemy data
        current_encoded_enemy_data[0] = encode_enemy_positions();
        current_encoded_enemy_data[1] = encode_enemy_details_1();
        current_encoded_enemy_data[2] = encode_enemy_details_2();
        current_encoded_enemy_data[3] = current_level_code.player_column;

        // Compare with previous encoded values (positions 16-19)
        for (UBYTE i = 0; i < 4; i++)
        {
            if (current_encoded_enemy_data[i] != previous_encoded_enemy_data[i])
            {
                mark_display_position_for_update(TOTAL_BLOCKS + i);
            }
        }

        // Check individual enemy positions for positions 20-22
        for (UBYTE i = 3; i < 6; i++)
        {
            if (current_level_code.enemy_positions[i] != previous_level_code.enemy_positions[i])
            {
                mark_display_position_for_update(TOTAL_BLOCKS + 4 + (i - 3)); // Positions 20-22
            }
        }

        // Check enemy directions for position 23
        if (current_level_code.enemy_directions != previous_level_code.enemy_directions)
        {
            mark_display_position_for_update(23);
        }

        // Update previous cache
        for (UBYTE i = 0; i < 4; i++)
        {
            previous_encoded_enemy_data[i] = current_encoded_enemy_data[i];
        }
    }

    // Update the cache
    previous_level_code = current_level_code;
}

// ============================================================================
// CORE LEVEL CODE FUNCTIONS
// ============================================================================

// Main level code initialization
void init_level_code(void) BANKED
{
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = 0;
    }
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;
    current_level_code.player_column = 0;

    // Initialize valid player positions
    update_valid_player_positions();

    // Ensure player starts at a valid position
    if (valid_player_count > 0 && !is_valid_player_position(current_level_code.player_column))
    {
        current_level_code.player_column = valid_player_columns[0];
    }
}

// Unified update function that coordinates all subsystems
void update_complete_level_code(void) BANKED
{
    extract_platform_data();
    extract_enemy_data();
    extract_player_data();
}

// ============================================================================
// DISPLAY SYSTEM FUNCTIONS
// ============================================================================

// Selective level code display - prevents flicker by only updating changed chars
void display_selective_level_code(void) BANKED
{
    update_complete_level_code();
    detect_level_code_changes();

    // Only update characters that have changed
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (display_position_needs_update(i))
        {
            get_display_position(i, &display_x, &display_y);
            display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        }
    }

    // Display player column (position 16)
    if (display_position_needs_update(16))
    {
        get_display_position(16, &display_x, &display_y);
        display_char_at_position(current_level_code.player_column, display_x, display_y);
    }

    // Display enemy data (positions 17-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),                                                                   // Position 17: Enemy count + position checksum
        encode_enemy_details_1(),                                                                   // Position 18: Compressed positions for first 3 enemies
        encode_enemy_details_2(),                                                                   // Position 19: Enemy directions + type bits
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            display_char_at_position(enemy_data[i], display_x, display_y);
        }
    }

    // Clear update flags after updating
    clear_display_update_flags();
}

// Fast selective update that doesn't re-extract all data
void display_selective_level_code_fast(void) BANKED
{
    // DON'T call update_complete_level_code() - assume data is already correct
    detect_level_code_changes();

    // Only update characters that have changed
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (display_position_needs_update(i))
        {
            get_display_position(i, &display_x, &display_y);
            display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        }
    }

    // Display player column (position 16)
    if (display_position_needs_update(16))
    {
        get_display_position(16, &display_x, &display_y);
        UBYTE display_char = get_extended_display_char(current_level_code.player_column);
        display_char_at_position(display_char, display_x, display_y);
    }

    // Display enemy data (positions 17-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),                                                                   // Position 17: Enemy count + position checksum
        encode_enemy_details_1(),                                                                   // Position 18: Compressed positions for first 3 enemies
        encode_enemy_details_2(),                                                                   // Position 19: Enemy directions + type bits
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            display_char_at_position(enemy_data[i], display_x, display_y);
        }
    }

    // Clear update flags after updating
    clear_display_update_flags();
}

// Force a complete redraw (useful for initialization)
void force_complete_level_code_display(void) BANKED
{
    update_complete_level_code();
    clear_level_code_display();

    // Declare display_x and display_y for use in this function
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        get_display_position(i, &display_x, &display_y);
        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
    }

    // Display player column (position 16)
    get_display_position(16, &display_x, &display_y);
    UBYTE player_display_char = get_extended_display_char(current_level_code.player_column);
    display_char_at_position(player_display_char, display_x, display_y);

    // Display enemy data (positions 17-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),                                                                   // Position 17: Enemy count + position checksum
        encode_enemy_details_1(),                                                                   // Position 18: Compressed positions for first 3 enemies
        encode_enemy_details_2(),                                                                   // Position 19: Enemy directions + type bits
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        get_display_position(pos, &display_x, &display_y);
        display_char_at_position(enemy_data[i], display_x, display_y);
    }

    // Initialize the cache after complete redraw
    previous_level_code = current_level_code;
    level_code_initialized = 1;
    clear_display_update_flags();
}

// Main level code display function
void display_complete_level_code(void) BANKED
{
    // Use selective update to prevent flicker
    display_selective_level_code();
}

// ============================================================================
// CHARACTER DISPLAY FUNCTIONS
// ============================================================================

UBYTE get_extended_display_char(UBYTE value) BANKED
{
    if (value < 10)
    {
        return '0' + value;
    }
    else if (value < 35)
    {
        return 'A' + (value - 10);
    }
    return 'Z'; // Default for invalid values
}

void display_char_at_position(UBYTE display_char, UBYTE x, UBYTE y) BANKED
{
    UBYTE tile_index;

    if (display_char >= '0' && display_char <= '9')
    {
        tile_index = 48 + (display_char - '0');
    }
    else if (display_char >= 'A' && display_char <= 'Z')
    {
        UBYTE letter_offset = display_char - 'A';
        tile_index = 58 + letter_offset;
    }
    else
    {
        tile_index = 48; // Default to '0'
    }

    replace_meta_tile(x, y, tile_index, 1);
}

void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    // Convert numeric value to display character, then use same mapping as other chars
    UBYTE display_char = get_extended_display_char(value);
    display_char_at_position(display_char, x, y);
}

void clear_level_code_display(void) BANKED
{
    // Only clear the actual level code character positions, not the spaces/background
    for (UBYTE i = 0; i < LEVEL_CODE_CHARS_TOTAL; i++)
    {
        UBYTE display_x, display_y;
        get_display_position(i, &display_x, &display_y);
        replace_meta_tile(display_x, display_y, 0, 1);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert display position (x,y) to character index (0-23)
UBYTE get_char_index_from_display_position(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within the level code display area
    if (x < LEVEL_CODE_START_X || y < LEVEL_CODE_START_Y || y >= LEVEL_CODE_START_Y + 2)
    {
        return 255; // Invalid position
    }

    UBYTE rel_x = x - LEVEL_CODE_START_X;
    UBYTE rel_y = y - LEVEL_CODE_START_Y;

    // Layout: 3 blocks of 4 characters per row, with spaces between blocks
    // Row format: "0000 0000 0000" (positions 0,1,2,3, 5,6,7,8, 10,11,12,13)

    // Check if we're on a space (positions 4, 9, 14)
    if (rel_x == 4 || rel_x == 9 || rel_x == 14)
    {
        return 255; // Space position, not a character
    }

    // Calculate which block (0, 1, or 2)
    UBYTE block;
    UBYTE pos_in_block;

    if (rel_x < 4)
    {
        block = 0;
        pos_in_block = rel_x;
    }
    else if (rel_x < 9)
    {
        block = 1;
        pos_in_block = rel_x - 5; // Skip the space at position 4
    }
    else if (rel_x < 14)
    {
        block = 2;
        pos_in_block = rel_x - 10; // Skip spaces at positions 4 and 9
    }
    else
    {
        return 255; // Beyond valid range
    }

    // Calculate final character index
    UBYTE char_index = rel_y * 12 + block * 4 + pos_in_block;

    return (char_index < LEVEL_CODE_CHARS_TOTAL) ? char_index : 255;
}
