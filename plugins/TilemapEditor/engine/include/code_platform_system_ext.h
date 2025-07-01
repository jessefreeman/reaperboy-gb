#ifndef CODE_PLATFORM_SYSTEM_EXT_H
#define CODE_PLATFORM_SYSTEM_EXT_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// LEVEL CODE INTEGRATION FUNCTIONS (BANK 253)
// ============================================================================

// Level code integration functions (for character-index based pattern validation)
UBYTE get_block_x_from_char_index(UBYTE char_index) BANKED;
UBYTE get_next_valid_pattern_for_char(UBYTE char_index, UBYTE current_pattern) BANKED;
UBYTE get_previous_valid_pattern_for_char(UBYTE char_index, UBYTE current_pattern) BANKED;
UBYTE is_pattern_valid_for_char_index(UBYTE char_index, UBYTE pattern_id) BANKED;

// ============================================================================
// COMPREHENSIVE PATTERN APPLICATION FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Platform reconstruction
void reconstruct_tilemap_from_level_code_ext(void) BANKED;

// Apply a pattern with full validation and race condition prevention
void apply_valid_pattern_to_block_ext(UBYTE block_index, UBYTE pattern_id) BANKED;

// Increment to next valid pattern for a block (handles edge cases automatically)
UBYTE increment_block_pattern_ext(UBYTE block_index) BANKED;

// Decrement to previous valid pattern for a block (handles edge cases automatically)
UBYTE decrement_block_pattern_ext(UBYTE block_index) BANKED;

// Tilemap application functions (moved from bank 254)
void apply_pattern_to_tilemap_ext(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_endcaps_ext(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED;
void apply_pattern_with_brush_logic_ext(UBYTE block_index, UBYTE pattern_id) BANKED;

// Block management functions (moved from bank 254)
void update_all_affected_block_codes_ext(UBYTE block_index) BANKED;
void validate_final_pattern_match_ext(UBYTE block_index, UBYTE intended_pattern_id) BANKED;

// Platform row application functions (moved from bank 254)
void apply_row_platforms_ext(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED;
void place_platform_run_ext(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED;

// Platform data extraction function (moved from bank 254)
void extract_platform_data_ext(void) BANKED;

// Core pattern extraction and matching functions (moved from bank 254)
UBYTE extract_chunk_pattern_ext(UBYTE x, UBYTE y) BANKED;
UBYTE match_platform_pattern_ext(UBYTE pattern) BANKED;

// Pattern validation functions (moved from bank 254)
UBYTE is_pattern_valid_for_position_ext(UBYTE pattern_id, UBYTE block_x) BANKED;
UBYTE get_next_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED;
UBYTE get_previous_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED;

// Surgical suppression system functions (moved from bank 254)
void suppress_code_updates_for_block_ext(UBYTE block_index) BANKED;
void enable_code_updates_for_block_ext(UBYTE block_index) BANKED;
UBYTE is_block_suppressed_ext(UBYTE block_index) BANKED;
void clear_all_suppression_ext(void) BANKED;
void set_suppress_display_updates_ext(UBYTE suppress) BANKED;
UBYTE get_suppress_display_updates_ext(void) BANKED;

#endif // CODE_PLATFORM_SYSTEM_EXT_H
