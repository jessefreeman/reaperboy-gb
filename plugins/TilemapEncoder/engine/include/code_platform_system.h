#ifndef CODE_PLATFORM_SYSTEM_H
#define CODE_PLATFORM_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// PLATFORM PATTERN DATA AND CONSTANTS
// ============================================================================

// Platform patterns array (simplified to 5-bit patterns for single-row platforms)
extern const UBYTE PLATFORM_PATTERNS[];
extern const UBYTE PATTERN_TILE_MAP[];
extern const UBYTE EXTENDED_PATTERN_TILE_MAP[];
extern const UBYTE PATTERN_NEIGHBOR_UPDATE_FLAGS[];

// Pattern validation arrays
extern const UBYTE INVALID_PATTERNS_FIRST_COLUMN[];
extern const UBYTE INVALID_PATTERNS_LAST_COLUMN[];
#define INVALID_PATTERNS_FIRST_COLUMN_COUNT 5
#define INVALID_PATTERNS_LAST_COLUMN_COUNT 5

#define PLATFORM_PATTERN_COUNT 21

// ============================================================================
// PLATFORM SYSTEM FUNCTIONS
// ============================================================================

// System initialization
void init_platform_system(void) BANKED;
void init_test_platform_patterns(void) BANKED;

// Core pattern extraction and matching (simplified for single-row platforms)
UBYTE extract_chunk_pattern(UBYTE x, UBYTE y) BANKED;
UBYTE match_platform_pattern(UBYTE pattern) BANKED;

// Platform data extraction
void extract_platform_data(void) BANKED;
void update_all_platform_patterns(void) BANKED;

// Tilemap application functions
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED;

// Block management functions
void update_single_block_code(UBYTE block_index) BANKED;
void update_neighboring_block_codes(UBYTE block_index) BANKED;
void update_all_affected_block_codes(UBYTE block_index) BANKED;
void validate_final_pattern_match(UBYTE block_index, UBYTE intended_pattern_id) BANKED;
void validate_all_block_patterns(void) BANKED;

// Platform reconstruction
void apply_row_platforms(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED;
void reconstruct_tilemap_from_level_code(void) BANKED;
void place_platform_run(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED;
UBYTE has_adjacent_platform(UBYTE block_index, BYTE direction) BANKED;

// Surgical suppression system (prevents race conditions)
void suppress_code_updates_for_block(UBYTE block_index) BANKED;
void enable_code_updates_for_block(UBYTE block_index) BANKED;
UBYTE is_block_suppressed(UBYTE block_index) BANKED;
void clear_all_suppression(void) BANKED;

// Global display suppression control (prevents flicker during pattern application)
void set_suppress_display_updates(UBYTE suppress) BANKED;
UBYTE get_suppress_display_updates(void) BANKED;

// Pattern validation for position-specific constraints
UBYTE is_pattern_valid_for_position(UBYTE pattern_id, UBYTE block_x) BANKED;
UBYTE get_next_valid_pattern(UBYTE current_pattern, UBYTE block_x) BANKED;
UBYTE get_previous_valid_pattern(UBYTE current_pattern, UBYTE block_x) BANKED;

// Comprehensive pattern application (solves validation + race conditions)
void apply_valid_pattern_to_block(UBYTE block_index, UBYTE pattern_id) BANKED;
UBYTE increment_block_pattern(UBYTE block_index) BANKED;
UBYTE decrement_block_pattern(UBYTE block_index) BANKED;

// Banked function wrappers for VM system access (for functions defined in platform system only)
void b_init_platform_system(void) BANKED;
void b_init_test_platform_patterns(void) BANKED;
void b_update_all_platform_patterns(void) BANKED;
void b_init_level_persistence(void) BANKED;

#endif // CODE_PLATFORM_SYSTEM_H
