#ifndef CODE_PERSISTENCE_H
#define CODE_PERSISTENCE_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// PERSISTENCE CONSTANTS
// ============================================================================

// SRAM storage constants
#define SRAM_LEVEL_CODE_OFFSET 0x0000
#define SRAM_LEVEL_CODE_MAGIC 0xABCD

// SRAM data structure
typedef struct
{
    UWORD magic;                           // Magic number for validation
    UBYTE platform_patterns[TOTAL_BLOCKS]; // Platform patterns
    UBYTE enemy_positions[MAX_ENEMIES];    // Enemy positions
    UBYTE enemy_directions;                // Enemy directions
    UBYTE enemy_types;                     // Enemy types
    UBYTE player_column;                   // Player starting column
    UBYTE checksum;                        // Simple checksum
} sram_level_code_t;

// ============================================================================
// PERSISTENCE FUNCTIONS
// ============================================================================

// System initialization
void init_level_persistence(void) BANKED;

// Character conversion utilities
UBYTE char_to_value(UBYTE display_char) BANKED;

// Variable-based persistence (recommended for simple persistence)
void save_level_code_to_variables(void) BANKED;
void load_level_code_from_variables(void) BANKED;

// SRAM-based persistence (for more complex persistence)
void save_level_code_to_sram(void) BANKED;
UBYTE load_level_code_from_sram(void) BANKED;
UBYTE calculate_level_code_checksum(sram_level_code_t *data) BANKED;

// VM wrapper functions
void vm_has_saved_level_code(SCRIPT_CTX *THIS) BANKED;

// Advanced character editing
void vm_cycle_character(SCRIPT_CTX *THIS) OLDCALL BANKED;
void vm_cycle_character_reverse(SCRIPT_CTX *THIS) OLDCALL BANKED;
void update_level_code_from_character_edit(UBYTE char_index, UBYTE new_value) BANKED;

// Helper function to update display with numeric value
void update_display_with_value(UBYTE char_index, UBYTE value, UBYTE x, UBYTE y) BANKED;

// ============================================================================
// STRING-BASED LEVEL CODE PERSISTENCE
// ============================================================================

// String-based level code functions
void generate_level_code_string(UBYTE level_code_chars[24]) BANKED;
void save_level_code_string_to_variables(void) BANKED;
void load_level_code_string_from_variables(void) BANKED;
void apply_level_code_string(UBYTE level_code_chars[24]) BANKED;

// Individual character management
void set_level_code_character(UBYTE char_index, UBYTE value) BANKED;
UBYTE get_level_code_character(UBYTE char_index) BANKED;
UBYTE has_saved_level_code_string(void) BANKED;
void clear_level_code_string(void) BANKED;

// Predefined level system
void load_predefined_level(UBYTE level_index) BANKED;
UBYTE get_predefined_level_count(void) BANKED;

// VM wrapper functions for string-based level codes
void vm_save_level_code_string(SCRIPT_CTX *THIS) BANKED;
void vm_load_level_code_string(SCRIPT_CTX *THIS) BANKED;
void vm_has_saved_level_code_string(SCRIPT_CTX *THIS) BANKED;
void vm_clear_level_code_string(SCRIPT_CTX *THIS) BANKED;
void vm_set_level_code_character(SCRIPT_CTX *THIS) BANKED;
void vm_get_level_code_character(SCRIPT_CTX *THIS) BANKED;
void vm_load_predefined_level(SCRIPT_CTX *THIS) BANKED;
void vm_get_predefined_level_count(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// SIMPLE MEMORY-BASED RESTORE SYSTEM
// ============================================================================

// Simple memory-based restore system
void restore_level_from_memory(void) BANKED;
void vm_restore_level_from_memory(SCRIPT_CTX *THIS) BANKED;

// ============================================================================
// STRING-BASED LEVEL CODE CONFIGURATION
// ============================================================================

// IMPORTANT: Update this value to match your GB Studio project
// This should be the ID of the first variable you allocate for level code storage
// The system will use 24 consecutive variables starting from this ID
#define VAR_LEVEL_CODE_CHAR_BASE 50  // Change this to match your GB Studio variables

#endif // CODE_PERSISTENCE_H
