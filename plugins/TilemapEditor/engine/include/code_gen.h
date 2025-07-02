#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <gbdk/platform.h>
#include "vm.h"

// Include the modular system headers
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "code_persistence.h"

// ============================================================================
// COORDINATOR-SPECIFIC CONSTANTS
// ============================================================================

// Note: Core constants and types are now in code_level_core.h

// ============================================================================
// COORDINATION FUNCTIONS - High-level operations using multiple modules
// ============================================================================

// Main coordination functions
void regenerate_complete_level(void) BANKED;
void handle_platform_change(UBYTE block_index, UBYTE pattern_id) BANKED;
void handle_level_code_edit(UBYTE char_index, UBYTE new_value) BANKED;

// Zone management coordination
void draw_segment_ids(void) BANKED;
void update_zone_code(UBYTE zone_index) BANKED;
UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS - Coordination entry points for GB Studio scripts
// ============================================================================

// Main coordination VM wrappers
void vm_save_level_code(SCRIPT_CTX *THIS) BANKED;
void vm_load_level_code(SCRIPT_CTX *THIS) BANKED;

// Test VM wrappers
void vm_init_test_patterns(SCRIPT_CTX *THIS) BANKED;
void vm_test_cycle_enemy_patterns(SCRIPT_CTX *THIS) BANKED;
void vm_test_cycle_enemy_directions(SCRIPT_CTX *THIS) BANKED;

// Legacy compatibility VM wrappers
void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED;
void vm_init_level_code(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS - Maintain backward compatibility
// ============================================================================

// Legacy function names that may be called from existing scripts
void display_level_code(void) BANKED;
void save_current_level_code(void) BANKED;
void load_saved_level_code(void) BANKED;

#endif // CODE_GEN_H
