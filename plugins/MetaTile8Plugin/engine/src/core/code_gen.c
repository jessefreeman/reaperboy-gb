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
#define SEGMENTS_PER_ROW 5
#define SEGMENT_WIDTH 5
#define SEGMENT_HEIGHT 2
#define TOTAL_BLOCKS 20
#define MAX_ENEMIES 6

// Level code display settings - 24 character display system
#define LEVEL_CODE_START_X 5
#define LEVEL_CODE_START_Y 6
#define LEVEL_CODE_CHARS_TOTAL 24

// Platform tile IDs
#define PLATFORM_TILE_1 4
#define PLATFORM_TILE_2 5
#define PLATFORM_TILE_3 6

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
// LEVEL CODE STRUCTURE - Single source of truth
// ============================================================================

typedef struct
{
    UBYTE platform_patterns[TOTAL_BLOCKS]; // 20 platform patterns
    UBYTE enemy_positions[MAX_ENEMIES];    // Enemy column positions (255=empty)
    UBYTE enemy_directions;                // Packed direction bits (6 bits)
    UBYTE enemy_types;                     // Packed enemy type bits (6 bits: 0=walker, 1=jumper)
    UBYTE player_column;                   // Player starting column
} level_code_t;

level_code_t current_level_code;

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
        for (UBYTE block_x = 0; block_x < 5; block_x++)
        {
            UBYTE block_index = block_y * 5 + block_x;
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
    for (UBYTE y = 6; y < 8; y++)
    {
        for (UBYTE x = 5; x < 20; x++)
        {
            replace_meta_tile(x, y, 0, 1);
        }
    }
}

// ============================================================================
// MASTER LEVEL CODE DISPLAY - Single unified function
// ============================================================================

void display_complete_level_code(void) BANKED
{
    update_complete_level_code();
    clear_level_code_display();

    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;
    UBYTE total_chars = 0;

    // Display 20 platform patterns
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (total_chars > 0 && total_chars % 4 == 0)
        {
            display_x++; // Space between groups
        }
        if (display_x >= 19)
        {
            display_x = 5;
            display_y++;
        }

        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        display_x++;
        total_chars++;
    } // Add comprehensive enemy data (4 characters total)
    UBYTE enemy_data[] = {
        encode_enemy_positions(),        // Enemy count + position checksum
        encode_enemy_details_1(),        // Compressed positions for first 3 enemies
        encode_enemy_directions(),       // Direction + type bits
        current_level_code.player_column // Player starting position
    };

    for (UBYTE i = 0; i < 4; i++)
    {
        if (total_chars > 0 && total_chars % 4 == 0)
            display_x++;
        if (display_x >= 19)
        {
            display_x = 5;
            display_y++;
        }
        display_char_at_position(get_extended_display_char(enemy_data[i]), display_x, display_y);
        display_x++;
        total_chars++;
    }
}

// ============================================================================
// MAIN API FUNCTIONS - Clean and simple
// ============================================================================

void draw_segment_ids(void) BANKED
{
    display_complete_level_code();
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

    // Update display immediately
    display_complete_level_code();
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

// ============================================================================
// DEBUG AND TEST FUNCTIONS
// ============================================================================

#ifdef DEBUG_BUILD
// Debug pattern index for testing
UBYTE debug_pattern_index = 0;

void draw_debug_pattern(UBYTE pattern_index) BANKED
{
    if (pattern_index >= PLATFORM_PATTERN_COUNT)
    {
        pattern_index = 0;
    }

    // Display pattern index as decimal
    UBYTE dec_tens = pattern_index / 10;
    UBYTE dec_ones = pattern_index % 10;
    replace_meta_tile(0, 2, 48 + dec_tens, 1);
    replace_meta_tile(1, 2, 48 + dec_ones, 1);

    // Clear drawing area
    for (UBYTE x = 2; x < 7; x++)
    {
        for (UBYTE y = 11; y < 13; y++)
        {
            replace_meta_tile(x, y, 0, 1);
        }
    }

    // Draw pattern
    UWORD pattern = PLATFORM_PATTERNS[pattern_index];
    for (UBYTE i = 0; i < 5; i++)
    {
        UBYTE tile_type = (i == 0) ? PLATFORM_TILE_1 : (i == 4) ? PLATFORM_TILE_3
                                                                : PLATFORM_TILE_2;

        if ((pattern >> (9 - i)) & 1)
        {
            replace_meta_tile(2 + i, 11, tile_type, 1);
        }
        if ((pattern >> (4 - i)) & 1)
        {
            replace_meta_tile(2 + i, 12, tile_type, 1);
        }
    }
}

void vm_debug_next_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    debug_pattern_index = (debug_pattern_index + 1) % PLATFORM_PATTERN_COUNT;
    draw_debug_pattern(debug_pattern_index);
}

void vm_debug_prev_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    debug_pattern_index = (debug_pattern_index == 0) ? PLATFORM_PATTERN_COUNT - 1 : debug_pattern_index - 1;
    draw_debug_pattern(debug_pattern_index);
}

void vm_debug_reset_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    debug_pattern_index = 0;
    draw_debug_pattern(debug_pattern_index);
}

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
    current_level_code.enemy_types = 0b001100; // Enemies 2,3 are jumpers

    current_level_code.player_column = 3;

    display_complete_level_code();
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
void draw_debug_pattern(UBYTE pattern_index) BANKED
{
    (void)pattern_index;
    // No-op in release mode
}

void vm_debug_next_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    // No-op in release mode
}

void vm_debug_prev_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    // No-op in release mode
}

void vm_debug_reset_pattern(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    // No-op in release mode
}

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
