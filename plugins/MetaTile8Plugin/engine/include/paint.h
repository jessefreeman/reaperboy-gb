#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

// Selector animation state constants
#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_RIGHT 2
#define SELECTOR_STATE_ENEMY_LEFT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLAYER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7

// Helper function to validate platform row placement
UBYTE is_valid_platform_row(UBYTE y) BANKED;

// Helper functions for player placement
UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED;
UBYTE can_paint_player(UBYTE x, UBYTE y) BANKED;
void clear_existing_player_on_row_11(void) BANKED;
void paint_player(UBYTE x, UBYTE y) BANKED;

// Helper functions for enemy placement
UBYTE count_enemies_on_map(void) BANKED;
UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED;
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED;
void remove_enemies_above_platform(UBYTE x, UBYTE y) BANKED;
void delete_enemy(UBYTE x, UBYTE y) BANKED;
void paint_enemy_left(UBYTE x, UBYTE y) BANKED;
void paint_enemy_right(UBYTE x, UBYTE y) BANKED;

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
