#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"

#define PLATFORM_Y_MIN 11
#define TILE_0 48
#define SEGMENTS_PER_ROW 4 // Changed from 3 to 4 (4 blocks horizontally)
#define SEGMENT_WIDTH 5    // Changed from 6 to 5
#define SEGMENT_HEIGHT 2
#define TILE_HEX_DEBUG 96
#define TILE_FLIP_OFFSET 32 // each flip variant lives 2 rows (16 tiles) down

// Platform tile IDs
#define PLATFORM_TILE_1 4
#define PLATFORM_TILE_2 5
#define PLATFORM_TILE_3 6

// Debug display settings
#define DEBUG_START_X 5
#define DEBUG_START_Y 5
#define DEBUG_TILES_PER_ROW 13 // (18 - 5 = 13 tiles per row)

// 18 base patterns with unique IDs for each variation = 57 total unique patterns (0-56)
const UWORD PLATFORM_PATTERNS[] = {
    0b0000000000, // Index 0
    0b0000100000, // Index 1
    0b1000000000, // Index 2
    0b0000000001, // Index 3
    0b0000010000, // Index 4
    0b0001100000, // Index 5
    0b1100000000, // Index 6
    0b0000000011, // Index 7
    0b0000011000, // Index 8
    0b0011000000, // Index 9
    0b0110000000, // Index 10
    0b0000000110, // Index 11
    0b0000001100, // Index 12
    0b0011100000, // Index 13
    0b1110000000, // Index 14
    0b0000000111, // Index 15
    0b0000011100, // Index 16
    0b0110100000, // Index 17
    0b1011000000, // Index 18
    0b0000001101, // Index 19
    0b0000010110, // Index 20
    0b0111000000, // Index 21
    0b0000001110, // Index 22
    0b0111100000, // Index 23
    0b1111000000, // Index 24
    0b0000001111, // Index 25
    0b0000011110, // Index 26
    0b1000100000, // Index 27
    0b0000010001, // Index 28
    0b1001100000, // Index 29
    0b1100100000, // Index 30
    0b0000010011, // Index 31
    0b0000011001, // Index 32
    0b1011100000, // Index 33
    0b1110100000, // Index 34
    0b0000010111, // Index 35
    0b0000011101, // Index 36
    0b1101100000, // Index 37
    0b0000011011, // Index 38
    0b1111100000, // Index 39
    0b0000011111, // Index 40
    0b0000110000, // Index 41
    0b1000000001, // Index 42
    0b0001110000, // Index 43
    0b1100000001, // Index 44 
    0b1000000011, // Index 45
    0b0000111000, // Index 46
    0b0011110000, // Index 47
    0b1110000001, // Index 48
    0b1000000111, // Index 49
    0b0000111100, // Index 50
    0b0011010000, // Index 51
    0b0110000001, // Index 52
    0b1000000110, // Index 53
    0b0000101100, // Index 54
    0b0001111000, // Index 55
    0b1100000011  // Index 56
};

#define PLATFORM_PATTERN_COUNT (sizeof(PLATFORM_PATTERNS) / sizeof(PLATFORM_PATTERNS[0]))

UWORD current_code[20] = { // 4x5 grid = 20 blocks total
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0};

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
            return i; // Return the unique pattern ID (0-56)
        }
    }
    return 0; // fallback to pattern 0
}

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED
{
    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(chunk_x, chunk_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    current_code[chunk_index] = pattern_id; // Store unique pattern ID (0-56)
}

void display_code_tile(UWORD pattern_id, UBYTE i) BANKED
{ // pattern_id is now a unique ID from 0-56
    // Debug tiles are numbered 0-79 starting at (0,6) ending at (15,10)

    // Safety check - only use IDs 0-56, but we have debug tiles 0-79 available
    if (pattern_id > 56)
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

    for (UBYTE i = 0; i < 20; i++) // 4x5 grid = 20 blocks
    {
        UBYTE chunk_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;               // Start at x=2, move by 5s
        UBYTE chunk_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT; // Start at y=11, move by 2s
        update_code_at_chunk(chunk_x, chunk_y, i);
        display_code_tile(current_code[i], i);
    }
}

void draw_segment_ids(void) BANKED
{
    // Update all 20 zones and display their debug codes at (5,5) area
    for (UBYTE i = 0; i < 20; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;               // Start at x=2, move by 5s
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT; // Start at y=11, move by 2s

        // Extract pattern and get pattern ID
        UBYTE row0, row1;
        UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UWORD pattern_id = match_platform_pattern(pattern);

        // Update the stored code
        current_code[i] = pattern_id;

        // Display debug tile in the debug area
        display_code_tile(pattern_id, i);
    }
}

void update_zone_code(UBYTE zone_index) BANKED
{
    if (zone_index >= 20)
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
    {
        // Top row (bits 9-5)
        if ((pattern >> (9 - i)) & 1)
        {
            uint8_t tile_type;
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
            uint8_t tile_type;
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
