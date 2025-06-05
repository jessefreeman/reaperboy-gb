// paint.c
#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "paint.h"     // ← add this so paint(x,y) is declared

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

// VM wrapper for “get_brush_tile”. Pops X→FN_ARG0, Y→FN_ARG1, VarIdx→FN_ARG2,
// calls get_brush_tile(x,y), stores result into script_memory[varIdx].
void vm_get_brush_tile(SCRIPT_CTX *THIS) BANKED {
    UBYTE x        = *(UBYTE *) VM_REF_TO_PTR(FN_ARG0);
    UBYTE y        = *(UBYTE *) VM_REF_TO_PTR(FN_ARG1);
    uint16_t varIdx = *(uint16_t*) VM_REF_TO_PTR(FN_ARG2);

    UBYTE result = get_brush_tile(x, y);
    script_memory[varIdx] = result;
}

// Pure-C helper: read the metatile ID from SRAM at (x,y).
UBYTE get_brush_tile(UBYTE x, UBYTE y) BANKED {
    return sram_map_data[METATILE_MAP_OFFSET(x, y)];
}