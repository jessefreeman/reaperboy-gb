#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "paint.h"

#define PLATFORM_X_MIN 2
#define PLATFORM_X_MAX 21
#define PLATFORM_Y_MIN 11
#define PLATFORM_Y_MAX 20
#define PLATFORM_MIN_VERTICAL_GAP 1
#define PLATFORM_MAX_LENGTH 8
#define TILE_0 48  // Or whatever metatile ID represents '0'
#define DEBUG_DRAW_QUADRANTS 1

void draw_quadrant_ids() BANKED;

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

UBYTE get_tile_type(UBYTE tile_id) BANKED {
    switch(tile_id) {
        case TILE_PLATFORM_LEFT:
        case TILE_PLATFORM_MIDDLE:
        case TILE_PLATFORM_RIGHT:
            return 1; // Platform
        case TILE_ENEMY:
            return 2; // Enemy
        case TILE_EXIT_TOP_LEFT:
        case TILE_EXIT_TOP_RIGHT:
        case TILE_EXIT_BOTTOM_LEFT:
        case TILE_EXIT_BOTTOM_RIGHT:
            return 3; // Exit
        case TILE_PLAYER:
            return 4; // Player
        default:
            return 5; // Empty
    }
}

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

    // Fallback: try matching just row0
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

    UBYTE cursor_x = 4 + (chunk_index % 15);
    UBYTE cursor_y = 6 + (chunk_index / 15);

    replace_meta_tile(cursor_x, cursor_y, TILE_0 + current_code[chunk_index], 1);
}

void display_code_tile(UBYTE code_index, UBYTE i) BANKED {
    if (code_index > 15) return;

    UBYTE cursor_x = 4 + (i % 15);
    UBYTE cursor_y = 6 + (i / 15);
    UBYTE tile_id = TILE_0 + code_index;

    replace_meta_tile(cursor_x, cursor_y, tile_id, 1);
}


UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y) BANKED {
    for (BYTE dy = -PLATFORM_MIN_VERTICAL_GAP; dy <= PLATFORM_MIN_VERTICAL_GAP; dy++) {
        if (dy == 0) continue;

        BYTE check_y = y + dy;
        if (check_y < PLATFORM_Y_MIN || check_y > PLATFORM_Y_MAX) continue;

        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, check_y)]) == 1) return 1;
    }
    return 0;
}

void rebuild_platform_row(UBYTE y) BANKED {
    UBYTE i;
    UBYTE seq_start = 255;
    UBYTE current_len = 0;

    for (i = PLATFORM_X_MIN; i <= PLATFORM_X_MAX + 1; i++) {
        UBYTE is_platform = (i <= PLATFORM_X_MAX)
            && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(i, y)]) == 1;

        if (is_platform) {
            if (seq_start == 255) {
                seq_start = i;
                current_len = 1;
            } else {
                current_len++;
            }

            // If we've reached the max length, end this segment early
            if (current_len == PLATFORM_MAX_LENGTH || i == PLATFORM_X_MAX) {
                // Cap this segment now
                UBYTE j;
                if (current_len == 1) {
                    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
                } else {
                    for (j = 0; j < current_len; j++) {
                        UBYTE tile_x = seq_start + j;
                        if (j == 0)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_LEFT, 1);
                        else if (tile_x == PLATFORM_X_MAX)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_RIGHT, 1);
                        else if (j == current_len - 1)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_RIGHT, 1);
                        else
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_MIDDLE, 1);
                    }
                }

                // Prepare for new segment
                seq_start = 255;
                current_len = 0;
            }

        } else {
            // End of segment
            if (seq_start != 255) {
                if (current_len == 1) {
                    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
                } else {
                    for (UBYTE j = 0; j < current_len; j++) {
                        UBYTE tile_x = seq_start + j;
                        if (j == 0)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_LEFT, 1);
                        else if (tile_x == PLATFORM_X_MAX)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_RIGHT, 1);
                        else if (j == current_len - 1)
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_RIGHT, 1);
                        else
                            replace_meta_tile(tile_x, y, TILE_PLATFORM_MIDDLE, 1);
                    }
                }

                seq_start = 255;
                current_len = 0;
            }
        }
    }
}

#define HEX_TILE_BASE  91  // tile ID at (11, 5)
#define CHUNKS_PER_ROW 3
#define CHUNK_WIDTH    6
#define CHUNK_HEIGHT   2

void draw_quadrant_ids() BANKED {
    for (UBYTE i = 0; i < 24; i++) {
        UBYTE chunk_x = 3 + (i % CHUNKS_PER_ROW) * CHUNK_WIDTH;
        UBYTE chunk_y = PLATFORM_Y_MIN + (i / CHUNKS_PER_ROW) * CHUNK_HEIGHT;

        UBYTE row0 = 0, row1 = 0;
        extract_chunk_pattern(chunk_x, chunk_y, &row0, &row1);
        UBYTE code_index = match_platform_pattern(row0, row1);
        if (code_index != 0xFF) {
            replace_meta_tile(chunk_x, chunk_y + 1, TILE_0 + code_index, 1);
        } else {
            replace_meta_tile(chunk_x, chunk_y + 1, TILE_0, 1); // fallback to 0
        }
    }
}

void paint(UBYTE x, UBYTE y) BANKED {
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX || y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX) return;
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != 5) return;

    if (check_platform_vertical_conflict(x, y)) return;
    if (x < PLATFORM_X_MAX 
        && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == 5 
        && check_platform_vertical_conflict(x + 1, y)) return;

    UBYTE left = (x > PLATFORM_X_MIN) ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)]) : 0;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) : 0;

    if (left == 1 || right == 1) {
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
    } else if (x < PLATFORM_X_MAX && right == 5) {
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    } else {
        return;
    }

    rebuild_platform_row(y);

    UBYTE chunk_index = (x - 1) / 6;
    UBYTE chunk_x = 1 + chunk_index * 6;

    for (UBYTE i = 0; i < 8; i++) wait_vbl_done();

    update_code_at_chunk(chunk_x, chunk_index);

    UBYTE row0 = 0, row1 = 0;
    extract_chunk_pattern(chunk_x, PLATFORM_Y_MIN, &row0, &row1);

    replace_meta_tile(4, 7, TILE_0 + (row0 >> 4), 1);
    replace_meta_tile(5, 7, TILE_0 + (row0 & 0xF), 1);
    replace_meta_tile(6, 7, TILE_0 + (row1 >> 4), 1);
    replace_meta_tile(7, 7, TILE_0 + (row1 & 0xF), 1);

    draw_quadrant_ids();

    for (UBYTE i = 0; i < 18; i++) {
        display_code_tile(current_code[i], i);
    }
}

void delete_tile_at_pos(UBYTE x, UBYTE y, UBYTE commit) BANKED {
    replace_meta_tile(x, y, TILE_EMPTY, commit);
    if (commit) rebuild_platform_row(y);
    UBYTE chunk_index = (x - 1) / 6;
    UBYTE chunk_x = 1 + chunk_index * 6;
    update_code_at_chunk(chunk_x, chunk_index);

    draw_quadrant_ids();

    // Rebuild the platform row to ensure all tiles are correct
    rebuild_platform_row(y);

    // Update the display for the current code
    for (UBYTE i = 0; i < 18; i++) {
        display_code_tile(current_code[i], i);
    }

}

void vm_paint(SCRIPT_CTX *THIS) BANKED {
    UBYTE x = *(UBYTE*)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE*)VM_REF_TO_PTR(FN_ARG1);
    paint(x, y);
}

void vm_delete_tile_at_pos(SCRIPT_CTX *THIS) OLDCALL BANKED {
    uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
    uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);
    uint8_t commit = *(uint8_t *) VM_REF_TO_PTR(FN_ARG2);
    delete_tile_at_pos(x, y, commit);
}

UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED {
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
}

void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED {
    uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
    uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t*)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_pos(x, y);
}
