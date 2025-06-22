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

// 17 base patterns with unique IDs for each variation = 55 total unique patterns (0-54)
const UWORD PLATFORM_PATTERNS[] = {
    // Pattern 0: ID 0 (all variants the same)
    0b0000000000, // 00, 01, 02, 03 → ID 0
    
    // Pattern 1: IDs 1-4
    0b0000100000, // 10 → ID 1
    0b1000000000, // 11 → ID 2
    0b0000000001, // 12 → ID 3
    0b0000010000, // 13 → ID 4
    
    // Pattern 2: IDs 5-8
    0b0001100000, // 20 → ID 5
    0b1100000000, // 21 → ID 6
    0b0000000011, // 22 → ID 7
    0b0000011000, // 23 → ID 8
    
    // Pattern 3: IDs 9-12
    0b0011000000, // 30 → ID 9
    0b0110000000, // 31 → ID 10
    0b0000000110, // 32 → ID 11
    0b0000001100, // 33 → ID 12
    
    // Pattern 4: IDs 13-16
    0b0011100000, // 40 → ID 13
    0b1110000000, // 41 → ID 14
    0b0000000111, // 42 → ID 15
    0b0000011100, // 43 → ID 16
    
    // Pattern 5: IDs 17-20
    0b0110100000, // 50 → ID 17
    0b1011000000, // 51 → ID 18
    0b0000001101, // 52 → ID 19
    0b0000010110, // 53 → ID 20
    
    // Pattern 6: IDs 21-22 (variants 0&1 same, 2&3 same)
    0b0111000000, // 60, 61 → ID 21
    0b0000001110, // 62, 63 → ID 22
    
    // Pattern 7: IDs 23-26
    0b0111100000, // 70 → ID 23
    0b1111000000, // 71 → ID 24
    0b0000001111, // 72 → ID 25
    0b0000011110, // 73 → ID 26
    
    // Pattern 8: IDs 27-28 (variants 0&1 same, 2&3 same)
    0b1000100000, // 80, 81 → ID 27
    0b0000010001, // 82, 83 → ID 28
    
    // Pattern 9: IDs 29-32
    0b1001100000, // 90 → ID 29
    0b1100100000, // 91 → ID 30
    0b0000010011, // 92 → ID 31
    0b0000011001, // 93 → ID 32
    
    // Pattern 10: IDs 33-36
    0b1011100000, // A0 → ID 33
    0b1110100000, // A1 → ID 34
    0b0000010111, // A2 → ID 35
    0b0000011101, // A3 → ID 36
    
    // Pattern 11: IDs 37-38 (variants 0&1 same, 2&3 same)
    0b1101100000, // B0, B1 → ID 37
    0b0000011011, // B2, B3 → ID 38
    
    // Pattern 12: IDs 39-40 (variants 0&1 same, 2&3 same)
    0b1111100000, // C0, C1 → ID 39
    0b0000011111, // C2, C3 → ID 40
    
    // Pattern 13: IDs 41-42 (variants 0&3 same, 1&2 same)
    0b0000110000, // D0, D3 → ID 41
    0b1000000001, // D1, D2 → ID 42
    
    // Pattern 14: IDs 43-46
    0b0001110000, // E0 → ID 43
    0b1100000001, // E1 → ID 44
    0b1000000011, // E2 → ID 45
    0b0000111000, // E3 → ID 46    // Pattern 15: IDs 47-50
    0b0001111000, // F0 → ID 47
    0b1100000011, // F1 → ID 48
    0b1100000011, // F2 → ID 49 (same as F1)
    0b0001111000, // F3 → ID 50 (same as F0)
    
    // Pattern 16: IDs 51-54
    0b0011010000, // G0 → ID 51
    0b0110000001, // G1 → ID 52
    0b1000000110, // G2 → ID 53
    0b0000101100  // G3 → ID 54
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
            return i; // Return the unique pattern ID (0-54)
        }
    }
    return 0; // fallback to pattern 0
}

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED
{
    UBYTE row0, row1;
    UWORD pattern = extract_chunk_pattern(chunk_x, chunk_y, &row0, &row1);
    UWORD pattern_id = match_platform_pattern(pattern);

    current_code[chunk_index] = pattern_id; // Store unique pattern ID (0-54)
}

void display_code_tile(UWORD pattern_id, UBYTE i) BANKED
{
    // pattern_id is now a unique ID from 0-54
    // Debug tiles are numbered 0-79 starting at (0,6) ending at (15,10)
    
    // Safety check - only use IDs 0-54, but we have debug tiles 0-79 available
    if (pattern_id > 54) {
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

void draw_segment_ids() BANKED
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
