#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

// ============================================================================
// SELECTOR STATE CONSTANTS
// ============================================================================

#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_RIGHT 2
#define SELECTOR_STATE_ENEMY_LEFT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLAYER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7

// ============================================================================
// CORE PAINTING FUNCTIONS
// ============================================================================

void paint(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// VALIDATION FUNCTIONS
// ============================================================================

UBYTE is_valid_platform_row(UBYTE y) BANKED;
UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED;
UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED;
UBYTE has_enemy_below_player(UBYTE x, UBYTE y) BANKED;
UBYTE can_paint_player(UBYTE x, UBYTE y) BANKED;
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED;
UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y) BANKED;
UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED;
UBYTE count_connected_platform_length(UBYTE x, UBYTE y) BANKED;
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// ENTITY MANAGEMENT
// ============================================================================

void move_player_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;
void move_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;
UBYTE find_next_available_enemy_slot(void) BANKED;
void paint_player(UBYTE x, UBYTE y) BANKED;
void paint_enemy_right(UBYTE x, UBYTE y) BANKED;
void paint_enemy_left(UBYTE x, UBYTE y) BANKED;
void delete_enemy(UBYTE x, UBYTE y) BANKED;
void clear_existing_player_on_row_11(void) BANKED;
void remove_enemies_above_platform(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// PLATFORM MANAGEMENT
// ============================================================================

void rebuild_platform_row(UBYTE y) BANKED;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

UBYTE count_enemies_on_map(void) BANKED;
UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED;
UBYTE validate_level_setup(void) BANKED;
void get_level_stats(UBYTE *player_x, UBYTE *enemy_count) BANKED;

// Optimized utility functions
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y) BANKED;
UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// SMART UPDATE FUNCTIONS
// ============================================================================

void update_level_code_for_paint(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED;
void vm_enable_editor(SCRIPT_CTX *THIS) BANKED;
void vm_paint(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED;
void vm_setup_map(SCRIPT_CTX *THIS, INT16 idx) OLDCALL BANKED;

#endif // PAINT_H
