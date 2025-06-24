#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <gbdk/platform.h>
#include "vm.h"

// ============================================================================
// CORE PATTERN EXTRACTION AND MATCHING
// ============================================================================

UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UWORD match_platform_pattern(UWORD pattern) BANKED;

// ============================================================================
// LEVEL CODE SYSTEM - Unified 24-character display
// ============================================================================

// Main level code functions
void display_complete_level_code(void) BANKED;
void init_level_code(void) BANKED;
void update_complete_level_code(void) BANKED;

// Zone management
void draw_segment_ids(void) BANKED;
void update_zone_code(UBYTE zone_index) BANKED;
UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED;

// Encoding functions for compact display
UBYTE encode_enemy_bitmask(void) BANKED;
UBYTE encode_enemy_directions(void) BANKED;

// Character display helpers
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED;
void clear_level_code_display(void) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED;
void vm_init_level_code(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

void draw_debug_pattern(UBYTE pattern_index) BANKED;
void vm_debug_next_pattern(SCRIPT_CTX *THIS) BANKED;
void vm_debug_prev_pattern(SCRIPT_CTX *THIS) BANKED;
void vm_debug_reset_pattern(SCRIPT_CTX *THIS) BANKED;
void test_enemy_encoding(void) BANKED;
void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED;

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

// ============================================================================

#endif // CODE_GEN_H
