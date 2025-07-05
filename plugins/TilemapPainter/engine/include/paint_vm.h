#ifndef PAINT_VM_H
#define PAINT_VM_H

#include <gbdk/platform.h>
#include "vm.h"

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

// Set up actor IDs for paint operations
void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED;

// Enable the paint editor
void vm_enable_editor(SCRIPT_CTX *THIS) BANKED;

// Paint at a position
void vm_paint(SCRIPT_CTX *THIS) BANKED;

// Get the brush tile position state
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;

// Get the brush preview tile state
void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED;

#endif // PAINT_VM_H
