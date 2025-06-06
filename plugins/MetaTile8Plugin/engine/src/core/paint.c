// paint.c
#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "paint.h"     // ← add this so paint(x,y) is declared

#define PLATFORM_X_MIN 2
#define PLATFORM_X_MAX 21
#define PLATFORM_Y_MIN 11
#define PLATFORM_Y_MAX 20

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

// VM wrapper: pop X (arg 0) and Y (arg 1) from the VM stack
void vm_paint(SCRIPT_CTX *THIS) BANKED {
    
    UBYTE x = *(UBYTE*)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE*)VM_REF_TO_PTR(FN_ARG1);

    paint(x,y);
}

void paint(UBYTE x, UBYTE y) BANKED {
    // 0) Hard‐coded bounds:
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX || y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX) return;

    // 1) Only paint on empty:
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != 5) return;

    // 2) Read neighbors (within bounds because of step 0):
    UBYTE leftType = 0, rightType = 0;
    if (x > PLATFORM_X_MIN) {
        leftType = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)]);
    }
    if (x < PLATFORM_X_MAX) {
        rightType = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]);
    }

    // 3a) If there’s a platform to the right (i.e. x is immediately left of it),
    //     place a LEFT cap at x and turn (x+1) into MIDDLE:
    if (rightType == 1) {
        replace_meta_tile(x,     y, TILE_PLATFORM_LEFT,   1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_MIDDLE, 1);
        return;
    }

    // 3b) If there’s a platform to the left (i.e. x is immediately right of it),
    //     place a RIGHT cap at x and turn (x-1) into MIDDLE:
    if (leftType == 1) {
        replace_meta_tile(x,     y, TILE_PLATFORM_RIGHT,  1);
        replace_meta_tile(x - 1, y, TILE_PLATFORM_MIDDLE, 1);
        return;
    }

    // 4) Otherwise start a new 2‐tile platform (x=LEFT, x+1=RIGHT) if x+1 is in bounds and empty:
    if (x < PLATFORM_X_MAX &&
        get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == 5) {
        replace_meta_tile(x,     y, TILE_PLATFORM_LEFT,  1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    }
}

// VM wrapper for “get_brush_tile”.
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED {
    uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);
	script_memory[*(int16_t*)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_pos(x, y);
}

// Pure-C helper: read the metatile ID from SRAM at (x,y).
UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED {
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
}

// VM wrapper for vm_delete_tile_at_pos.
void vm_delete_tile_at_pos(SCRIPT_CTX * THIS) OLDCALL BANKED {
	uint8_t x = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	uint8_t y = *(uint8_t *) VM_REF_TO_PTR(FN_ARG1);	
	uint8_t commit = *(uint8_t *) VM_REF_TO_PTR(FN_ARG2);	
	delete_tile_at_pos(x, y, commit);	
}

void delete_tile_at_pos(UBYTE x, UBYTE y, UBYTE commit) BANKED {	
	replace_meta_tile(x, y, TILE_EMPTY, commit);
}