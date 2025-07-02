#ifndef CODE_ENEMY_SYSTEM_H
#define CODE_ENEMY_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// NEW SIMPLIFIED ENEMY ENCODING SYSTEM
// ============================================================================

// Character set constants
extern const char POS41[];  // 41-character position alphabet
extern const char BASE32[]; // 32-character base alphabet
extern const UBYTE POS41_TILE_MAP[];
extern const UBYTE BASE32_TILE_MAP[];

// ============================================================================
// CORE ENEMY FUNCTIONS
// ============================================================================

// Data extraction from tilemap
void extract_enemy_data(void) BANKED;

// New encoding functions (POS41 system)
char encode_enemy_position(UBYTE enemy_index) BANKED;
char encode_odd_mask(void) BANKED;
char encode_direction_mask(void) BANKED;

// Decoding functions for level code editing
void decode_enemy_position(UBYTE enemy_index, char pos_char, UBYTE odd_bit, UBYTE dir_bit) BANKED;
void decode_enemy_data_from_level_code(const char *enemy_chars) BANKED;

// ============================================================================
// ACTOR MANAGEMENT FUNCTIONS
// ============================================================================

// Actor management for enemy placement/removal
void clear_all_enemy_actors(void) BANKED;
void place_enemy_actor(UBYTE enemy_index, UBYTE col, UBYTE row, UBYTE direction) BANKED;

// ============================================================================
// COMPATIBILITY FUNCTIONS (maintain existing API)
// ============================================================================

// These functions now use the new encoding system internally
UBYTE encode_enemy_positions(void) BANKED;  // Character 17
UBYTE encode_enemy_details_1(void) BANKED;  // Character 18
UBYTE encode_enemy_details_2(void) BANKED;  // Character 19
UBYTE encode_enemy_directions(void) BANKED; // Character 23
UBYTE encode_enemy_bitmask(void) BANKED;    // Compatibility alias

// New functions for additional characters
UBYTE encode_enemy_position_4(void) BANKED; // Character 20
UBYTE encode_enemy_position_5(void) BANKED; // Character 21
UBYTE encode_odd_mask_value(void) BANKED;   // Character 22

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Character set utilities
UBYTE find_pos41_index(char c) BANKED;
UBYTE find_base32_index(char c) BANKED;
UBYTE get_pos41_display_tile(char c) BANKED;
UBYTE get_base32_display_tile(char c) BANKED;
UBYTE enemy_char_to_value(UBYTE tile_id) BANKED;

// Row detection
UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED;

// Actor management
void clear_enemy_actor(UBYTE enemy_index) BANKED;
void place_enemy_actor(UBYTE enemy_index, UBYTE tilemap_x, UBYTE tilemap_y, UBYTE direction) BANKED;
void clear_all_enemy_actors(void) BANKED;

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// ============================================================================

// Pattern system (simplified)
UBYTE extract_enemy_pattern_for_block_row(UBYTE row) BANKED;
void apply_enemy_pattern_to_block_row(UBYTE row, UBYTE pattern_id, UBYTE direction_mask) BANKED;

// Testing and validation
void validate_enemy_encoding(void) BANKED;
void init_test_enemy_patterns(void) BANKED;
void test_cycle_enemy_patterns(void) BANKED;
void test_cycle_enemy_directions(void) BANKED;
void update_enemy_patterns(void) BANKED;
void validate_enemy_positions_after_platform_change(void) BANKED;

// New testing functions
void test_new_enemy_encoding(void) BANKED;
void validate_enemy_encoding_roundtrip(void) BANKED;

// ============================================================================
// LEVEL CODE EDITING SUPPORT
// ============================================================================

void handle_enemy_data_edit(UBYTE char_index, UBYTE new_value) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_test_new_enemy_encoding(SCRIPT_CTX *THIS) BANKED;
void vm_validate_enemy_encoding_roundtrip(SCRIPT_CTX *THIS) BANKED;
void vm_extract_enemy_data(SCRIPT_CTX *THIS) BANKED;
void vm_get_enemy_encoding_info(SCRIPT_CTX *THIS) BANKED;

// VM functions for direct enemy level code editing
void vm_edit_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED;
void vm_increment_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED;
void vm_decrement_enemy_level_code_char(SCRIPT_CTX *THIS) BANKED;
void vm_test_enemy_level_code_editing(SCRIPT_CTX *THIS) BANKED;

// Legacy VM functions (maintained for compatibility)
void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED;
void vm_get_enemy_info(SCRIPT_CTX *THIS) BANKED;
void vm_init_test_enemy_patterns(SCRIPT_CTX *THIS) BANKED;

#endif // CODE_ENEMY_SYSTEM_H
