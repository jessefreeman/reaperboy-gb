#ifndef CODE_ENEMY_SYSTEM_H
#define CODE_ENEMY_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// INITIALIZATION
// ============================================================================

// Initialize the enemy system including position validation
void init_enemy_system(void) BANKED;

// ============================================================================
// ENEMY NUMERIC SYSTEM
// ============================================================================

// ============================================================================
// CORE ENEMY FUNCTIONS
// ============================================================================

// Data extraction from tilemap
void extract_enemy_data(void) BANKED;

// Encoding functions - all return numeric values directly
UBYTE encode_enemy_position(UBYTE enemy_index) BANKED;
UBYTE encode_odd_mask(void) BANKED;
UBYTE encode_direction_mask(void) BANKED;

// Decoding functions for level code editing
void decode_enemy_position(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit, UBYTE dir_bit) BANKED;
void decode_enemy_data_from_values(const UBYTE *enemy_values) BANKED;

// ============================================================================
// LEVEL CODE INTERFACE FUNCTIONS (called by code_level_core.c)
// ============================================================================

UBYTE encode_enemy_positions(void) BANKED;  // Character 17
UBYTE encode_enemy_details_1(void) BANKED;  // Character 18
UBYTE encode_enemy_details_2(void) BANKED;  // Character 19
UBYTE encode_enemy_position_4(void) BANKED; // Character 20
UBYTE encode_enemy_position_5(void) BANKED; // Character 21
UBYTE encode_odd_mask_value(void) BANKED;   // Character 22
UBYTE encode_enemy_directions(void) BANKED; // Character 23

// ============================================================================
// LEVEL CODE EDITING SUPPORT
// ============================================================================

void handle_enemy_data_edit(UBYTE char_index, UBYTE new_value) BANKED;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Tile mapping
UBYTE get_pos41_display_tile(UBYTE value) BANKED;
UBYTE get_base32_display_tile(UBYTE value) BANKED;
UBYTE enemy_char_to_value(UBYTE tile_id) BANKED;

// Row detection
UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED;

// ============================================================================
// ACTOR MANAGEMENT
// ============================================================================

void clear_enemy_actor(UBYTE enemy_index) BANKED;
void place_enemy_actor(UBYTE enemy_index, UBYTE tilemap_x, UBYTE tilemap_y, UBYTE direction) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

// No VM functions needed - all enemy operations are handled through
// handle_enemy_data_edit() called from code_level_core.c

#endif // CODE_ENEMY_SYSTEM_H
