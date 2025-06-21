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

// 13 base patterns (0-12) with 4 variations each = 52 total patterns (0-51)
const UBYTE PLATFORM_PATTERNS[13][4] = {
    {0b00000, 0b00001, 0b10000, 0b10001}, // Pattern 0: variations 0-3
    {0b00011, 0b00110, 0b11000, 0b11100}, // Pattern 1: variations 4-7
    {0b00111, 0b01110, 0b11110, 0b01111}, // Pattern 2: variations 8-11
    {0b01111, 0b11111, 0b11110, 0b11111}, // Pattern 3: variations 12-15
    {0b01100, 0b00110, 0b01100, 0b00110}, // Pattern 4: variations 16-19
    {0b01010, 0b10101, 0b01010, 0b10101}, // Pattern 5: variations 20-23
    {0b01001, 0b10010, 0b01001, 0b10010}, // Pattern 6: variations 24-27
    {0b10001, 0b01000, 0b00010, 0b10001}, // Pattern 7: variations 28-31
    {0b11001, 0b10011, 0b11001, 0b10011}, // Pattern 8: variations 32-35
    {0b10110, 0b01101, 0b10110, 0b01101}, // Pattern 9: variations 36-39
    {0b11011, 0b10111, 0b11101, 0b01111}, // Pattern 10: variations 40-43
    {0b11101, 0b01111, 0b11110, 0b11111}, // Pattern 11: variations 44-47
    {0b11111, 0b01111, 0b11110, 0b11101}  // Pattern 12: variations 48-51
};

#define PLATFORM_PATTERN_COUNT (sizeof(PLATFORM_PATTERNS) / sizeof(PLATFORM_PATTERNS[0]))

UBYTE current_code[20] = { // 4x5 grid = 20 blocks total
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0};

UBYTE extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED
{
    *row0 = 0;
    *row1 = 0;
    for (UBYTE i = 0; i < 5; i++) // Changed from 6 to 5
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y)]) == 1)
            *row0 |= (1 << (4 - i)); // Changed from (5 - i) to (4 - i)
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y + 1)]) == 1)
            *row1 |= (1 << (4 - i)); // Changed from (5 - i) to (4 - i)
    }
    return 1;
}

UBYTE match_platform_pattern(UBYTE row0) BANKED
{
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        for (UBYTE variant = 0; variant < 4; variant++) // Check all 4 variations
        {
            if (PLATFORM_PATTERNS[i][variant] == row0)
                return (i * 4) + variant; // Return pattern index (0-51)
        }
    }
    return 0; // fallback to pattern 0
}

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED
{
    UBYTE row0, row1;
    extract_chunk_pattern(chunk_x, chunk_y, &row0, &row1);
    UBYTE pattern_index = match_platform_pattern(row0);

    current_code[chunk_index] = pattern_index; // Store pattern index (0-51)
}

void display_code_tile(UBYTE pattern_index, UBYTE i) BANKED
{
    // pattern_index is now 0-51 directly
    if (pattern_index > 51) // Ensure we don't exceed our pattern range
        pattern_index = 0;

    UBYTE cursor_x = 4 + (i % 15);
    UBYTE cursor_y = 6 + (i / 15);
    replace_meta_tile(cursor_x, cursor_y, TILE_0 + pattern_index, 1);
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

void draw_segment_ids() BANKED
{
    // 4 blocks horizontally × 5 blocks vertically = 20 total blocks
    for (UBYTE i = 0; i < 20; i++)
    {
        UBYTE segment_x = 2 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;               // Start at x=2, move by 5s
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT; // Start at y=11, move by 2s

        UBYTE row0 = 0;
        UBYTE dummy = 0;
        extract_chunk_pattern(segment_x, segment_y, &row0, &dummy);
        UBYTE pattern_index = match_platform_pattern(row0);

        // pattern_index is now 0-51 directly
        if (pattern_index > 51)
            pattern_index = 0;

        // Place debug marker at bottom of each 5x2 block
        replace_meta_tile(segment_x, segment_y + 1, TILE_HEX_DEBUG + pattern_index, 1);
    }
}

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
{
    // Suppress unused parameter warning
    (void)THIS;

    draw_segment_ids();
}
