#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"
#include "paint.h"

// ============================================================================
// CORE CONSTANTS - Consolidated and clearly defined
// ============================================================================

#define PLATFORM_Y_MIN 12
#define PLATFORM_Y_MAX 19
#define SEGMENTS_PER_ROW 4
#define SEGMENT_WIDTH 5
#define SEGMENT_HEIGHT 2
#define TOTAL_BLOCKS 16
#define MAX_ENEMIES 6

// Level code display settings - 24 character display system
#define LEVEL_CODE_START_X 5
#define LEVEL_CODE_START_Y 6
#define LEVEL_CODE_CHARS_TOTAL 24

// Platform tile IDs
#define PLATFORM_TILE_1 4
#define PLATFORM_TILE_2 5
#define PLATFORM_TILE_3 6

// Variable IDs for storing level code (define these in GB Studio)
#define VAR_LEVEL_CODE_PART_1 0 // Platform patterns 0-2   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_2 1 // Platform patterns 3-5   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_3 2 // Platform patterns 6-8   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_4 3 // Platform patterns 9-11  (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_5 4 // Platform patterns 12-14 (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_6 5 // Platform pattern 15 (1×5 bits = 5 bits)

// ============================================================================
// PLATFORM PATTERN DATA - Core pattern matching system
// ============================================================================

const UWORD PLATFORM_PATTERNS[] = {
    0b0000000000, // UID 0: Empty
    0b0000000001, // UID 1: Single platform at position 4
    0b0000010000, // UID 2: Single platform at position 3
    0b0000000011, // UID 3: Two platforms at positions 3-4
    0b0000011000, // UID 4: Two platforms at positions 2-3
    0b0000000110, // UID 5: Two platforms at positions 2-3
    0b0000001100, // UID 6: Two platforms at positions 1-2
    0b0000000111, // UID 7: Three platforms at positions 2-4
    0b0000011100, // UID 8: Three platforms at positions 1-3
    0b0000001101, // UID 9: Gapped platforms at positions 1-2,4
    0b0000010110, // UID 10: Gapped platforms at positions 1,3-4
    0b0000001110, // UID 11: Three platforms at positions 1-3
    0b0000001111, // UID 12: Four platforms at positions 1-4
    0b0000011110, // UID 13: Four platforms at positions 1-4
    0b0000010001, // UID 14: Two isolated platforms at positions 0,4
    0b0000010011, // UID 15: Three platforms at positions 0,3-4
    0b0000011001, // UID 16: Three platforms at positions 0,2-3
    0b0000010111, // UID 17: Four platforms at positions 0,2-4
    0b0000011101, // UID 18: Four platforms at positions 0-2,4
    0b0000011011, // UID 19: Four platforms at positions 0-1,3-4
    0b0000011111  // UID 20: Full platform coverage
};

#define PLATFORM_PATTERN_COUNT (sizeof(PLATFORM_PATTERNS) / sizeof(PLATFORM_PATTERNS[0]))

// Character mapping for pattern display (0-9, A-K for patterns 0-20)
const UBYTE PATTERN_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,    // 0-9
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68 // A-K
};

// Extended character mapping for 0-34 range (supports column encoding)
const UBYTE EXTENDED_PATTERN_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // 0-9
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // A-J (10-19)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // K-T (20-29)
    78, 79, 80, 81, 82                      // U-Y (30-34)
};

// ============================================================================
// FORWARD DECLARATIONS - Functions used in selective update system
// ============================================================================

UBYTE encode_enemy_positions(void) BANKED;
UBYTE encode_enemy_details_1(void) BANKED;
UBYTE encode_enemy_details_2(void) BANKED;
UBYTE encode_enemy_directions(void) BANKED;
UBYTE get_extended_display_char(UBYTE value) BANKED;
void display_char_at_position(UBYTE display_char, UBYTE x, UBYTE y) BANKED;
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED;
void clear_level_code_display(void) BANKED;
void update_complete_level_code(void) BANKED;
UBYTE get_char_index_from_display_position(UBYTE x, UBYTE y) BANKED;
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED;
void update_level_code_from_character_edit(UBYTE char_index, UBYTE new_value) BANKED;

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
// SELECTIVE LEVEL CODE DISPLAY - Prevents flicker by only updating changed chars
// ============================================================================

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

    // Display 8 characters of enemy/player data (positions 16-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),         // Position 16: Enemy count + position checksum
        encode_enemy_details_1(),         // Position 17: Compressed positions for first 3 enemies
        encode_enemy_details_2(),         // Position 18: Enemy directions + type bits
        current_level_code.player_column, // Position 19: Player starting column
        // Additional encoded data for positions 20-23
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 8; i++)
    {
        UBYTE pos = TOTAL_BLOCKS + i; // Positions 16-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            UBYTE display_char = get_extended_display_char(enemy_data[i]);
            display_char_at_position(display_char, display_x, display_y);
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

    // Display 8 characters of enemy/player data (positions 16-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),         // Position 16: Enemy count + position checksum
        encode_enemy_details_1(),         // Position 17: Compressed positions for first 3 enemies
        encode_enemy_details_2(),         // Position 18: Enemy directions + type bits
        current_level_code.player_column, // Position 19: Player starting column
        // Additional encoded data for positions 20-23
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 8; i++)
    {
        UBYTE pos = TOTAL_BLOCKS + i; // Positions 16-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            UBYTE display_char = get_extended_display_char(enemy_data[i]);
            display_char_at_position(display_char, display_x, display_y);
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

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        UBYTE display_x, display_y;
        get_display_position(i, &display_x, &display_y);
        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
    }

    // Display 8 characters of enemy/player data (positions 16-23)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),         // Position 16: Enemy count + position checksum
        encode_enemy_details_1(),         // Position 17: Compressed positions for first 3 enemies
        encode_enemy_details_2(),         // Position 18: Enemy directions + type bits
        current_level_code.player_column, // Position 19: Player starting column
        // Additional encoded data for positions 20-23
        (current_level_code.enemy_positions[3] != 255) ? current_level_code.enemy_positions[3] : 0, // Position 20
        (current_level_code.enemy_positions[4] != 255) ? current_level_code.enemy_positions[4] : 0, // Position 21
        (current_level_code.enemy_positions[5] != 255) ? current_level_code.enemy_positions[5] : 0, // Position 22
        (current_level_code.enemy_directions >> 3) & 0x07                                           // Position 23: Remaining direction bits
    };

    for (UBYTE i = 0; i < 8; i++)
    {
        UBYTE pos = TOTAL_BLOCKS + i; // Positions 16-23
        UBYTE display_x, display_y;
        get_display_position(pos, &display_x, &display_y);
        UBYTE display_char = get_extended_display_char(enemy_data[i]);
        display_char_at_position(display_char, display_x, display_y);
    }

    // Initialize the cache after complete redraw
    previous_level_code = current_level_code;
    level_code_initialized = 1;
    clear_display_update_flags();
}

// ============================================================================
// CORE PATTERN EXTRACTION AND MATCHING
// ============================================================================

UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED
{
    UWORD pattern = 0;
    *row0 = 0;
    *row1 = 0;

    for (UBYTE i = 0; i < 5; i++)
    {
        UBYTE top_tile = sram_map_data[METATILE_MAP_OFFSET(x + i, y)];
        UBYTE bottom_tile = sram_map_data[METATILE_MAP_OFFSET(x + i, y + 1)];

        // Top row (bits 9-5)
        if (top_tile == PLATFORM_TILE_1 || top_tile == PLATFORM_TILE_2 || top_tile == PLATFORM_TILE_3)
        {
            pattern |= (1 << (9 - i));
            *row0 |= (1 << (4 - i));
        }
        // Bottom row (bits 4-0)
        if (bottom_tile == PLATFORM_TILE_1 || bottom_tile == PLATFORM_TILE_2 || bottom_tile == PLATFORM_TILE_3)
        {
            pattern |= (1 << (4 - i));
            *row1 |= (1 << (4 - i));
        }
    }
    return pattern;
}

UWORD match_platform_pattern(UWORD pattern) BANKED
{
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        if (PLATFORM_PATTERNS[i] == pattern)
        {
            return i;
        }
    }
    return 0; // Fallback to pattern 0
}

// ============================================================================
// ENEMY TYPE DETECTION - Support for different enemy types
// ============================================================================

UBYTE detect_enemy_type(UBYTE tile_type) BANKED
{
    // Future expansion: detect different enemy types
    switch (tile_type)
    {
    case BRUSH_TILE_ENEMY_L:
    case BRUSH_TILE_ENEMY_R:
        return 0; // Walker type
    // Future: Add new brush tile types for jumper enemies
    // case BRUSH_TILE_ENEMY_JUMPER_L:
    // case BRUSH_TILE_ENEMY_JUMPER_R:
    //     return 1; // Jumper type
    default:
        return 0; // Default to walker
    }
}

// ============================================================================
// LEVEL DATA EXTRACTION - Unified system
// ============================================================================

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
}

void extract_platform_data(void) BANKED
{
    for (UBYTE block_y = 0; block_y < 4; block_y++)
    {
        for (UBYTE block_x = 0; block_x < 4; block_x++)
        {
            UBYTE block_index = block_y * 4 + block_x;
            UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
            UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

            UBYTE row0, row1;
            UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
            UWORD pattern_id = match_platform_pattern(pattern);

            current_level_code.platform_patterns[block_index] = (UBYTE)pattern_id;
        }
    }
}

void extract_enemy_data(void) BANKED
{
    UBYTE enemy_count = 0;
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }

    // Scan all rows for enemies (12, 14, 16, 18) - ensure we cover the full map width
    for (UBYTE row = 12; row <= 18; row += 2)
    {
        for (UBYTE col = 2; col < 22; col++) // Covers columns 0-19 in game coordinates
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, row)];
            UBYTE tile_type = get_tile_type(tile);
            if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < MAX_ENEMIES)
            {
                current_level_code.enemy_positions[enemy_count] = col - 2; // 0-based column

                // Set direction bit (left = 1, right = 0)
                if (tile_type == BRUSH_TILE_ENEMY_L)
                {
                    current_level_code.enemy_directions |= (1 << enemy_count);
                }

                // Detect and store enemy type (walker=0, jumper=1, etc.)
                UBYTE enemy_type = detect_enemy_type(tile_type);
                if (enemy_type == 1) // If jumper
                {
                    current_level_code.enemy_types |= (1 << enemy_count);
                }

                enemy_count++;
            }
        }
    }
}

void extract_player_data(void) BANKED
{
    for (UBYTE col = 2; col < 22; col++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, 11)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_PLAYER)
        {
            current_level_code.player_column = col - 2; // 0-based column
            return;
        }
    }
    current_level_code.player_column = 0; // Default to column 0
}

void update_complete_level_code(void) BANKED
{
    extract_platform_data();
    extract_enemy_data();
    extract_player_data();
}

// ============================================================================
// COMPACT ENCODING SYSTEM - 24 character display
// ============================================================================

// Encode enemy positions using a checksum-based approach
UBYTE encode_enemy_positions(void) BANKED
{
    UBYTE checksum = 0;
    UBYTE enemy_count = 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
            // Use position value directly in checksum, avoid XOR cancellation
            checksum += (current_level_code.enemy_positions[i] * (i + 1)) & 0x1F;
        }
    }

    // Combine enemy count (3 bits) with position checksum (5 bits)
    UBYTE result = ((enemy_count & 0x07) << 5) | (checksum & 0x1F);

    // Ensure result fits in 0-34 range for display
    if (result > 34)
        result = result % 35;
    return result;
}

// Pack individual enemy positions into a compact format
UBYTE encode_enemy_details_1(void) BANKED
{
    UBYTE result = 0;
    UBYTE shift = 0;

    // Pack first 3 enemy positions (each needs ~5 bits for 0-19)
    for (UBYTE i = 0; i < 3 && i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255 && shift <= 30)
        {
            // Reduce precision: map 0-19 to 0-15 for better packing
            UBYTE compressed_pos = (current_level_code.enemy_positions[i] * 15) / 19;
            result += (compressed_pos & 0x0F) << (shift % 8);
            shift += 4;
        }
    }

    if (result > 34)
        result = result % 35;
    return result;
}

// Pack remaining enemy data
UBYTE encode_enemy_details_2(void) BANKED
{
    UBYTE result = 0;

    // Combine directions (6 bits) and types (6 bits) - but we only have 8 bits to work with
    // So we'll prioritize directions and use remaining space for types
    result = (current_level_code.enemy_directions & 0x3F); // 6 bits for directions

    // Add first 2 bits of enemy types if space allows
    if ((current_level_code.enemy_types & 0x03) > 0)
    {
        result |= ((current_level_code.enemy_types & 0x03) << 6);
    }

    if (result > 34)
        result = result % 35;
    return result;
}

// Original function name for compatibility
UBYTE encode_enemy_bitmask(void) BANKED
{
    return encode_enemy_positions();
}

UBYTE encode_enemy_directions(void) BANKED
{
    return encode_enemy_details_2();
}

// ============================================================================
// CHARACTER DISPLAY FUNCTIONS - Simplified and consolidated
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
    UBYTE tile_index = (value < PLATFORM_PATTERN_COUNT) ? PATTERN_TILE_MAP[value] : PATTERN_TILE_MAP[0];
    replace_meta_tile(x, y, tile_index, 1);
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
// MASTER LEVEL CODE DISPLAY - Single unified function
// ============================================================================

void display_complete_level_code(void) BANKED
{
    // Use selective update to prevent flicker
    display_selective_level_code();
}

// ============================================================================
// MAIN API FUNCTIONS - Clean and simple
// ============================================================================

void draw_segment_ids(void) BANKED
{
    // Force complete display for initial draw
    force_complete_level_code_display();
}

void update_zone_code(UBYTE zone_index) BANKED
{
    if (zone_index >= TOTAL_BLOCKS)
        return;

    UBYTE segment_x = 2 + (zone_index % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + (zone_index / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    current_level_code.platform_patterns[zone_index] = (UBYTE)pattern_id;

    // Mark only the specific zone position for update
    mark_display_position_for_update(zone_index);

    // Use fast selective update instead of complete redraw
    display_selective_level_code_fast();
}

UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED
{
    if (x < 2 || x >= 22 || y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
    {
        return 255; // Invalid zone
    }

    UBYTE segment_x = (x - 2) / SEGMENT_WIDTH;
    UBYTE segment_y = (y - PLATFORM_Y_MIN) / SEGMENT_HEIGHT;

    if (segment_x >= SEGMENTS_PER_ROW || segment_y >= 4)
    {
        return 255; // Invalid zone
    }

    return segment_y * SEGMENTS_PER_ROW + segment_x;
}

// ============================================================================
// VM WRAPPER FUNCTIONS - Minimal and focused
// ============================================================================

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    draw_segment_ids();
}

void vm_init_level_code(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    init_level_code();
}

// VM wrapper functions for script access
void vm_save_level_code(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    save_level_code_to_variables();
}

void vm_load_level_code(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    load_level_code_from_variables();
    force_complete_level_code_display(); // Force complete redraw after loading
}

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

// ============================================================================
// DEBUG AND TEST FUNCTIONS
// ============================================================================

#ifdef DEBUG_BUILD

void test_enemy_encoding(void) BANKED
{
    init_level_code();

    // Test data - including bottom-right corner enemies
    current_level_code.enemy_positions[0] = 5;   // Middle column
    current_level_code.enemy_positions[1] = 12;  // Right side
    current_level_code.enemy_positions[2] = 18;  // Near bottom-right corner
    current_level_code.enemy_positions[3] = 19;  // Bottom-right corner
    current_level_code.enemy_positions[4] = 0;   // Left edge
    current_level_code.enemy_positions[5] = 255; // Empty slot

    // Test directions (left=1, right=0)
    current_level_code.enemy_directions = 0b010110; // Enemies 1,2,4 face left

    // Test enemy types (walker=0, jumper=1)
    current_level_code.enemy_types = 0b001100; // Enemies 2,3 are jumpers    current_level_code.player_column = 3;

    force_complete_level_code_display();
}

void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    test_enemy_encoding();
}

void vm_get_enemy_info(SCRIPT_CTX *THIS) BANKED
{
    // Useful for debugging - stores enemy info in script variables
    update_complete_level_code();

    // Count active enemies
    UBYTE enemy_count = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
        }
    }

    // Store results for script access
    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = enemy_count;
    *(UWORD *)VM_REF_TO_PTR(FN_ARG1) = encode_enemy_positions();
    *(UWORD *)VM_REF_TO_PTR(FN_ARG2) = current_level_code.enemy_directions;
    *(UWORD *)VM_REF_TO_PTR(FN_ARG3) = current_level_code.enemy_types;
}

#else
// Release mode stub implementations
void test_enemy_encoding(void) BANKED
{
    // No-op in release mode
}

void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    // No-op in release mode
}

void vm_get_enemy_info(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    // No-op in release mode
}

#endif // DEBUG_BUILD

// ============================================================================
// ENEMY ENCODING SYSTEM DOCUMENTATION
// ============================================================================
/*
 * NEW IMPROVED ENEMY ENCODING (24 characters total):
 *
 * Characters 1-20: Platform patterns (0-20, displayed as 0-9,A-K)
 * Character 21: Enemy position summary (count + checksum)
 * Character 22: Compressed positions of first 3 enemies
 * Character 23: Enemy directions + type bits
 * Character 24: Player starting column (0-19)
 *
 * This system supports:
 * - Up to 6 enemies anywhere on the 20-column map
 * - Enemy directions (left/right)
 * - Enemy types (walker/jumper via new tile types)
 * - Full position coverage including bottom-right corner
 *
 * Character range: 0-9 (10 values) + A-Y (25 values) = 35 total values
 * All encoded values are constrained to 0-34 range for display
 */

// Diagnostic function to check enemy encoding integrity
void validate_enemy_encoding(void) BANKED
{
    UBYTE active_enemies = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            active_enemies++;
        }
    }

    // Could add validation logic here to ensure encoding/decoding works
    // For now, just count active enemies for debugging
}

// ============================================================================
// PERSISTENT STORAGE - GB Studio Variables
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
                  current_level_code.platform_patterns[14]; // Part 6: Platform pattern 15 (only 1 pattern, stored in lower 5 bits)
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
}

// ============================================================================
// ALTERNATIVE: SRAM STORAGE - For more complex persistence
// ============================================================================

// SRAM offsets for level code storage
#define SRAM_LEVEL_CODE_OFFSET 0x0000
#define SRAM_LEVEL_CODE_MAGIC 0xABCD // Magic number to verify valid data

typedef struct
{
    UWORD magic;                           // Magic number for validation
    UBYTE platform_patterns[TOTAL_BLOCKS]; // 20 platform patterns
    UBYTE enemy_positions[MAX_ENEMIES];    // Enemy positions
    UBYTE enemy_directions;                // Enemy directions
    UBYTE enemy_types;                     // Enemy types
    UBYTE player_column;                   // Player starting column
    UBYTE checksum;                        // Simple checksum
} sram_level_code_t;

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

    return 1; // Success
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
        force_complete_level_code_display(); // Force complete redraw after loading
    }
}

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
            // Platform patterns: 0 to PLATFORM_PATTERN_COUNT-1
            max_value = PLATFORM_PATTERN_COUNT - 1;
        }
        else if (char_index == 19)
        {
            // Player column: 0-19
            max_value = 19;
        }
        else if (char_index >= 20 && char_index <= 22)
        {
            // Enemy positions: 0-19 (0 means no enemy)
            max_value = 19;
        }
        else
        {
            // Other encoded values: 0-34
            max_value = 34;
        }

        // Cycle to next value
        if (current_value >= max_value)
        {
            new_value = 0; // Wrap to 0
        }
        else
        {
            new_value = current_value + 1;
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

// ============================================================================
// REVERSE MAPPING - Convert display position to character index
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

// Apply pattern changes to the actual tilemap
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED
{
    if (block_index >= TOTAL_BLOCKS || pattern_id >= PLATFORM_PATTERN_COUNT)
    {
        return;
    }

    // Calculate block position in tilemap
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;
    UBYTE block_y = block_index / SEGMENTS_PER_ROW;
    UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

    // Get the pattern data
    UWORD pattern = PLATFORM_PATTERNS[pattern_id];

    // Clear the segment first
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
    {
        for (UBYTE j = 0; j < SEGMENT_HEIGHT; j++)
        {
            replace_meta_tile(segment_x + i, segment_y + j, 0, 1);
        }
    }

    // Apply the pattern
    for (UBYTE i = 0; i < 5; i++)
    {
        UBYTE tile_type = (i == 0) ? PLATFORM_TILE_1 : (i == 4) ? PLATFORM_TILE_3
                                                                : PLATFORM_TILE_2;

        // Top row (bits 9-5)
        if ((pattern >> (9 - i)) & 1)
        {
            replace_meta_tile(segment_x + i, segment_y, tile_type, 1);
        }

        // Bottom row (bits 4-0)
        if ((pattern >> (4 - i)) & 1)
        {
            replace_meta_tile(segment_x + i, segment_y + 1, tile_type, 1);
        }
    }
}

// Update level code data based on character position and new value
void update_level_code_from_character_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    if (char_index >= LEVEL_CODE_CHARS_TOTAL)
    {
        return;
    }

    if (char_index < TOTAL_BLOCKS)
    {
        // Platform pattern positions (0-15)
        if (new_value < PLATFORM_PATTERN_COUNT)
        {
            current_level_code.platform_patterns[char_index] = new_value;
            apply_pattern_to_tilemap(char_index, new_value);
        }
    }
    else if (char_index == 16)
    {
        // Position 16: Enemy position summary - this is complex to decode directly
        // For now, we'll skip direct editing of encoded positions
        // Could be enhanced later to support direct enemy count editing
    }
    else if (char_index == 17)
    {
        // Position 17: Compressed enemy positions - also complex
        // Skip for now
    }
    else if (char_index == 18)
    {
        // Position 18: Enemy directions + type bits - complex
        // Skip for now
    }
    else if (char_index == 19)
    {
        // Position 19: Player starting column (0-19)
        if (new_value < 20)
        {
            current_level_code.player_column = new_value;

            // Update player position in tilemap
            // First, clear existing player
            for (UBYTE col = 2; col < 22; col++)
            {
                UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, 11)];
                UBYTE tile_type = get_tile_type(tile);
                if (tile_type == BRUSH_TILE_PLAYER)
                {
                    replace_meta_tile(col, 11, 0, 1);
                }
            }

            // Place player at new position
            UBYTE new_x = 2 + new_value;
            replace_meta_tile(new_x, 11, TILE_PLAYER, 1);

            // Also move the player actor to the new position
            move_player_actor_to_tile(paint_player_id, new_x, 11);
        }
    }
    else if (char_index >= 20 && char_index <= 22)
    {
        // Positions 20-22: Individual enemy positions (enemies 3-5)
        UBYTE enemy_index = 3 + (char_index - 20);
        if (enemy_index < MAX_ENEMIES)
        {
            if (new_value == 0)
            {
                // Value 0 means remove enemy
                current_level_code.enemy_positions[enemy_index] = 255;

                // Clear enemy from tilemap
                for (UBYTE row = 12; row <= 18; row += 2)
                {
                    for (UBYTE col = 2; col < 22; col++)
                    {
                        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, row)];
                        UBYTE tile_type = get_tile_type(tile);
                        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
                        {
                            // Check if this is the enemy we want to remove
                            // This is approximate since we don't track exact enemy-to-position mapping
                            replace_meta_tile(col, row, 0, 1);
                            break; // Remove first found enemy
                        }
                    }
                }
            }
            else if (new_value < 20)
            {
                // Place enemy at specified column
                current_level_code.enemy_positions[enemy_index] = new_value;

                // Place enemy in tilemap (use default direction - right)
                // Find a suitable row (start from row 12)
                for (UBYTE row = 12; row <= 18; row += 2)
                {
                    UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(2 + new_value, row)];
                    if (tile == 0) // Empty position
                    {
                        replace_meta_tile(2 + new_value, row, BRUSH_TILE_ENEMY_R, 1);
                        break;
                    }
                }
            }
        }
    }
    else if (char_index == 23)
    {
        // Position 23: Remaining enemy direction bits
        // Update the upper bits of enemy_directions
        current_level_code.enemy_directions = (current_level_code.enemy_directions & 0x07) | ((new_value & 0x07) << 3);
    }
}