#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"

#define PLATFORM_Y_MIN 11
#define TILE_0 48
#define SEGMENTS_PER_ROW 3
#define SEGMENT_WIDTH 6
#define SEGMENT_HEIGHT 2
#define TILE_HEX_DEBUG 96
#define TILE_FLIP_OFFSET 32 // each flip variant lives 2 rows (16 tiles) down

const UBYTE PLATFORM_PATTERNS[25][2] = {
    {0b000000, 0b000000}, // Fallback: all empty
    {0b000001, 0b100000}, // 00
    {0b000011, 0b110000}, // 10
    {0b000110, 0b011000}, // 20
    {0b001100, 0b001100}, // 30 (symmetrical)
    {0b000111, 0b111000}, // 40
    {0b001111, 0b111100}, // 50
    {0b001110, 0b011100}, // 60
    {0b010110, 0b011010}, // 70
    {0b011000, 0b000110}, // 80
    {0b011100, 0b001110}, // 90
    {0b011110, 0b011110}, // A0 (symmetrical)
    {0b100001, 0b100001}, // B0 (symmetrical)
    {0b110011, 0b110011}, // C0 (symmetrical)
    {0b110110, 0b011011}, // D0
    {0b111000, 0b000111}, // E0
    {0b111100, 0b001111}, // F0
    {0b111110, 0b011111}, // H0
    {0b111111, 0b111111}, // I0 (symmetrical)
    {0b111011, 0b110111},
    {0b110001, 0b100011}, // new
    {0b111001, 0b100111}, // new
    {0b111101, 0b101111}, // new
    {0b101110, 0b011101}  // new
};

#define PLATFORM_PATTERN_COUNT (sizeof(PLATFORM_PATTERNS) / sizeof(PLATFORM_PATTERNS[0]))

UBYTE current_code[18] = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0};

UBYTE extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED
{
    *row0 = 0;
    *row1 = 0;
    for (UBYTE i = 0; i < 6; i++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y)]) == 1)
            *row0 |= (1 << (5 - i));
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y + 1)]) == 1)
            *row1 |= (1 << (5 - i));
    }
    return 1;
}

UBYTE match_platform_pattern(UBYTE row0) BANKED
{
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        if (PLATFORM_PATTERNS[i][0] == row0)
            return (i << 4) | 0; // variant 0
        if (PLATFORM_PATTERNS[i][1] == row0)
            return (i << 4) | 1; // variant 1 (flipped)
    }
    return 0x00; // fallback
}

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_index) BANKED
{
    UBYTE row0, row1;
    extract_chunk_pattern(chunk_x, PLATFORM_Y_MIN, &row0, &row1);
    UBYTE code_index = match_platform_pattern(row0);

    current_code[chunk_index] = (code_index != 0xFF) ? code_index : 0;
}

void display_code_tile(UBYTE code_index, UBYTE i) BANKED
{
    UBYTE pattern_id = code_index >> 4;
    UBYTE variant = code_index & 0x0F;
    if (variant > 1)
        variant = 0;

    if (pattern_id >= PLATFORM_PATTERN_COUNT)
        return;

    UBYTE cursor_x = 4 + (i % 15);
    UBYTE cursor_y = 6 + (i / 15);
    replace_meta_tile(cursor_x, cursor_y, TILE_0 + pattern_id + (variant * TILE_FLIP_OFFSET), 1);
}

void vm_update_code(SCRIPT_CTX *THIS) BANKED
{
    for (UBYTE i = 0; i < 18; i++)
    {
        UBYTE chunk_x = 1 + i * 6;
        update_code_at_chunk(chunk_x, i);
        display_code_tile(current_code[i], i);
    }
}

void draw_segment_ids() BANKED
{
    for (UBYTE i = 0; i < 24; i++)
    {
        UBYTE segment_x = 3 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0 = 0;
        UBYTE dummy = 0;
        extract_chunk_pattern(segment_x, segment_y, &row0, &dummy);
        UBYTE code_index = match_platform_pattern(row0);

        UBYTE pattern_id = code_index >> 4;
        UBYTE variant = code_index & 0x0F;
        if (variant > 1)
            variant = 0;

        replace_meta_tile(segment_x, segment_y + 1, TILE_HEX_DEBUG + pattern_id + (variant * TILE_FLIP_OFFSET), 1);
    }
}

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
{
    draw_segment_ids();
}
