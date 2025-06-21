#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

// Selector animation state constants
#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_LEFT 2
#define SELECTOR_STATE_ENEMY_RIGHT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLATFORM_CENTER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7
#define SELECTOR_STATE_PLAYER 8

void paint(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_preview_tile(UBYTE x, UBYTE y) BANKED;
UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED;
UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED;
UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y) BANKED;

void vm_paint(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED;
void vm_move_actor_to_test(SCRIPT_CTX *THIS) OLDCALL BANKED;
void vm_setup_map(SCRIPT_CTX *THIS, INT16 idx) OLDCALL BANKED;

#endif
