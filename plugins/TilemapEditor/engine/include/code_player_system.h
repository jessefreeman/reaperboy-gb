#ifndef CODE_PLAYER_SYSTEM_H
#define CODE_PLAYER_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// PLAYER SYSTEM DATA
// ============================================================================

// Valid player position tracking
extern UBYTE column_has_platform[20];
extern UBYTE valid_player_columns[20];
extern UBYTE valid_player_count;

// ============================================================================
// PLAYER SYSTEM FUNCTIONS
// ============================================================================

// System initialization
void init_player_system(void) BANKED;

// Player data extraction
void extract_player_data(void) BANKED;

// Valid position management
void init_column_platform_tracking(void) BANKED;
void refresh_column_platform_tracking(void) BANKED;
void rebuild_valid_player_list(void) BANKED;
void update_column_platform_painted(UBYTE tilemap_col, UBYTE tilemap_row) BANKED;
void update_column_platform_deleted(UBYTE tilemap_col, UBYTE tilemap_row) BANKED;
void update_valid_player_positions(void) BANKED;
UBYTE is_valid_player_position(UBYTE column) BANKED;
UBYTE get_next_valid_player_position(UBYTE current_position) BANKED;
UBYTE get_previous_valid_player_position(UBYTE current_position) BANKED;

// Position utility functions
void position_player_at_valid_location(void) BANKED;
void update_exit_position_after_platform_change(void) BANKED;
void update_player_system(void) BANKED;

// Debug functions
void debug_show_valid_positions(void) BANKED;
void test_valid_player_positions(void) BANKED;
void debug_print_platform_tracking(void) BANKED;
void debug_test_platform_detection(void) BANKED;
void debug_allow_all_player_positions(void) BANKED;
void debug_show_tracking_state(void) BANKED;

// VM wrapper functions
void vm_test_valid_player_positions(SCRIPT_CTX *THIS) BANKED;
void vm_debug_show_valid_positions(SCRIPT_CTX *THIS) BANKED;

// Character editing handlers
void handle_player_position_edit(UBYTE new_value) BANKED;

#endif // CODE_PLAYER_SYSTEM_H
