#ifndef CODE_ENEMY_SYSTEM_H
#define CODE_ENEMY_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// ENEMY SYSTEM FUNCTIONS
// ============================================================================

// System initialization
void init_enemy_system(void) BANKED;

// Enemy type detection
UBYTE detect_enemy_type(UBYTE tile_type) BANKED;

// Enemy data extraction
void extract_enemy_data(void) BANKED;

// Enemy encoding functions for 24-character display
UBYTE encode_enemy_positions(void) BANKED;
UBYTE encode_enemy_details_1(void) BANKED;
UBYTE encode_enemy_details_2(void) BANKED;
UBYTE encode_enemy_directions(void) BANKED;
UBYTE encode_enemy_bitmask(void) BANKED; // Compatibility alias

// Enemy pattern system (from refactor document)
typedef struct
{
    UBYTE pattern_id;
    UBYTE positions[MAX_ENEMIES];
    UBYTE count;
    char description[32];
} enemy_pattern_t;

extern const enemy_pattern_t ENEMY_PATTERNS[];

// Pattern functions
UBYTE extract_enemy_pattern_for_block_row(UBYTE row) BANKED;
void apply_enemy_pattern_to_block_row(UBYTE row, UBYTE pattern_id, UBYTE direction_mask) BANKED;

// Validation and testing
void validate_enemy_encoding(void) BANKED;
void init_test_enemy_patterns(void) BANKED;
void test_cycle_enemy_patterns(void) BANKED;
void test_cycle_enemy_directions(void) BANKED;
void update_enemy_patterns(void) BANKED;
void validate_enemy_positions_after_platform_change(void) BANKED;

// Debug functions
void test_enemy_encoding(void) BANKED;

// VM wrapper functions
void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED;
void vm_get_enemy_info(SCRIPT_CTX *THIS) BANKED;
void vm_init_test_enemy_patterns(SCRIPT_CTX *THIS) BANKED;

// Character editing handlers
void handle_enemy_data_edit(UBYTE char_index, UBYTE new_value) BANKED;

#endif // CODE_ENEMY_SYSTEM_H
