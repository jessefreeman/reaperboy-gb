#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"
#include "paint.h"

#define PLATFORM_Y_MIN 12 // Changed from 11 to 12 to start at y=12
#define PLATFORM_Y_MAX 19 // Maximum Y coordinate for platform placement
#define TILE_0 48
#define SEGMENTS_PER_ROW 5 // 5 blocks horizontally (updated for 5x4 grid)
#define SEGMENT_WIDTH 5    // 5 tiles wide per segment
#define SEGMENT_HEIGHT 2   // 2 tiles high per segment
#define TILE_HEX_DEBUG 96
#define TILE_FLIP_OFFSET 32 // each flip variant lives 2 rows (16 tiles) down

// Level code display settings
#define LEVEL_CODE_START_X 5        // Starting X position for level code display
#define LEVEL_CODE_START_Y 1        // Starting Y position for level code display (moved to avoid conflict)
#define LEVEL_CODE_CHARS_PER_ROW 12 // 12 chars per row (3 blocks of 4 chars)
#define LEVEL_CODE_BLOCK_SIZE 4     // 4 characters per block

// Hex tile definitions - hex chars are at row 3 (y=3), columns 0-15 (x=0-15)
#define HEX_TILE_ROW 3
#define HEX_TILE_0 (HEX_TILE_ROW * 16 + 0)  // Tile at (0,3) = 48
#define HEX_TILE_F (HEX_TILE_ROW * 16 + 15) // Tile at (15,3) = 63

// Platform tile IDs
#define PLATFORM_TILE_1 4
#define PLATFORM_TILE_2 5
#define PLATFORM_TILE_3 6

// Enemy tile IDs - enemies appear on even rows (12, 14, 16, 18)
#define ENEMY_TILE_LEFT 7  // Enemy facing left
#define ENEMY_TILE_RIGHT 8 // Enemy facing right

// Level structure constants
#define MAX_ENEMIES 6    // Maximum 6 enemies per level
#define TOTAL_BLOCKS 20  // 5x4 grid = 20 blocks
#define TOTAL_COLUMNS 25 // 5 blocks * 5 tiles = 25 columns

// Debug display settings
#define DEBUG_START_X 5
#define DEBUG_START_Y 5
#define DEBUG_TILES_PER_ROW 5 // Match the 5x4 chunk grid layout

// Extended character mapping - supports 0-9, A-Y (covers 0-34, enough for 25 columns)
const UBYTE EXTENDED_PATTERN_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // 0-9
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // A-J (10-19)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // K-T (20-29)
    78, 79, 80, 81, 82                      // U-Y (30-34)
};

// Platform patterns - 5x2 chunks with platforms on the 2nd row (bottom row)
// Aligned with alternating platform paint logic for rows 13, 15, 17, 19
// Each pattern corresponds to the UID values from your specification
const UWORD PLATFORM_PATTERNS[] = {
    0b0000000000, // UID 0: Pattern 0 - Empty
    0b0000000001, // UID 1: Pattern 1.0 - Single platform at position 4
    0b0000010000, // UID 2: Pattern 1.1 - Single platform at position 3
    0b0000000011, // UID 3: Pattern 2.0 - Two platforms at positions 3-4
    0b0000011000, // UID 4: Pattern 2.1 - Two platforms at positions 2-3
    0b0000000110, // UID 5: Pattern 3.0 - Two platforms at positions 2-3
    0b0000001100, // UID 6: Pattern 3.1 - Two platforms at positions 1-2
    0b0000000111, // UID 7: Pattern 4.0 - Three platforms at positions 2-4
    0b0000011100, // UID 8: Pattern 4.1 - Three platforms at positions 1-3
    0b0000001101, // UID 9: Pattern 5.0 - Gapped platforms at positions 1-2,4
    0b0000010110, // UID 10: Pattern 5.1 - Gapped platforms at positions 1,3-4
    0b0000001110, // UID 11: Pattern 6.0 & 6.1 - Three platforms at positions 1-3
    0b0000001111, // UID 12: Pattern 7.0 - Four platforms at positions 1-4
    0b0000011110, // UID 13: Pattern 7.1 - Four platforms at positions 1-4
    0b0000010001, // UID 14: Pattern 8.0 & 8.1 - Two isolated platforms at positions 0,4
    0b0000010011, // UID 15: Pattern 9.0 - Three platforms at positions 0,3-4
    0b0000011001, // UID 16: Pattern 9.1 - Three platforms at positions 0,2-3
    0b0000010111, // UID 17: Pattern 10.0 - Four platforms at positions 0,2-4
    0b0000011101, // UID 18: Pattern 10.1 - Four platforms at positions 0-2,4
    0b0000011011, // UID 19: Pattern 11.0 & 11.1 - Four platforms at positions 0-1,3-4
    0b0000011111  // UID 20: Pattern 12.0 & 12.1 - Full platform coverage
};

#define PLATFORM_PATTERN_COUNT (sizeof(PLATFORM_PATTERNS) / sizeof(PLATFORM_PATTERNS[0]))

// Tile mapping for each pattern ID - you can manually set each tile index here
// This array maps pattern IDs (0-20) to their corresponding tile indices
const UBYTE PATTERN_TILE_MAP[] = {
    48, // Pattern 0  -> '0' at (0,3)
    49, // Pattern 1  -> '1' at (1,3)
    50, // Pattern 2  -> '2' at (2,3)
    51, // Pattern 3  -> '3' at (3,3)
    52, // Pattern 4  -> '4' at (4,3)
    53, // Pattern 5  -> '5' at (5,3)
    54, // Pattern 6  -> '6' at (6,3)
    55, // Pattern 7  -> '7' at (7,3)
    56, // Pattern 8  -> '8' at (8,3)
    57, // Pattern 9  -> '9' at (9,3)
    58, // Pattern 10 -> 'A' at (10,3)
    59, // Pattern 11 -> 'B' at (11,3)
    60, // Pattern 12 -> 'C' at (12,3)
    61, // Pattern 13 -> 'D' at (13,3)
    62, // Pattern 14 -> 'E' at (14,3)
    63, // Pattern 15 -> 'F' at (15,3)
    64, // Pattern 16 -> 'G' at (4,0) - CHANGE THIS TO CORRECT TILE INDEX
    65, // Pattern 17 -> 'H' at (5,0) - CHANGE THIS TO CORRECT TILE INDEX
    66, // Pattern 18 -> 'I' at (6,0) - CHANGE THIS TO CORRECT TILE INDEX
    67, // Pattern 19 -> 'J' at (7,0) - CHANGE THIS TO CORRECT TILE INDEX
    68  // Pattern 20 -> 'K' at (8,0) - CHANGE THIS TO CORRECT TILE INDEX
};

// Level code storage - expanded for complete level encoding
typedef struct
{
    UBYTE platform_patterns[TOTAL_BLOCKS]; // 20 chars: platform patterns (0-K)
    UBYTE enemy_positions[MAX_ENEMIES];    // 6 chars: enemy column positions (0-Y, 255=empty)
    UBYTE enemy_directions;                // 1 char: packed direction bits (0=right, 1=left)
    UBYTE player_column;                   // 1 char: player starting column (0-Y)
    UBYTE reserved[2];                     // 2 chars: for future expansion
} level_code_t;                            // Total: 30 characters (under our 36 limit)

// Current level data
level_code_t current_level_code;

// Debug function to draw patterns sequentially
UBYTE debug_pattern_index = 0;

// Helper functions for character display and encoding
UBYTE get_pattern_display_char(UBYTE pattern_id) BANKED
{
    if (pattern_id < PLATFORM_PATTERN_COUNT)
    {
        // Return the actual character (0-9, A-K) for the pattern
        if (pattern_id < 10)
        {
            return '0' + pattern_id;
        }
        else
        {
            return 'A' + (pattern_id - 10);
        }
    }
    return '0'; // Default to '0' for invalid patterns
}

UBYTE get_extended_display_char(UBYTE value) BANKED
{
    // Map 0-34 to characters 0-9, A-Y
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
    // Convert character to tile index and display it
    UBYTE tile_index;

    if (display_char >= '0' && display_char <= '9')
    {
        // Numbers: use tiles 48-57 (row 3, columns 0-9)
        tile_index = 48 + (display_char - '0');
    }
    else if (display_char >= 'A' && display_char <= 'Z')
    {
        // Letters A-Z: use tiles 58-83 (row 3 col 10-15, then row 4 col 0-9, then row 5 col 0-5)
        UBYTE letter_offset = display_char - 'A';
        tile_index = 58 + letter_offset;
    }
    else
    {
        // Default to '0' for unknown characters
        tile_index = 48;
    }

    // Use the existing meta tile replacement function
    replace_meta_tile(x, y, tile_index, 1);
}

UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED
{
    UWORD pattern = 0;
    *row0 = 0;
    *row1 = 0;

    // Extract 5x2 pattern into a 10-bit value
    for (UBYTE i = 0; i < 5; i++) // 5 tiles wide
    {
        UBYTE top_tile = sram_map_data[METATILE_MAP_OFFSET(x + i, y)];
        UBYTE bottom_tile = sram_map_data[METATILE_MAP_OFFSET(x + i, y + 1)];

        // Top row (bits 9-5) - check if tile is platform type (4, 5, or 6)
        if (top_tile == PLATFORM_TILE_1 || top_tile == PLATFORM_TILE_2 || top_tile == PLATFORM_TILE_3)
        {
            pattern |= (1 << (9 - i)); // Set bit for top row
            *row0 |= (1 << (4 - i));   // Keep for compatibility
        }
        // Bottom row (bits 4-0) - check if tile is platform type (4, 5, or 6)
        if (bottom_tile == PLATFORM_TILE_1 || bottom_tile == PLATFORM_TILE_2 || bottom_tile == PLATFORM_TILE_3)
        {
            pattern |= (1 << (4 - i)); // Set bit for bottom row
            *row1 |= (1 << (4 - i));   // Keep for compatibility
        }
    }
    return pattern;
}

UWORD match_platform_pattern(UWORD pattern) BANKED
{
    // Search through the flat array of unique patterns
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        if (PLATFORM_PATTERNS[i] == pattern)
        {
            return i; // Return the unique pattern ID (0-20)
        }
    }
    return 0; // fallback to pattern 0
}

// Initialize level code structure
void init_level_code(void) BANKED
{
    // Clear platform patterns
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = 0;
    }

    // Clear enemy data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255; // 255 = no enemy
    }
    current_level_code.enemy_directions = 0;
    current_level_code.player_column = 0;
    current_level_code.reserved[0] = 0;
    current_level_code.reserved[1] = 0;
}

// Extract enemy positions from the map (enemies on even rows: 12, 14, 16, 18)
void extract_enemy_data(void) BANKED
{
    UBYTE enemy_count = 0;
    current_level_code.enemy_directions = 0;

    // Clear enemy positions
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }

    // Scan even rows for enemies (rows 12, 14, 16, 18)
    for (UBYTE row = 12; row <= 18; row += 2)
    {
        for (UBYTE col = 2; col < 22; col++) // Scan columns 2-21 (20 tiles wide)
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, row)];
            UBYTE tile_type = get_tile_type(tile);

            // Check for enemy tiles using the tile type (brush tile constants)
            if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < MAX_ENEMIES)
            {
                // Store enemy column position (relative to level start)
                current_level_code.enemy_positions[enemy_count] = col - 2; // 0-based column

                // Set direction bit (0=right, 1=left)
                if (tile_type == BRUSH_TILE_ENEMY_L)
                {
                    current_level_code.enemy_directions |= (1 << enemy_count);
                }

                enemy_count++;
            }
        }
    }
}

// Extract player starting position
void extract_player_data(void) BANKED
{
    // Scan row 11 for player tile
    for (UBYTE col = 2; col < 22; col++) // Scan columns 2-21 (20 tiles wide)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, 11)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_PLAYER)
        {
            current_level_code.player_column = col - 2; // 0-based column
            return;
        }
    }

    // If no player found, default to column 0
    current_level_code.player_column = 0;
}

// Update complete level code (platforms + enemies + player)
void update_complete_level_code(void) BANKED
{
    // Extract platform patterns for all 20 blocks (5x4 grid)
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

    // Extract enemy and player data
    extract_enemy_data();
    extract_player_data();
}

// Display extended character (supports 0-34)
void display_extended_char(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    UBYTE tile_index;

    if (value < sizeof(EXTENDED_PATTERN_TILE_MAP))
    {
        tile_index = EXTENDED_PATTERN_TILE_MAP[value];
    }
    else
    {
        tile_index = EXTENDED_PATTERN_TILE_MAP[0]; // Fallback to '0'
    }

    replace_meta_tile(x, y, tile_index, 1);
}

// Encode enemy directions into a single character (6 bits + padding)
UBYTE encode_enemy_directions(void) BANKED
{
    // Pack 6 direction bits into one character
    // We only need 6 bits, so this fits comfortably in 0-63 range
    return current_level_code.enemy_directions & 0x3F; // Mask to 6 bits
}

// NEW: Encode enemy positions as a compact bitmask (which columns have enemies)
UBYTE encode_enemy_bitmask(void) BANKED
{
    // Strategy: Instead of storing exact positions, encode which of the 20 columns have enemies
    // Use a compact encoding that fits in 0-34 range (single character)

    UBYTE bitmask = 0;
    UBYTE enemy_count = 0;

    // Count enemies and create a simple hash/index based on positions
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
            // Create a simple hash by XORing position values
            bitmask ^= (current_level_code.enemy_positions[i] + 1);
        }
    }

    // Combine enemy count (0-6) with position hash
    // Format: [count:3bits][hash:3bits] = 6 bits total (0-63 range)
    UBYTE result = ((enemy_count & 0x07) << 3) | (bitmask & 0x07);

    // Ensure it's in valid display range (0-34)
    if (result > 34)
        result = result % 35;

    return result;
}

// ============================================================================
// UNIFIED LEVEL CODE DISPLAY SYSTEM
// ============================================================================
// All level code rendering now goes through display_complete_level_code()
// which is called by:
// 1. draw_segment_ids() - the main event function you trigger
// 2. All paint functions in paint.c when platforms/enemies/player are modified
//
// This eliminates the previous fragmented update system and ensures:
// - No flicker (single full update per change)
// - All data types (platforms, enemies, player) are always in sync
// - Single source of truth for all level code display
// ============================================================================

// MASTER LEVEL CODE DISPLAY FUNCTION - handles all level code rendering
void display_complete_level_code(void) BANKED
{
    // Extract all current level data
    update_complete_level_code();

    // Clear the display area once
    clear_level_code_display();

    // Format: Exactly 24 characters total
    // XXXX XXXX XXXX XXXX XXXX XXXX (6 groups of 4)
    // Starting at (5,6)

    UBYTE display_x = 5; // Start at column 5
    UBYTE display_y = 6; // Start at row 6
    UBYTE total_chars = 0;

    // Display all 20 platform patterns first
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        // Add space every 4 characters (between groups)
        if (total_chars > 0 && total_chars % 4 == 0)
        {
            display_x++; // Space between groups
        }

        // Wrap to next line if we exceed the screen width
        if (display_x >= 19) // Leave room for at least one more character
        {
            display_x = 5; // Reset to start column
            display_y++;   // Move to next row
        }

        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        display_x++;
        total_chars++;
    }

    // Add COMPACT enemy data (exactly 3 characters total)

    // Character 21: Enemy bitmask (which positions have enemies)
    if (total_chars > 0 && total_chars % 4 == 0)
        display_x++; // Space before new group
    if (display_x >= 19)
    {
        display_x = 5;
        display_y++;
    }
    display_char_at_position(get_extended_display_char(encode_enemy_bitmask()), display_x, display_y);
    display_x++;
    total_chars++;

    // Character 22: Enemy directions (packed direction bits for all enemies)
    if (total_chars > 0 && total_chars % 4 == 0)
        display_x++; // Space before new group
    if (display_x >= 19)
    {
        display_x = 5;
        display_y++;
    }
    display_char_at_position(get_extended_display_char(encode_enemy_directions()), display_x, display_y);
    display_x++;
    total_chars++;

    // Character 23: Player column
    if (total_chars > 0 && total_chars % 4 == 0)
        display_x++; // Space before new group
    if (display_x >= 19)
    {
        display_x = 5;
        display_y++;
    }
    display_char_at_position(get_extended_display_char(current_level_code.player_column), display_x, display_y);
    display_x++;
    total_chars++;

    // Character 24: Reserved/checksum
    if (total_chars > 0 && total_chars % 4 == 0)
        display_x++; // Space before new group
    if (display_x >= 19)
    {
        display_x = 5;
        display_y++;
    }
    display_char_at_position('0', display_x, display_y);
}

void draw_segment_ids(void) BANKED
{
    // Simply call the master function to display everything
    display_complete_level_code();
}

void update_zone_code(UBYTE zone_index) BANKED
{
    if (zone_index >= TOTAL_BLOCKS)
        return; // Safety check

    UBYTE segment_x = 2 + (zone_index % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + (zone_index / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

    // Extract pattern and get pattern ID
    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    // Update the stored code
    current_level_code.platform_patterns[zone_index] = (UBYTE)pattern_id;

    // Update just this debug tile
    display_code_tile(pattern_id, zone_index);
}

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    draw_segment_ids();
}

void display_code_tile(UWORD pattern_id, UBYTE i) BANKED
{
    // Safety check - only use IDs 0-20, but we have debug tiles 0-79 available
    if (pattern_id > 20)
    {
        pattern_id = 0;
    }

    // Calculate debug tile position: starting at (0,6), 16 tiles per row
    // Debug tiles go from (0,6) to (15,10) = 5 rows × 16 columns = 80 tiles (0-79)
    UBYTE debug_tile_col = pattern_id % 16;
    UBYTE debug_tile_row = 6 + (pattern_id / 16);

    UBYTE meta_tile_index = debug_tile_col + (debug_tile_row * 16);

    // Calculate debug display position: starting at (5,5), 13 tiles per row
    UBYTE debug_x = DEBUG_START_X + (i % DEBUG_TILES_PER_ROW);
    UBYTE debug_y = DEBUG_START_Y + (i / DEBUG_TILES_PER_ROW);

    replace_meta_tile(debug_x, debug_y, meta_tile_index, 1);
}

void vm_update_code(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    // Use the formatted debug display system
    display_debug_patterns_formatted();
}

void draw_debug_pattern(UBYTE pattern_index) BANKED
{
    if (pattern_index >= PLATFORM_PATTERN_COUNT)
    {
        pattern_index = 0; // Wrap around
    }

    // Display the pattern index as decimal digits in the debug area
    // Show pattern index at (0,2) and (1,2) as decimal digits
    UBYTE dec_tens = pattern_index / 10; // Tens digit (0-5)
    UBYTE dec_ones = pattern_index % 10; // Ones digit (0-9)

    // Display decimal digits using TILE_0 (0-9 are consecutive)
    replace_meta_tile(0, 2, TILE_0 + dec_tens, 1);
    replace_meta_tile(1, 2, TILE_0 + dec_ones, 1);

    // Display the pattern tile in the debug area
    display_code_tile(pattern_index, 0);

    // First clear the drawing area by replacing with empty tiles (0)
    for (UBYTE x = 2; x < 7; x++)
    {
        for (UBYTE y = 11; y < 13; y++)
        {
            replace_meta_tile(x, y, 0, 1); // Replace with empty tile and commit
        }
    }

    // Now get the pattern and replace with platform tiles where needed
    UWORD pattern = PLATFORM_PATTERNS[pattern_index];
    for (UBYTE i = 0; i < 5; i++)
    {
        // Top row (bits 9-5)
        if ((pattern >> (9 - i)) & 1)
        {
            UBYTE tile_type;
            if (i == 0)
            {
                tile_type = PLATFORM_TILE_1; // Left platform
            }
            else if (i == 4)
            {
                tile_type = PLATFORM_TILE_3; // Right platform
            }
            else
            {
                tile_type = PLATFORM_TILE_2; // Middle platform
            }
            replace_meta_tile(2 + i, 11, tile_type, 1);
        }

        // Bottom row (bits 4-0)
        if ((pattern >> (4 - i)) & 1)
        {
            UBYTE tile_type;
            if (i == 0)
            {
                tile_type = PLATFORM_TILE_1; // Left platform
            }
            else if (i == 4)
            {
                tile_type = PLATFORM_TILE_3; // Right platform
            }
            else
            {
                tile_type = PLATFORM_TILE_2; // Middle platform
            }
            replace_meta_tile(2 + i, 12, tile_type, 1);
        }
    }
}

void vm_debug_next_pattern(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    debug_pattern_index++;
    if (debug_pattern_index >= PLATFORM_PATTERN_COUNT)
    {
        debug_pattern_index = 0; // Wrap around
    }
    draw_debug_pattern(debug_pattern_index);
}

void vm_debug_prev_pattern(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    if (debug_pattern_index == 0)
    {
        debug_pattern_index = PLATFORM_PATTERN_COUNT - 1; // Wrap to last
    }
    else
    {
        debug_pattern_index--;
    }
    draw_debug_pattern(debug_pattern_index);
}

void vm_debug_reset_pattern(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    debug_pattern_index = 0;
    draw_debug_pattern(debug_pattern_index);
}

// Helper function to display a pattern character using the mapping table
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    UBYTE tile_index;

    if (value < PLATFORM_PATTERN_COUNT)
    {
        // Use the mapping table - you can edit PATTERN_TILE_MAP[] above to fix tile indices
        tile_index = PATTERN_TILE_MAP[value];
    }
    else
    {
        // Fallback to '0' for invalid pattern IDs
        tile_index = PATTERN_TILE_MAP[0];
    }

    replace_meta_tile(x, y, tile_index, 1);
}

// Legacy function for hex digit display (0-F only)
void display_hex_digit(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    // Clamp to 0-15 and use display_pattern_char
    if (value > 15)
        value = 0;
    display_pattern_char(value, x, y);
}

// Generate and display complete level code (platforms + player + enemies)
void generate_and_display_level_code(void) BANKED
{
    // Generate platform pattern codes (20 segments, each pattern ID 0-20)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0, row1;
        UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UWORD pattern_id = match_platform_pattern(pattern);

        // Use full range 0-20 (supports 0-9, A-K)
        if (pattern_id > 20)
            pattern_id = 0;

        current_level_code.platform_patterns[i] = (UBYTE)pattern_id;
    }

    // Clear the display area first
    clear_level_code_display();

    // Display in 4-character groups at position (5,6)
    UBYTE display_x = 5; // Start at column 5
    UBYTE display_y = 6; // Start at row 6
    UBYTE char_count = 0;
    UBYTE chars_in_current_group = 0;
    UBYTE groups_on_line = 0;

    // Display all 20 platform patterns using 4-character groups
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        // Check if we need to wrap to next line (at column 18)
        if (display_x >= 18)
        {
            display_x = 5; // Reset to start column
            display_y++;   // Move to next row
            groups_on_line = 0;
            chars_in_current_group = 0;
        }

        // Add space between groups (but not at start of line)
        if (chars_in_current_group == 0 && groups_on_line > 0)
        {
            display_x++; // Add space between groups
        }

        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        display_x++;
        chars_in_current_group++;

        // Complete a group of 4 characters
        if (chars_in_current_group >= 4)
        {
            chars_in_current_group = 0;
            groups_on_line++;
        }
    }
}

// Display debug patterns in formatted layout (5-char blocks with spaces)
void display_debug_patterns_formatted(void) BANKED
{
    // Clear the debug display area first
    clear_debug_display();

    // Generate pattern codes for all 20 segments (5x4 grid)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0, row1;
        UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UWORD pattern_id = match_platform_pattern(pattern);

        // Use full range 0-20 (supports 0-9, A-K)
        if (pattern_id > 20)
            pattern_id = 0;

        current_level_code.platform_patterns[i] = (UBYTE)pattern_id;
    }

    // Display in formatted layout at debug position (5,5)
    // Format: 5 digits + space + 5 digits + space + 5 digits + space + 5 digits = 23 chars per row
    // Row 1: 00000 00000 00000 00000 (20 digits)

    UBYTE display_x = DEBUG_START_X;
    UBYTE display_y = DEBUG_START_Y;
    UBYTE char_count = 0;

    // Display all 20 platform patterns using extended character set
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        // Add space before each 5-char block (except the first)
        if (char_count > 0 && char_count % 5 == 0)
        {
            display_x++; // Skip one position for space
        }

        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        display_x += 1; // Move 1 position for single character
        char_count += 1;

        // Move to next row after 20 characters (full 5x4 grid)
        if (char_count >= 20)
        {
            display_x = DEBUG_START_X;
            display_y++;
            char_count = 0;
        }
    }
}

// Helper function to clear the level code display area
void clear_level_code_display(void) BANKED
{
    // Clear 2 rows starting at (5,6) for level code display
    for (UBYTE y = 6; y < 8; y++) // Rows 6 and 7
    {
        for (UBYTE x = 5; x < 20; x++) // Columns 5-19 (wide enough for the display)
        {
            replace_meta_tile(x, y, 0, 1); // Replace with empty tile
        }
    }
}

// Helper function to clear the debug display area
void clear_debug_display(void) BANKED
{
    // Clear 2 rows starting at debug position (5,5)
    for (UBYTE y = DEBUG_START_Y; y < DEBUG_START_Y + 2; y++)
    {
        for (UBYTE x = DEBUG_START_X; x < DEBUG_START_X + 15; x++) // 15 chars wide should be enough
        {
            replace_meta_tile(x, y, 0, 1); // Replace with empty tile
        }
    }
}

// Test function to display a sample level code pattern
void test_level_code_display(void) BANKED
{
    // Clear the display area first
    clear_level_code_display();

    // Display test pattern using extended character set: 0123 456G ABCD
    UBYTE test_values[] = {0, 1, 2, 3, 4, 5, 6, 16, 10, 11, 12, 13, 14, 15, 7, 8, 9, 17, 18, 19}; // 20 values for 5x4 grid
    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;
    UBYTE char_count = 0;

    // Display 20 test characters
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        // Add space before each 5-char block (except the first)
        if (char_count > 0 && char_count % 5 == 0)
        {
            display_x++; // Skip one position for space
        }

        display_pattern_char(test_values[i], display_x, display_y);
        display_x += 1;
        char_count += 1;

        // Move to next row after 20 characters
        if (char_count >= 20)
        {
            display_x = LEVEL_CODE_START_X;
            display_y++;
            char_count = 0;
        }
    }
}

// Debug function to test hex tile display
void test_hex_tiles(void) BANKED
{
    // Display hex digits 0-F at position (5,0) to test tile mapping
    for (UBYTE i = 0; i < 16; i++)
    {
        display_hex_digit(i, 5 + i, 0);
    }
}

// Test function to demonstrate enemy encoding
void test_enemy_encoding(void) BANKED
{
    // Initialize level code
    init_level_code();

    // Set up test data: 3 enemies at columns 5, 12, 18
    current_level_code.enemy_positions[0] = 5;   // Enemy at column 5, right-facing
    current_level_code.enemy_positions[1] = 12;  // Enemy at column 12, left-facing
    current_level_code.enemy_positions[2] = 18;  // Enemy at column 18, right-facing
    current_level_code.enemy_positions[3] = 255; // No enemy
    current_level_code.enemy_positions[4] = 255; // No enemy
    current_level_code.enemy_positions[5] = 255; // No enemy

    // Set directions: enemy 1 = left (bit 1 set)
    current_level_code.enemy_directions = 0b000010; // Only enemy 1 facing left

    // Set player at column 3
    current_level_code.player_column = 3;

    // Display the encoded result
    display_complete_level_code();
}

// Debug function to display current enemy data for troubleshooting
void debug_display_enemy_data(void) BANKED
{
    // Display at position (5,3) to avoid conflicts with level code display at (5,6)
    UBYTE debug_x = 5;
    UBYTE debug_y = 3;

    // Clear debug area first
    for (UBYTE x = 5; x < 20; x++)
    {
        replace_meta_tile(x, debug_y, 0, 1); // Clear with empty tile
    }

    // Display "E:" prefix to indicate enemy data
    display_char_at_position('E', debug_x, debug_y);
    debug_x++;
    display_char_at_position(':', debug_x, debug_y);
    debug_x++;

    // Display compact enemy encoding (bitmask + directions)
    display_char_at_position(get_extended_display_char(encode_enemy_bitmask()), debug_x, debug_y);
    debug_x++;
    display_char_at_position(get_extended_display_char(encode_enemy_directions()), debug_x, debug_y);
    debug_x++;

    // Show enemy count for reference
    UBYTE enemy_count = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
        }
    }
    display_char_at_position(get_extended_display_char(enemy_count), debug_x, debug_y);
}

void vm_debug_display_enemy_data(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;

    // Extract current enemy data and display it
    extract_enemy_data();
    debug_display_enemy_data();
}

// VM wrapper functions
void vm_generate_and_display_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    generate_and_display_level_code();
}

void vm_init_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    init_level_code();
}

void vm_test_level_code_display(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    test_level_code_display();
}

void vm_test_hex_tiles(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    test_hex_tiles();
}

void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    test_enemy_encoding();
}

void vm_display_debug_patterns_formatted(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;
    display_debug_patterns_formatted();
}

// Helper function to calculate zone index from tile coordinates
UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED
{
    // Check if coordinates are within the platform area
    if (x < 2 || x >= 22 || y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
        return 255; // Invalid zone

    // Convert tile coordinates to segment coordinates
    // Segments are 5 tiles wide, starting at x=2
    UBYTE segment_x = (x - 2) / SEGMENT_WIDTH;
    UBYTE segment_y = (y - PLATFORM_Y_MIN) / SEGMENT_HEIGHT;

    // Ensure we're within the 5x4 grid
    if (segment_x >= SEGMENTS_PER_ROW || segment_y >= 4)
        return 255; // Invalid zone

    return segment_y * SEGMENTS_PER_ROW + segment_x;
}
