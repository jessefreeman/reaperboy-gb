#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

void paint(UBYTE x, UBYTE y) BANKED;
void delete_tile_at_pos(UBYTE x, UBYTE y, UBYTE commit) BANKED;
UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED;

void vm_paint(SCRIPT_CTX *THIS) BANKED;
void vm_delete_tile_at_pos(SCRIPT_CTX *THIS) OLDCALL BANKED;
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;

#endif
