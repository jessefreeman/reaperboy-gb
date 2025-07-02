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

#endif // CODE_PERSISTENCE_H
