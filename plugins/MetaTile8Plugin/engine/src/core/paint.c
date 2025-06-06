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

#define PLATFORM_MAX_LENGTH 8

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



void paint(UBYTE x, UBYTE y) BANKED {
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX || y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX) return;
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != 5) return;

    // Check vertical conflicts
    if (check_platform_vertical_conflict(x, y)) return;
    if (x < PLATFORM_X_MAX && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == 5) {
        if (check_platform_vertical_conflict(x + 1, y)) return;
    }

    UBYTE left = (x > PLATFORM_X_MIN) ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)]) : 0;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) : 0;

    if (left == 1 || right == 1) {
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
    } else if (x < PLATFORM_X_MAX && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == 5) {
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    } else {
        return;
    }

    rebuild_platform_row(y);
}

void delete_tile_at_pos(UBYTE x, UBYTE y, UBYTE commit) BANKED {
    replace_meta_tile(x, y, TILE_EMPTY, commit);
    if (commit) rebuild_platform_row(y);
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
