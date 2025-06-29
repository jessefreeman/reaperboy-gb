#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <gbdk/platform.h>
#include "vm.h"

// ============================================================================
// CORE CONSTANTS AND TYPE DEFINITIONS
// ============================================================================

#define TOTAL_BLOCKS 16
#define MAX_ENEMIES 5

// Level code constants
#define LEVEL_CODE_CHARS_TOTAL 24
#define TOTAL_PLATFORM_BLOCKS 16

// Screen dimensions for metatile system
#define METATILE_SCREEN_WIDTH 20
#define METATILE_SCREEN_HEIGHT 18

// Level code structure - CORRECTED: Platform patterns must come first!
typedef struct
{
    UBYTE platform_patterns[TOTAL_BLOCKS]; // Characters 0-15: Platform patterns (16 blocks, 5x2 each)
    UBYTE player_column;                   // Character 16: Player starting column (0-19)
    UBYTE enemy_patterns[5];               // Characters 17-21: Enemy pattern data (one per block row)
    UBYTE enemy_directions;                // Character 22: Enemy direction bitmask (5 bits used)
    UBYTE reserved;                        // Character 23: Reserved for future use

    // Legacy compatibility fields (kept for backward compatibility)
    UBYTE enemy_positions[MAX_ENEMIES]; // Enemy column positions (255=empty) - DEPRECATED
    UBYTE enemy_types;                  // Packed enemy type bits (6 bits: 0=walker, 1=jumper) - DEPRECATED
} level_code_t;

// External reference to main level code structure
extern level_code_t current_level_code;

// External reference to display update flags
extern UBYTE update_flags[LEVEL_CODE_CHARS_TOTAL];

// ============================================================================
// CORE PATTERN EXTRACTION AND MATCHING
// ============================================================================

UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UWORD match_platform_pattern(UWORD pattern) BANKED;

// Platform application with proper end cap logic
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UWORD pattern, UBYTE block_index) BANKED;
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED;
void update_neighboring_block_codes(UBYTE block_index) BANKED;
void update_single_block_code(UBYTE block_index) BANKED;
void reconstruct_tilemap_from_level_code(void) BANKED;
void apply_row_platforms(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED;
void place_platform_run(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED;
UBYTE has_adjacent_platform(UBYTE block_index, BYTE direction) BANKED;

// ============================================================================
// LEVEL CODE SYSTEM - Unified 24-character display
// ============================================================================

// Main level code functions
void display_complete_level_code(void) BANKED;
void force_complete_level_code_display(void) BANKED;
void init_level_code(void) BANKED;
void update_complete_level_code(void) BANKED;

// Selective update system
void mark_display_position_for_update(UBYTE position) BANKED;
void detect_level_code_changes(void) BANKED;
void clear_display_update_flags(void) BANKED;
void display_selective_level_code(void) BANKED;
void display_selective_level_code_fast(void) BANKED;
void get_display_position(UBYTE char_index, UBYTE *x, UBYTE *y) BANKED;
UBYTE display_position_needs_update(UBYTE position) BANKED;

// Zone management
void draw_segment_ids(void) BANKED;
void update_zone_code(UBYTE zone_index) BANKED;
UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED;
void update_all_affected_block_codes(UBYTE block_index) BANKED;
void validate_final_pattern_match(UBYTE block_index, UBYTE intended_pattern_id) BANKED;
void validate_all_block_patterns(void) BANKED;

// Encoding functions for compact display
UBYTE encode_enemy_bitmask(void) BANKED;
UBYTE encode_enemy_directions(void) BANKED;

// Character display helpers
UBYTE get_extended_display_char(UBYTE value) BANKED;
void display_char_at_position(UBYTE display_char, UBYTE x, UBYTE y) BANKED;
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED;
void clear_level_code_display(void) BANKED;

// Character display helpers
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED;
void clear_level_code_display(void) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED;
void vm_init_level_code(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// PERSISTENT STORAGE FUNCTIONS
// ============================================================================

// Variable-based storage (recommended for simple persistence)
void save_level_code_to_variables(void) BANKED;
void load_level_code_from_variables(void) BANKED;
void vm_save_level_code(SCRIPT_CTX *THIS) BANKED;
void vm_load_level_code(SCRIPT_CTX *THIS) BANKED;
void vm_has_saved_level_code(SCRIPT_CTX *THIS) BANKED;

// SRAM-based storage (for more complex persistence)
void save_level_code_to_sram(void) BANKED;
UBYTE load_level_code_from_sram(void) BANKED;
void vm_save_level_code_sram(SCRIPT_CTX *THIS) BANKED;
void vm_load_level_code_sram(SCRIPT_CTX *THIS) BANKED;

void vm_cycle_character(SCRIPT_CTX *THIS) OLDCALL BANKED;

// ============================================================================
// VALID PLAYER POSITION SYSTEM
// ============================================================================

void update_valid_player_positions(void) BANKED;
UBYTE get_next_valid_player_position(UBYTE current_position) BANKED;
UBYTE is_valid_player_position(UBYTE column) BANKED;

// Debug functions
void debug_show_valid_positions(void) BANKED;
void vm_debug_show_valid_positions(SCRIPT_CTX *THIS) BANKED;
void test_valid_player_positions(void) BANKED;
void vm_test_valid_player_positions(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// EXIT POSITIONING SYSTEM
// ============================================================================

void update_exit_position_after_platform_change(void) BANKED;

// ============================================================================
// ENEMY PATTERN ENCODING SYSTEM - New refactored system
// ============================================================================

// Extract and apply enemy patterns for block rows
UBYTE extract_enemy_pattern_for_block_row(UBYTE row) BANKED;
void apply_enemy_pattern_to_block_row(UBYTE row, UBYTE pattern_id, UBYTE direction_mask) BANKED;

// Enemy patterns from refactor document
// Enemy pattern structure for new pattern system
typedef struct
{
    UBYTE pattern_id;
    UBYTE positions[MAX_ENEMIES];
    UBYTE count;
    char description[32];
} enemy_pattern_t;

// External reference to enemy patterns
extern const enemy_pattern_t ENEMY_PATTERNS[];

// Debug and test functions for enemy patterns
void init_test_enemy_patterns(void) BANKED;
void vm_init_test_enemy_patterns(SCRIPT_CTX *THIS) BANKED;

// Additional helper functions for enemy pattern system
void test_cycle_enemy_patterns(void) BANKED;
void test_cycle_enemy_directions(void) BANKED;
UBYTE is_valid_player_position(UBYTE column) BANKED;
UBYTE get_next_valid_player_position(UBYTE current_position) BANKED;
void position_player_at_valid_location(void) BANKED;

// Enemy encoding functions for new pattern system
UBYTE encode_enemy_pattern_id(void) BANKED;
UBYTE encode_enemy_directions(void) BANKED;
UBYTE encode_enemy_details(void) BANKED;

#endif // CODE_GEN_H
