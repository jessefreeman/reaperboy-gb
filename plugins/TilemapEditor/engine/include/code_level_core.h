#ifndef CODE_LEVEL_CORE_H
#define CODE_LEVEL_CORE_H

#include <gbdk/platform.h>
#include "vm.h"

// ============================================================================
// SHARED CONSTANTS AND DATA STRUCTURES
// ============================================================================

// Core geometric constants
#define PLATFORM_Y_MIN 12
#define PLATFORM_Y_MAX 19
#define PLATFORM_X_MIN 2
#define PLATFORM_X_MAX 21
#define SEGMENTS_PER_ROW 4
#define SEGMENT_WIDTH 5
#define SEGMENT_HEIGHT 2
#define TOTAL_BLOCKS 16

// Enemy system constants
#define MAX_ENEMIES 6 // Maximum number of enemy actors supported by the system

// Level code display settings - 24 character display system
#define LEVEL_CODE_START_X 5
#define LEVEL_CODE_START_Y 6
#define LEVEL_CODE_CHARS_TOTAL 24

// Level code layout:
// 0-15: Platform patterns (16 blocks)
// 16: Player column position
// 17-23: Enemy data (7 chars reserved)

// Platform tile IDs
#define PLATFORM_TILE_1 4
#define PLATFORM_TILE_2 5
#define PLATFORM_TILE_3 6

// Variable IDs for storing level code (define these in GB Studio)
#define VAR_LEVEL_CODE_PART_1 0 // Platform patterns 0-2   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_2 1 // Platform patterns 3-5   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_3 2 // Platform patterns 6-8   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_4 3 // Platform patterns 9-11  (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_5 4 // Platform patterns 12-14 (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_6 5 // Platform pattern 15 (1×5 bits = 5 bits)

// ============================================================================
// LEVEL CODE DATA STRUCTURE
// ============================================================================

// Main level code data structure containing all level information
typedef struct
{
    UBYTE platform_patterns[TOTAL_BLOCKS]; // 16 platform pattern IDs (0-15)
    UBYTE player_column;                   // Player starting column (0-19)
    UBYTE enemy_directions;                // Bitmask for enemy facing directions
    UBYTE enemy_types;                     // Bitmask for enemy types (walker/jumper)
    UBYTE enemy_positions[MAX_ENEMIES];    // Enemy column positions (255 = empty) - must be last
} level_code_t;

// Main level code data structure - shared across all modules
extern level_code_t current_level_code;

// ============================================================================
// LEVEL CODE CORE FUNCTIONS
// ============================================================================

// Core initialization and coordination
void init_level_code(void) BANKED;
void update_complete_level_code(void) BANKED;

// Display system functions
void display_complete_level_code(void) BANKED;
void display_selective_level_code(void) BANKED;
void display_selective_level_code_fast(void) BANKED;
void force_complete_level_code_display(void) BANKED;

// Selective update system
void mark_display_position_for_update(UBYTE position) BANKED;
UBYTE display_position_needs_update(UBYTE position) BANKED;
void clear_display_update_flags(void) BANKED;
void get_display_position(UBYTE char_index, UBYTE *x, UBYTE *y) BANKED;
void detect_level_code_changes(void) BANKED;

// Character display functions
UBYTE get_extended_display_char(UBYTE value) BANKED;
void display_char_at_position(UBYTE display_char, UBYTE x, UBYTE y) BANKED;
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED;
void clear_level_code_display(void) BANKED;

// Utility functions
UBYTE get_char_index_from_display_position(UBYTE x, UBYTE y) BANKED;

// Helper function for enemy data display
UBYTE get_enemy_display_char(UBYTE value, UBYTE char_position) BANKED;

// ============================================================================
// LEVEL CODE EDITING FUNCTIONS
// ============================================================================

// Handle level code character edits
void handle_level_code_character_edit(UBYTE char_index, UBYTE new_value) BANKED;
void vm_handle_level_code_character_edit(SCRIPT_CTX *THIS) BANKED;

// Testing functions
void test_level_code_character_editing(void) BANKED;
void vm_test_level_code_character_editing(SCRIPT_CTX *THIS) BANKED;

// New bidirectional testing functions
void test_level_code_editing_system(void) BANKED;
void vm_test_level_code_editing_system(SCRIPT_CTX *THIS) BANKED;
void test_increment_enemy_position_17(void) BANKED;
void vm_test_increment_enemy_position_17(SCRIPT_CTX *THIS) BANKED;

// Player actor management
void update_player_actor_position(void) BANKED;

// External function from platform system
extern void reconstruct_tilemap_from_level_code(void) BANKED;

// ============================================================================
// BIDIRECTIONAL LEVEL CODE EDITING FUNCTIONS
// ============================================================================

// External change tracking
void mark_level_code_position_changed(UBYTE position, UBYTE new_value) BANKED;
void process_level_code_external_changes(void) BANKED;

// VM functions for level code editing
void vm_set_level_code_display_value(SCRIPT_CTX *THIS) BANKED;
void vm_increment_level_code_value(SCRIPT_CTX *THIS) BANKED;
void vm_decrement_level_code_value(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// DIRECT LEVEL CODE EDITING VM FUNCTIONS
// ============================================================================

// Easy test functions
void test_set_enemy_17_to_value_11(void) BANKED;
void test_increment_enemy_17(void) BANKED;
void test_set_enemy_direction_left(void) BANKED;
void vm_test_set_enemy_17_to_value_11(SCRIPT_CTX *THIS) BANKED;
void vm_test_increment_enemy_17(SCRIPT_CTX *THIS) BANKED;
void vm_test_set_enemy_direction_left(SCRIPT_CTX *THIS) BANKED;

// Sync function for level code display values
void sync_level_code_display_values(void) BANKED;

#endif // CODE_LEVEL_CORE_H
