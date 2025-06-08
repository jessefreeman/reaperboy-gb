#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"

#define PLATFORM_Y_MIN 11
#define TILE_0 48
#define SEGMENTS_PER_ROW 3
#define SEGMENT_WIDTH    6
#define SEGMENT_HEIGHT   2

const UBYTE PLATFORM_PATTERNS[16][4] = {
    {0b000000, 0b000000, 0b000000, 0b000000}, // 0
    {0b000001, 0b100000, 0b000100, 0b100000}, // 1
    {0b001100, 0b001100, 0b000011, 0b110000}, // 2
    {0b001101, 0b101100, 0b001011, 0b110100}, // 3
    {0b001111, 0b111100, 0b001111, 0b111100}, // 4
    {0b011000, 0b000110, 0b000001, 0b100000}, // 5
    {0b011100, 0b001110, 0b000111, 0b111000}, // 6
    {0b011101, 0b101110, 0b001011, 0b110100}, // 7
    {0b011110, 0b011110, 0b011110, 0b011110}, // 8
    {0b100001, 0b100001, 0b010010, 0b010010}, // 9
    {0b110011, 0b110011, 0b001100, 0b001100}, // A
    {0b110110, 0b011011, 0b110110, 0b011011}, // B
    {0b111011, 0b110111, 0b110111, 0b111011}, // C
    {0b111100, 0b001111, 0b111100, 0b001111}, // D
    {0b111101, 0b101111, 0b111101, 0b101111}, // E
    {0b111111, 0b111111, 0b111111, 0b111111}  // F
};

UBYTE current_code[18] = {
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

UBYTE extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED {
    *row0 = 0;
    *row1 = 0;
    for (UBYTE i = 0; i < 6; i++) {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y)]) == 1)
            *row0 |= (1 << (5 - i));
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + i, y + 1)]) == 1)
            *row1 |= (1 << (5 - i));
    }
    return 1;
}

UBYTE match_platform_pattern(UBYTE row0, UBYTE row1) BANKED {
    for (UBYTE i = 0; i < 16; i++) {
        for (UBYTE v = 0; v < 4; v++) {
            if (PLATFORM_PATTERNS[i][v] == row0 &&
                PLATFORM_PATTERNS[i][v ^ 1] == row1) {
                return i;
            }
        }
    }
    for (UBYTE i = 0; i < 16; i++) {
        for (UBYTE v = 0; v < 4; v++) {
            if (PLATFORM_PATTERNS[i][v] == row0) return i;
        }
    }
    return 0xFF;
}

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_index) BANKED {
    UBYTE row0, row1;
    extract_chunk_pattern(chunk_x, PLATFORM_Y_MIN, &row0, &row1);
    UBYTE code_index = match_platform_pattern(row0, row1);
    current_code[chunk_index] = (code_index != 0xFF) ? code_index : 0;
}

void display_code_tile(UBYTE code_index, UBYTE i) BANKED {
    if (code_index > 15) return;
    UBYTE cursor_x = 4 + (i % 15);
    UBYTE cursor_y = 6 + (i / 15);
    replace_meta_tile(cursor_x, cursor_y, TILE_0 + code_index, 1);
}

void vm_update_code(SCRIPT_CTX *THIS) BANKED {
    for (UBYTE i = 0; i < 18; i++) {
        UBYTE chunk_x = 1 + i * 6;
        update_code_at_chunk(chunk_x, i);
        display_code_tile(current_code[i], i);
    }
}

void draw_segment_ids() BANKED {
    for (UBYTE i = 0; i < 24; i++) {
        UBYTE segment_x = 3 + (i % SEGMENTS_PER_ROW) * SEGMENT_WIDTH;
        UBYTE segment_y = PLATFORM_Y_MIN + (i / SEGMENTS_PER_ROW) * SEGMENT_HEIGHT;

        UBYTE row0 = 0, row1 = 0;
        extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
        UBYTE code_index = match_platform_pattern(row0, row1);
        replace_meta_tile(segment_x, segment_y + 1, TILE_0 + ((code_index != 0xFF) ? code_index : 0), 1);
    }
}

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED {
    draw_segment_ids();
}
