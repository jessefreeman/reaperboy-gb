#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"
#include "paint.h"

#define PLATFORM_Y_MIN 12 // Changed from 11 to 12 to start at y=12
#define TILE_0 48
#define SEGMENTS_PER_ROW 4 // 4 blocks horizontally
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

// Debug display settings
#define DEBUG_START_X 5
#define DEBUG_START_Y 5
#define DEBUG_TILES_PER_ROW 4 // Match the 4x4 chunk grid layout

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

UWORD current_code[16] = { // 4x4 grid = 16 blocks total
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0};

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

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED
{
    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(chunk_x, chunk_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    current_code[chunk_index] = pattern_id; // Store unique pattern ID (0-20)
}

void display_code_tile(UWORD pattern_id, UBYTE i) BANKED
{ // pattern_id is now a unique ID from 0-20
    // Debug tiles are numbered 0-79 starting at (0,6) ending at (15,10)

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

void draw_segment_ids(void) BANKED
{
    // Display debug patterns in formatted layout (4-char blocks with spaces)
    display_debug_patterns_formatted();

    // Also display the level code at a different location (optional)
    // generate_and_display_level_code();
}

void update_zone_code(UBYTE zone_index) BANKED
{
    if (zone_index >= 16)
        return; // Safety check

    UBYTE segment_x = 2 + (zone_index % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + (zone_index / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

    // Extract pattern and get pattern ID
    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    // Update the stored code
    current_code[zone_index] = pattern_id;

    // Update just this debug tile
    display_code_tile(pattern_id, zone_index);
}

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    draw_segment_ids();
}

// Debug function to draw patterns sequentially
UBYTE debug_pattern_index = 0;

void draw_debug_pattern(UBYTE pattern_index) BANKED
{
    if (pattern_index >= PLATFORM_PATTERN_COUNT)
    {
        pattern_index = 0; // Wrap around
    } // Display the pattern index as decimal digits in the debug area
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
    { // Top row (bits 9-5)
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
    // Generate platform pattern codes (16 segments, each pattern ID 0-20)
    UBYTE platform_patterns[16];
    for (UBYTE i = 0; i < 16; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0, row1;
        UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UWORD pattern_id = match_platform_pattern(pattern);

        // Use full range 0-20 (supports 0-9, A-F, G-T)
        if (pattern_id > 20)
            pattern_id = 0;

        platform_patterns[i] = (UBYTE)pattern_id;
    }

    // Clear the display area first
    clear_level_code_display();

    // Format: 4 digits per block, 3 blocks per row = 12 digits per row
    // Row 1: 0000 0000 0000 (12 digits)
    // Row 2: 0000 (4 remaining digits)

    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;
    UBYTE char_count = 0;

    // Display all 16 platform patterns using extended character set
    for (UBYTE i = 0; i < 16; i++)
    {
        // Add space before each 4-char block (except the first)
        if (char_count > 0 && char_count % LEVEL_CODE_BLOCK_SIZE == 0)
        {
            display_x++; // Skip one position for space
        }

        display_pattern_char(platform_patterns[i], display_x, display_y);
        display_x += 1; // Move 1 position for single character
        char_count += 1;

        // Move to next row after 12 characters
        if (char_count >= LEVEL_CODE_CHARS_PER_ROW)
        {
            display_x = LEVEL_CODE_START_X;
            display_y++;
            char_count = 0;
        }
    }
}

// VM wrapper for generating and displaying complete level code
void vm_generate_and_display_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    generate_and_display_level_code();
}

// Helper function to clear the level code display area
void clear_level_code_display(void) BANKED
{
    // Clear 2 rows starting at (5,1) for level code display - separate from debug tiles
    for (UBYTE y = LEVEL_CODE_START_Y; y < LEVEL_CODE_START_Y + 2; y++)
    {
        for (UBYTE x = LEVEL_CODE_START_X; x < LEVEL_CODE_START_X + 15; x++) // 15 chars wide should be enough
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
    UBYTE test_values[] = {0, 1, 2, 3, 4, 5, 6, 16, 10, 11, 12, 13, 14, 15, 7, 8}; // G=16
    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;
    UBYTE char_count = 0;

    // Display 16 test characters
    for (UBYTE i = 0; i < 16; i++)
    {
        // Add space before each 4-char block (except the first)
        if (char_count > 0 && char_count % LEVEL_CODE_BLOCK_SIZE == 0)
        {
            display_x++; // Skip one position for space
        }

        display_pattern_char(test_values[i], display_x, display_y);
        display_x += 1;
        char_count += 1;

        // Move to next row after 12 characters
        if (char_count >= LEVEL_CODE_CHARS_PER_ROW)
        {
            display_x = LEVEL_CODE_START_X;
            display_y++;
            char_count = 0;
        }
    }
}

// VM wrapper for test function
void vm_test_level_code_display(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    test_level_code_display();
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

// VM wrapper for hex tile test
void vm_test_hex_tiles(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    test_hex_tiles();
}

// Display debug patterns in formatted layout (4-char blocks with spaces)
void display_debug_patterns_formatted(void) BANKED
{
    // Clear the debug display area first
    clear_debug_display();

    // Generate pattern codes for all 16 segments
    UBYTE platform_patterns[16];
    for (UBYTE i = 0; i < 16; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0, row1;
        UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UWORD pattern_id = match_platform_pattern(pattern);

        // Use full range 0-20 (supports 0-9, A-F, G-T)
        if (pattern_id > 20)
            pattern_id = 0;

        platform_patterns[i] = (UBYTE)pattern_id;
        current_code[i] = pattern_id; // Update stored code
    }

    // Display in formatted layout at debug position (5,5)
    // Format: 4 digits + space + 4 digits + space + 4 digits = 12 chars per row
    // Row 1: 0000 0000 0000 (12 digits)
    // Row 2: 0000 (4 remaining digits)

    UBYTE display_x = DEBUG_START_X;
    UBYTE display_y = DEBUG_START_Y;
    UBYTE char_count = 0;

    // Display all 16 platform patterns using extended character set
    for (UBYTE i = 0; i < 16; i++)
    {
        // Add space before each 4-char block (except the first)
        if (char_count > 0 && char_count % LEVEL_CODE_BLOCK_SIZE == 0)
        {
            display_x++; // Skip one position for space
        }

        display_pattern_char(platform_patterns[i], display_x, display_y);
        display_x += 1; // Move 1 position for single character
        char_count += 1;

        // Move to next row after 12 characters
        if (char_count >= LEVEL_CODE_CHARS_PER_ROW)
        {
            display_x = DEBUG_START_X;
            display_y++;
            char_count = 0;
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

// VM wrapper for formatted debug display
void vm_display_debug_patterns_formatted(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    display_debug_patterns_formatted();
}
