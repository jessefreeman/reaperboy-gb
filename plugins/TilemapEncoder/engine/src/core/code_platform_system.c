#pragma bank 251

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_platform_system.h"
#include "code_platform_system_ext.h"
#include "code_level_core.h"
#include "code_player_system.h"
#include "tile_utils.h"
#include "paint.h"

// External function declarations
extern void display_selective_level_code_fast(void) BANKED;
extern void mark_display_position_for_update(UBYTE position) BANKED;

// External data declarations from extension bank
extern const UBYTE PLATFORM_PATTERNS[];
extern const UBYTE PATTERN_TILE_MAP[];
extern const UBYTE EXTENDED_PATTERN_TILE_MAP[];

// ============================================================================
// PLATFORM PATTERN DATA
// ============================================================================

// Platform pattern data moved to extension bank to save space

// ============================================================================
// CORE PATTERN EXTRACTION AND MATCHING
// ============================================================================

// Core pattern extraction and matching moved to extension bank to save space

// Simplified pattern extraction for single-row platforms
UBYTE extract_chunk_pattern(UBYTE x, UBYTE y) BANKED
{
    // Use extension function to save bank space
    return extract_chunk_pattern_ext(x, y);
}

UBYTE match_platform_pattern(UBYTE pattern) BANKED
{
    // Use extension function to save bank space
    return match_platform_pattern_ext(pattern);
}

// ============================================================================
// PLATFORM DATA EXTRACTION
// ============================================================================

void extract_platform_data(void) BANKED
{
    // Use extension function to save bank space
    extract_platform_data_ext();
}

// ============================================================================
// TILEMAP APPLICATION FUNCTIONS
// ============================================================================

// Apply pattern changes to the actual tilemap
void apply_pattern_to_tilemap(UBYTE block_index, UBYTE pattern_id) BANKED
{
    // Use extension function to save bank space
    apply_pattern_to_tilemap_ext(block_index, pattern_id);
}

// New function that applies patterns using the same brush logic as manual painting
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED
{
    // Use extension function to save bank space
    apply_pattern_with_brush_logic_ext(block_index, pattern_id);
}

// ============================================================================
// BLOCK MANAGEMENT FUNCTIONS
// ============================================================================

// Update the level code for a single block by extracting its current pattern
void update_single_block_code(UBYTE block_index) BANKED
{
    if (block_index >= TOTAL_BLOCKS)
        return;

    // Respect surgical suppression - skip if this block is suppressed
    if (is_block_suppressed(block_index))
        return;

    // Calculate segment position
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;
    UBYTE block_y = block_index / SEGMENTS_PER_ROW;
    UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

    // Extract the current pattern from the tilemap
    UBYTE pattern = extract_chunk_pattern(segment_x, segment_y);
    UBYTE pattern_id = match_platform_pattern(pattern);

    current_level_code.platform_patterns[block_index] = pattern_id;
}

// Update level codes for blocks adjacent to the given block that may have been affected by auto-completion
void update_neighboring_block_codes(UBYTE block_index) BANKED
{
    // Check horizontal neighbors (left and right)
    UBYTE current_row = block_index / SEGMENTS_PER_ROW;

    // Left neighbor
    if ((block_index % SEGMENTS_PER_ROW) > 0)
    {
        UBYTE left_neighbor = block_index - 1;
        update_single_block_code(left_neighbor);
    }

    // Right neighbor
    if ((block_index % SEGMENTS_PER_ROW) < (SEGMENTS_PER_ROW - 1))
    {
        UBYTE right_neighbor = block_index + 1;
        update_single_block_code(right_neighbor);
    }

    // Vertical neighbors (above and below)
    // Above neighbor
    if (current_row > 0)
    {
        UBYTE above_neighbor = block_index - SEGMENTS_PER_ROW;
        update_single_block_code(above_neighbor);
    }

    // Below neighbor
    if (current_row < (TOTAL_BLOCKS / SEGMENTS_PER_ROW - 1))
    {
        UBYTE below_neighbor = block_index + SEGMENTS_PER_ROW;
        update_single_block_code(below_neighbor);
    }
}

// Comprehensive update of all blocks that might have been affected by auto-completion
// This is more thorough than update_neighboring_block_codes and handles cascade effects
void update_all_affected_block_codes(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    update_all_affected_block_codes_ext(block_index);
}

// Validate that the pattern displayed matches what was actually applied
void validate_final_pattern_match(UBYTE block_index, UBYTE intended_pattern_id) BANKED
{
    // Use extension function to save bank space
    validate_final_pattern_match_ext(block_index, intended_pattern_id);
}

// Validate all block patterns to ensure consistency
void validate_all_block_patterns(void) BANKED
{
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        update_single_block_code(i);
    }
}

// ============================================================================
// PLATFORM RECONSTRUCTION AND ADVANCED FUNCTIONS
// ============================================================================

// Reconstruct the entire tilemap from the current level code using brush logic
void reconstruct_tilemap_from_level_code(void) BANKED
{
    // Use extension function to save bank space
    reconstruct_tilemap_from_level_code_ext();
}

// Helper function to check if there's a platform in adjacent segment
UBYTE has_adjacent_platform(UBYTE block_index, BYTE direction) BANKED
{
    // direction: -1 for left, +1 for right
    BYTE adjacent_index = (BYTE)block_index + direction;

    if (adjacent_index < 0 || adjacent_index >= TOTAL_BLOCKS)
        return 0;

    // Check if adjacent block is in the same row
    if ((adjacent_index / SEGMENTS_PER_ROW) != (block_index / SEGMENTS_PER_ROW))
        return 0;

    return current_level_code.platform_patterns[adjacent_index] != 0;
}

// Apply pattern with proper end cap logic considering neighboring segments
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    apply_pattern_with_endcaps_ext(segment_x, segment_y, pattern, block_index);
}

// Apply platforms for a single row with proper end cap logic
void apply_row_platforms(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED
{
    // Use extension function to save bank space
    apply_row_platforms_ext(base_x, y, row_pattern, has_left_neighbor, has_right_neighbor);
}

// Place a run of platforms with proper end caps
void place_platform_run(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED
{
    // Use extension function to save bank space
    place_platform_run_ext(start_x, y, length, connected_left, connected_right);
}

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

// Initialize the platform system
void init_platform_system(void) BANKED
{
    // Platform system initialization if needed
}

// Initialize test platform patterns
void init_test_platform_patterns(void) BANKED
{
    // Test pattern initialization if needed
}

// Update all platform patterns
void update_all_platform_patterns(void) BANKED
{
    // Update all platform patterns
    extract_platform_data();
}

// Initialize level persistence
void init_level_persistence(void) BANKED
{
    // Initialize the level code structure to ensure clean state
    // But don't auto-create any platforms
    init_level_code();
}

// ============================================================================
// END OF PLATFORM SYSTEM
// ============================================================================

// Banked function wrappers for VM system access (for functions defined in platform system only)
void b_init_platform_system(void) BANKED { init_platform_system(); }
void b_init_test_platform_patterns(void) BANKED { init_test_platform_patterns(); }
void b_update_all_platform_patterns(void) BANKED { update_all_platform_patterns(); }
void b_init_level_persistence(void) BANKED { init_level_persistence(); }

// ============================================================================
// PATTERN VALIDATION DATA (OPTIMIZED FOR FIXED SYSTEM)
// ============================================================================

// Pattern validation data moved to extension bank to save space

// ============================================================================
// PATTERN VALIDATION FUNCTIONS (OPTIMIZED)
// ============================================================================

// Fast validation using direct array lookup (optimized for fixed system)
UBYTE is_pattern_valid_for_position(UBYTE pattern_id, UBYTE block_x) BANKED
{
    // Use extension function to save bank space
    return is_pattern_valid_for_position_ext(pattern_id, block_x);
}

// Get next valid pattern (optimized for fixed system)
UBYTE get_next_valid_pattern(UBYTE current_pattern, UBYTE block_x) BANKED
{
    // Use extension function to save bank space
    return get_next_valid_pattern_ext(current_pattern, block_x);
}

// Get previous valid pattern (optimized for fixed system)
UBYTE get_previous_valid_pattern(UBYTE current_pattern, UBYTE block_x) BANKED
{
    // Use extension function to save bank space
    return get_previous_valid_pattern_ext(current_pattern, block_x);
}

// ============================================================================
// SURGICAL SUPPRESSION SYSTEM (PREVENTS RACE CONDITIONS)
// ============================================================================

// Surgical suppression system moved to extension bank to save space

// Suppress code updates for a specific block during painting
void suppress_code_updates_for_block(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    suppress_code_updates_for_block_ext(block_index);
}

// Re-enable code updates for a specific block
void enable_code_updates_for_block(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    enable_code_updates_for_block_ext(block_index);
}

// Check if code updates are suppressed for a specific block
UBYTE is_block_suppressed(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    return is_block_suppressed_ext(block_index);
}

// Clear all suppression flags
void clear_all_suppression(void) BANKED
{
    // Use extension function to save bank space
    clear_all_suppression_ext();
}

// Global display suppression control
void set_suppress_display_updates(UBYTE suppress) BANKED
{
    // Use extension function to save bank space
    set_suppress_display_updates_ext(suppress);
}

UBYTE get_suppress_display_updates(void) BANKED
{
    // Use extension function to save bank space
    return get_suppress_display_updates_ext();
}

// ============================================================================
// COMPREHENSIVE PATTERN APPLICATION (SOLVES RACE CONDITIONS + VALIDATION)
// ============================================================================

// Apply a pattern with full validation and race condition prevention
void apply_valid_pattern_to_block(UBYTE block_index, UBYTE pattern_id) BANKED
{
    // Use extension function to save bank space
    apply_valid_pattern_to_block_ext(block_index, pattern_id);
}

// Increment to next valid pattern for a block (handles edge cases automatically)
UBYTE increment_block_pattern(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    return increment_block_pattern_ext(block_index);
}

// Decrement to previous valid pattern for a block (handles edge cases automatically)
UBYTE decrement_block_pattern(UBYTE block_index) BANKED
{
    // Use extension function to save bank space
    return decrement_block_pattern_ext(block_index);
}

// ============================================================================
