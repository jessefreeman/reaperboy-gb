// paint.c
#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "paint.h"     // ← add this so paint(x,y) is declared

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

// Paint the tile at (x, y) with the metatile at (5, 0):
void paint(UBYTE x, UBYTE y) BANKED {
    replace_meta_tile(x, y, 5, 1);
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