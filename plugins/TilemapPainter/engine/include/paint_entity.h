#ifndef PAINT_ENTITY_H
#define PAINT_ENTITY_H

#include <gbdk/platform.h>
#include "meta_tiles.h" // For actor_t definition

// ============================================================================
// ACTOR SYSTEM DECLARATIONS - For GB Studio Engine Compatibility
// ============================================================================

// Use GB Studio standard direction constants
#define DIRECTION_DOWN 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_LEFT 3

#define MAX_PAINT_ENEMIES 5

// ============================================================================
// PAINT ACTOR STORAGE - Global actor IDs for paint operations
// ============================================================================

extern UBYTE paint_player_id;
extern UBYTE paint_exit_id;
extern UBYTE paint_enemy_ids[MAX_PAINT_ENEMIES];
extern UBYTE paint_enemy_slots_used[MAX_PAINT_ENEMIES];

// Enemy pool tracking
extern UBYTE enemy_paint_order[MAX_PAINT_ENEMIES];
extern UBYTE enemy_paint_count;
extern UBYTE next_paint_slot;

// ============================================================================
// ENTITY VALIDATION
// ============================================================================

// Check if there is an enemy below the player position
UBYTE has_enemy_below_player(UBYTE x, UBYTE y) BANKED;

// Check if there is an enemy at or adjacent to the position
UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED;

// Count the total number of enemies on the map
UBYTE count_enemies_on_map(void) BANKED;

// Check if the player can be painted at the position
UBYTE can_paint_player(UBYTE x, UBYTE y) BANKED;

// Check if a right-facing enemy can be painted at the position
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED;

// Find the next valid position for an enemy
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y) BANKED;

// ============================================================================
// ENTITY MOVEMENT AND MANAGEMENT
// ============================================================================

// Move an actor to a specific tile position
void move_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;

// Move a player actor to a tile position with special handling
void move_player_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED;

// Get the next enemy slot from the FIFO pool
UBYTE get_next_enemy_slot_from_pool(void) BANKED;

// Modify enemy paint order
void remove_enemy_from_paint_order(UBYTE slot) BANKED;
void add_enemy_to_front_of_paint_order(UBYTE slot) BANKED;

// Clear an enemy tile at a specific position
void clear_enemy_tile_at_position(UBYTE x, UBYTE y) BANKED;

// Reset the enemy pool to initial state
void reset_enemy_pool(void) BANKED;

// Clear any existing player on row 11
void clear_existing_player_on_row_11(void) BANKED;

// Remove enemies above a platform
void remove_enemies_above_platform(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// ENTITY PAINTING
// ============================================================================

// Paint the player at a position
void paint_player(UBYTE x, UBYTE y) BANKED;

// Paint a right-facing enemy at a position
void paint_enemy_right(UBYTE x, UBYTE y) BANKED;

// Paint a left-facing enemy at a position
void paint_enemy_left(UBYTE x, UBYTE y) BANKED;

// Delete an enemy at a position
void delete_enemy(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// EXIT POSITIONING
// ============================================================================

// Position the exit actor based on player position
void position_exit_for_player(UBYTE player_x, UBYTE player_y) BANKED;

// Clear any existing exit tiles
void clear_existing_exit_tiles(void) BANKED;

// ============================================================================
// DIRECT LEVEL CODE UPDATE FUNCTIONS
// ============================================================================

// Add enemy to level code structure directly
void add_enemy_to_level_code(UBYTE x, UBYTE y, UBYTE direction) BANKED;

// Remove enemy from level code structure directly  
void remove_enemy_from_level_code(UBYTE x, UBYTE y) BANKED;

// Update enemy direction in level code structure directly
void update_enemy_direction_in_level_code(UBYTE x, UBYTE y, UBYTE direction) BANKED;

#endif // PAINT_ENTITY_H
