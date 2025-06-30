#ifndef CODE_PLATFORM_SYSTEM_H
#define CODE_PLATFORM_SYSTEM_H

#include <gbdk/platform.h>
#include "code_level_core.h"

// ============================================================================
// PLATFORM PATTERN DATA AND CONSTANTS
// ============================================================================

// Platform patterns array
extern const UWORD PLATFORM_PATTERNS[];
extern const UBYTE PATTERN_TILE_MAP[];
extern const UBYTE EXTENDED_PATTERN_TILE_MAP[];

#define PLATFORM_PATTERN_COUNT 21

// ============================================================================
// PLATFORM SYSTEM FUNCTIONS
// ============================================================================

// System initialization
void init_platform_system(void) BANKED;
void init_test_platform_patterns(void) BANKED;

// Core pattern extraction and matching
UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UWORD match_platform_pattern(UWORD pattern) BANKED;

// Platform data extraction
void extract_platform_data(void) BANKED;
void update_all_platform_patterns(void) BANKED;

// Tilemap application functions
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UWORD pattern, UBYTE block_index) BANKED;

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

#endif // CODE_PLATFORM_SYSTEM_H
