#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

// Include all paint module headers
#include "paint_core.h"
#include "paint_platform.h"
#include "paint_entity.h"
#include "paint_ui.h"
#include "paint_vm.h"

// ============================================================================
// CORE PAINTING FUNCTIONS
// ============================================================================

// Core paint functions
void paint(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED;

// VM interface functions
void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED;
void vm_enable_editor(SCRIPT_CTX *THIS) BANKED;
void vm_paint(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED;

#endif // PAINT_H
