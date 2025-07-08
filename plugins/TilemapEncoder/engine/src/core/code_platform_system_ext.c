#pragma bank 253

#include <gbdk/platform.h>
#include "code_platform_system.h"
#include "code_level_core.h"
#include "tile_utils.h"
#include "paint.h"
#include "vm.h"
#include "meta_tiles.h"

// External declarations for meta tile system
extern UBYTE __at(SRAM_MAP_DATA_PTR) sram_map_data[];
extern UBYTE image_tile_width_bit;

// Define missing macro locally
#ifndef METATILE_MAP_OFFSET
#define METATILE_MAP_OFFSET(x, y) ((y << image_tile_width_bit) + x)
#endif

// Forward declarations of functions defined in this file
void apply_pattern_with_brush_logic_ext(UBYTE block_index, UBYTE pattern_id) BANKED;
void apply_pattern_with_endcaps_ext(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED;
void apply_row_platforms_ext(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED;
void place_platform_run_ext(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED;

// External declarations from main platform system
extern UBYTE has_adjacent_platform(UBYTE block_index, BYTE direction) BANKED;
extern UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED;
extern void paint(UBYTE x, UBYTE y) BANKED;
extern void replace_meta_tile(UBYTE x, UBYTE y, UBYTE tile_type, UBYTE update_display) BANKED;
extern void update_neighboring_block_codes(UBYTE block_index) BANKED;
extern void update_single_block_code(UBYTE block_index) BANKED;
extern void mark_display_position_for_update(UBYTE position) BANKED;
extern void display_selective_level_code_fast(void) BANKED;
extern void validate_all_block_patterns(void) BANKED;
extern void update_valid_player_positions(void) BANKED;
extern void rebuild_platform_row(UBYTE y) BANKED;

// Forward declarations for functions in this file
void extract_platform_data_ext(void) BANKED;
UBYTE extract_chunk_pattern_ext(UBYTE x, UBYTE y) BANKED;
UBYTE match_platform_pattern_ext(UBYTE pattern) BANKED;
UBYTE is_pattern_valid_for_position_ext(UBYTE pattern_id, UBYTE block_x) BANKED;
UBYTE get_next_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED;
UBYTE get_previous_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED;
void suppress_code_updates_for_block_ext(UBYTE block_index) BANKED;
void enable_code_updates_for_block_ext(UBYTE block_index) BANKED;
UBYTE is_block_suppressed_ext(UBYTE block_index) BANKED;
void clear_all_suppression_ext(void) BANKED;
void set_suppress_display_updates_ext(UBYTE suppress) BANKED;
UBYTE get_suppress_display_updates_ext(void) BANKED;

// ============================================================================
// PLATFORM PATTERN DATA (MOVED FROM BANK 254)
// ============================================================================

// Simplified 5-bit patterns (bottom 5 bits from original patterns)
const UBYTE PLATFORM_PATTERNS[] = {
    0b00000, // UID 0: Empty
    0b00001, // UID 1: Single platform at position 4
    0b10000, // UID 2: Single platform at position 3
    0b00011, // UID 3: Two platforms at positions 3-4
    0b11000, // UID 4: Two platforms at positions 2-3
    0b00110, // UID 5: Two platforms at positions 2-3
    0b01100, // UID 6: Two platforms at positions 1-2
    0b00111, // UID 7: Three platforms at positions 2-4
    0b11100, // UID 8: Three platforms at positions 1-3
    0b01101, // UID 9: Gapped platforms at positions 1-2,4
    0b10110, // UID 10: Gapped platforms at positions 1,3-4
    0b01110, // UID 11: Three platforms at positions 1-3
    0b01111, // UID 12: Four platforms at positions 1-4
    0b11110, // UID 13: Four platforms at positions 1-4
    0b10001, // UID 14: Two isolated platforms at positions 0,4
    0b10011, // UID 15: Three platforms at positions 0,3-4
    0b11001, // UID 16: Three platforms at positions 0,2-3
    0b10111, // UID 17: Four platforms at positions 0,2-4
    0b11101, // UID 18: Four platforms at positions 0-2,4
    0b11011, // UID 19: Four platforms at positions 0-1,3-4
    0b11111  // UID 20: Full platform coverage
};

// Character mapping for pattern display (0-9, A-K for patterns 0-20)
const UBYTE PATTERN_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,    // 0-9
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68 // A-K
};

// Extended character mapping for 0-34 range (supports column encoding)
const UBYTE EXTENDED_PATTERN_TILE_MAP[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, // 0-9
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // A-J (10-19)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // K-T (20-29)
    78, 79, 80, 81, 82                      // U-Y (30-34)
};

// ============================================================================
// PATTERN VALIDATION DATA (MOVED FROM BANK 254)
// ============================================================================

// Invalid patterns for first column (block_x = 0) - patterns that have platforms at leftmost position
// These patterns have their leftmost bit (position 0) set: 2, 10, 14, 15, 17 (in hex: 2, A, E, F, H)
const UBYTE INVALID_PATTERNS_FIRST_COLUMN[] = {2, 10, 14, 15, 17};
#define INVALID_PATTERNS_FIRST_COLUMN_COUNT 5

// Invalid patterns for last column (block_x = 3) - patterns that have platforms at rightmost position
// These patterns have their rightmost bit (position 4) set: 1, 9, 14, 16, 18 (in hex: 1, 9, E, G, I)
const UBYTE INVALID_PATTERNS_LAST_COLUMN[] = {1, 9, 14, 16, 18};
#define INVALID_PATTERNS_LAST_COLUMN_COUNT 5

// ============================================================================
// CORE PATTERN EXTRACTION AND MATCHING (MOVED FROM BANK 254)
// ============================================================================

// Simplified pattern extraction for single-row platforms
UBYTE extract_chunk_pattern_ext(UBYTE x, UBYTE y) BANKED
{
    UBYTE pattern = 0;

    // Since platforms are only rendered on the second row of each segment (odd rows),
    // we only need to check y+1 (the platform row) of the segment
    UBYTE platform_y = y + 1;

    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(x + i, platform_y)];

        if (tile == PLATFORM_TILE_1 || tile == PLATFORM_TILE_2 || tile == PLATFORM_TILE_3)
        {
            pattern |= (1 << (4 - i)); // Set bit for this position (bit 4 = position 0, bit 0 = position 4)
        }
    }

    return pattern;
}

UBYTE match_platform_pattern_ext(UBYTE pattern) BANKED
{
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        if (PLATFORM_PATTERNS[i] == pattern)
        {
            return i;
        }
    }
    return 0; // Fallback to pattern 0
}

// ============================================================================
// SURGICAL SUPPRESSION SYSTEM DATA (MOVED FROM BANK 254)
// ============================================================================

// Block-specific suppression flags (16 blocks = 16 bits in 2 bytes)
UWORD suppressed_blocks = 0;

// Global flag to suppress all display updates during pattern application
UBYTE suppress_display_updates = 0;

// ============================================================================
// PATTERN VALIDATION FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Fast validation using direct array lookup (optimized for fixed system)
UBYTE is_pattern_valid_for_position_ext(UBYTE pattern_id, UBYTE block_x) BANKED
{
    // Special handling for Pattern E/14 (platforms at both leftmost and rightmost positions)
    if (pattern_id == 14) {
        // Pattern E needs both left and right neighbors to work
        // Only valid in the middle columns (block_x = 1 or 2)
        return (block_x > 0 && block_x < (SEGMENTS_PER_ROW - 1)) ? 1 : 0;
    }
    
    // Special handling for Pattern 1 (single platform at position 4)
    if (pattern_id == 1) {
        // Pattern 1 is only invalid in the last column (block_x=3) where it would spill over with no neighbor
        if (block_x == 3) {
            return 0;
        }
        // Valid everywhere else
        return 1;
    }

    // Special handling for Pattern 2 (single platform at position 0)
    if (pattern_id == 2) {
        // Pattern 2 is only invalid in the first column (block_x=0) where it would spill over with no neighbor
        if (block_x == 0) {
            return 0;
        }
        // Valid everywhere else
        return 1;
    }
    
    // For the leftmost column (block_x=0), we restrict patterns with the leftmost bit set
    // that aren't covered by the special cases above
    if (block_x == 0) {
        // Patterns with leftmost bit set: 10, 15, 17
        // Convert pattern IDs to hex values: A=10, F=15, H=17
        if (pattern_id == 10 || pattern_id == 15 || pattern_id == 17) {
            return 0;
        }
    }
    
    // For the rightmost column (block_x=3), we restrict patterns with the rightmost bit set
    // that aren't covered by the special cases above
    if (block_x == 3) {
        // Patterns with rightmost bit set: 9, 16, 18
        // Convert pattern IDs to hex values: 9=9, G=16, I=18
        if (pattern_id == 9 || pattern_id == 16 || pattern_id == 18) {
            return 0;
        }
    }
    
    // All other patterns are valid in all positions
    return 1;
}

// Get next valid pattern (optimized for fixed system)
UBYTE get_next_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED
{
    UBYTE next_pattern = current_pattern;

    // Find next valid pattern (max 21 iterations)
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        next_pattern++;
        if (next_pattern >= PLATFORM_PATTERN_COUNT)
            next_pattern = 0; // Wrap around

        if (is_pattern_valid_for_position_ext(next_pattern, block_x))
        {
            return next_pattern;
        }
    }

    // Fallback (should never happen)
    return 0;
}

// Get previous valid pattern (optimized for fixed system)
UBYTE get_previous_valid_pattern_ext(UBYTE current_pattern, UBYTE block_x) BANKED
{
    UBYTE prev_pattern = current_pattern;

    // Find previous valid pattern (max 21 iterations)
    for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++)
    {
        if (prev_pattern == 0)
            prev_pattern = PLATFORM_PATTERN_COUNT - 1; // Wrap around
        else
            prev_pattern--;

        if (is_pattern_valid_for_position_ext(prev_pattern, block_x))
        {
            return prev_pattern;
        }
    }

    // Fallback (should never happen)
    return 0;
}

// ============================================================================
// SURGICAL SUPPRESSION SYSTEM FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Suppress code updates for a specific block during painting
void suppress_code_updates_for_block_ext(UBYTE block_index) BANKED
{
    if (block_index < TOTAL_BLOCKS)
    {
        suppressed_blocks |= (1 << block_index);
    }
}

// Re-enable code updates for a specific block
void enable_code_updates_for_block_ext(UBYTE block_index) BANKED
{
    if (block_index < TOTAL_BLOCKS)
    {
        suppressed_blocks &= ~(1 << block_index);
    }
}

// Check if code updates are suppressed for a specific block
UBYTE is_block_suppressed_ext(UBYTE block_index) BANKED
{
    if (block_index >= TOTAL_BLOCKS)
        return 0;
    return (suppressed_blocks & (1 << block_index)) != 0;
}

// Clear all suppression flags
void clear_all_suppression_ext(void) BANKED
{
    suppressed_blocks = 0;
    suppress_display_updates = 0;
}

// Global display suppression control
void set_suppress_display_updates_ext(UBYTE suppress) BANKED
{
    suppress_display_updates = suppress;
}

UBYTE get_suppress_display_updates_ext(void) BANKED
{
    return suppress_display_updates;
}

// ============================================================================
// PLATFORM DATA EXTRACTION (MOVED FROM BANK 254)
// ============================================================================

void extract_platform_data_ext(void) BANKED
{
    for (UBYTE block_y = 0; block_y < 4; block_y++)
    {
        for (UBYTE block_x = 0; block_x < 4; block_x++)
        {
            UBYTE block_index = block_y * 4 + block_x;
            UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
            UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

            UBYTE pattern = extract_chunk_pattern_ext(segment_x, segment_y);
            UBYTE pattern_id = match_platform_pattern_ext(pattern);

            current_level_code.platform_patterns[block_index] = pattern_id;
        }
    }

    // Update valid player positions after extracting platform patterns
    update_valid_player_positions();
}

// ============================================================================
// LEVEL CODE INTEGRATION FUNCTIONS (BANK 253)
// ============================================================================

// Convert character index (0-15) to block position for pattern validation
UBYTE get_block_x_from_char_index(UBYTE char_index) BANKED
{
    if (char_index >= TOTAL_BLOCKS)
        return 255; // Invalid

    return char_index % SEGMENTS_PER_ROW; // Returns 0-3 for columns
}

// Get next valid pattern for level code editing (character-index based)
UBYTE get_next_valid_pattern_for_char(UBYTE char_index, UBYTE current_pattern) BANKED
{
    if (char_index >= TOTAL_BLOCKS)
        return current_pattern; // Invalid character index

    UBYTE block_x = get_block_x_from_char_index(char_index);
    return get_next_valid_pattern_ext(current_pattern, block_x);
}

// Get previous valid pattern for level code editing (character-index based)
UBYTE get_previous_valid_pattern_for_char(UBYTE char_index, UBYTE current_pattern) BANKED
{
    if (char_index >= TOTAL_BLOCKS)
        return current_pattern; // Invalid character index

    UBYTE block_x = get_block_x_from_char_index(char_index);
    return get_previous_valid_pattern_ext(current_pattern, block_x);
}

// Check if pattern is valid for a specific character index (0-15)
UBYTE is_pattern_valid_for_char_index(UBYTE char_index, UBYTE pattern_id) BANKED
{
    if (char_index >= TOTAL_BLOCKS)
        return 0; // Invalid character index

    UBYTE block_x = get_block_x_from_char_index(char_index);
    return is_pattern_valid_for_position_ext(pattern_id, block_x);
}

// ============================================================================
// PLATFORM RECONSTRUCTION FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Reconstruct the entire tilemap from the current level code using brush logic
void reconstruct_tilemap_from_level_code_ext(void) BANKED
{
    // Apply each block's pattern using brush logic
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        apply_pattern_with_brush_logic_ext(i, current_level_code.platform_patterns[i]);
    }

    // Final validation pass to ensure everything is consistent
    validate_all_block_patterns();
}

// Apply a pattern with full validation and race condition prevention
void apply_valid_pattern_to_block_ext(UBYTE block_index, UBYTE pattern_id) BANKED
{
    if (block_index >= TOTAL_BLOCKS || pattern_id >= PLATFORM_PATTERN_COUNT)
    {
        return;
    }

    UBYTE block_x = block_index % SEGMENTS_PER_ROW;

    // If pattern is invalid for this position, auto-correct to next valid pattern
    if (!is_pattern_valid_for_position_ext(pattern_id, block_x))
    {
        pattern_id = get_next_valid_pattern_ext(pattern_id, block_x);
    }

    // CRITICAL: Suppress ALL display updates during the entire operation
    set_suppress_display_updates_ext(1);

    // Also suppress code updates for this block during painting
    suppress_code_updates_for_block_ext(block_index);

    // Handle special patterns that require cross-block connections
    // Pattern 1 = single platform at position 4 (rightmost)
    // Pattern 2 = single platform at position 0 (leftmost)
    // Pattern E/14 = platforms at positions 0 and 4 (both leftmost and rightmost)
    
    // For tracking left neighbor updates
    UBYTE need_to_update_left = 0;
    UBYTE left_neighbor_index = 0;
    UBYTE left_neighbor_pattern = 0;
    
    // For tracking right neighbor updates
    UBYTE need_to_update_right = 0;
    UBYTE right_neighbor_index = 0;
    UBYTE right_neighbor_pattern = 0;

    // Check for patterns with rightmost platform (position 4) - Patterns 1, 9, 14, 16, 18
    // These patterns need to connect to the right neighbor's leftmost position
    if ((pattern_id == 1 || pattern_id == 9 || pattern_id == 14 || pattern_id == 16 || 
         pattern_id == 18) && block_x < (SEGMENTS_PER_ROW - 1)) 
    {
        right_neighbor_index = block_index + 1;
        UBYTE current_neighbor_pattern = current_level_code.platform_patterns[right_neighbor_index];
        
        // We need to set leftmost bit in right neighbor's pattern
        UBYTE neighbor_bits = PLATFORM_PATTERNS[current_neighbor_pattern];
        neighbor_bits |= 0b10000; // Set leftmost bit
        
        // Find matching pattern for modified bits
        for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++) 
        {
            if (PLATFORM_PATTERNS[i] == neighbor_bits) 
            {
                right_neighbor_pattern = i;
                need_to_update_right = 1;
                break;
            }
        }
    }
    
    // Check for patterns with leftmost platform (position 0) - Patterns 2, 10, 14, 15, 17
    // These patterns need to connect to the left neighbor's rightmost position
    if ((pattern_id == 2 || pattern_id == 10 || pattern_id == 14 || pattern_id == 15 || 
         pattern_id == 17) && block_x > 0) 
    {
        left_neighbor_index = block_index - 1;
        UBYTE current_neighbor_pattern = current_level_code.platform_patterns[left_neighbor_index];
        
        // We need to set rightmost bit in left neighbor's pattern
        UBYTE neighbor_bits = PLATFORM_PATTERNS[current_neighbor_pattern];
        neighbor_bits |= 0b00001; // Set rightmost bit
        
        // Find matching pattern for modified bits
        for (UBYTE i = 0; i < PLATFORM_PATTERN_COUNT; i++) 
        {
            if (PLATFORM_PATTERNS[i] == neighbor_bits) 
            {
                left_neighbor_pattern = i;
                need_to_update_left = 1;
                break;
            }
        }
    }

    // Set the intended pattern in level code FIRST (so display shows correct pattern immediately)
    current_level_code.platform_patterns[block_index] = pattern_id;

    // Apply the pattern using brush logic (the tilemap changes will happen)
    apply_pattern_with_brush_logic_ext(block_index, pattern_id);

    // Update left neighbor if needed
    if (need_to_update_left) 
    {
        // Suppress code updates for the left neighbor
        suppress_code_updates_for_block_ext(left_neighbor_index);
        
        // Update left neighbor's pattern
        current_level_code.platform_patterns[left_neighbor_index] = left_neighbor_pattern;
        
        // Apply the new pattern to the left neighbor
        apply_pattern_with_brush_logic_ext(left_neighbor_index, left_neighbor_pattern);
        
        // Re-enable code updates for the left neighbor
        enable_code_updates_for_block_ext(left_neighbor_index);
        
        // Mark the left neighbor for display update
        mark_display_position_for_update(left_neighbor_index);
    }
    
    // Update right neighbor if needed
    if (need_to_update_right) 
    {
        // Suppress code updates for the right neighbor
        suppress_code_updates_for_block_ext(right_neighbor_index);
        
        // Update right neighbor's pattern
        current_level_code.platform_patterns[right_neighbor_index] = right_neighbor_pattern;
        
        // Apply the new pattern to the right neighbor
        apply_pattern_with_brush_logic_ext(right_neighbor_index, right_neighbor_pattern);
        
        // Re-enable code updates for the right neighbor
        enable_code_updates_for_block_ext(right_neighbor_index);
        
        // Mark the right neighbor for display update
        mark_display_position_for_update(right_neighbor_index);
    }

    // Re-enable code updates for this block
    enable_code_updates_for_block_ext(block_index);

    // Update neighbors after our block is complete (they should not be suppressed)
    update_neighboring_block_codes(block_index);

    // Re-enable display updates and force a single update at the end
    set_suppress_display_updates_ext(0);

    // Mark this position for display update to ensure it shows the correct pattern
    mark_display_position_for_update(block_index);

    // Trigger a single display update now that everything is complete
    display_selective_level_code_fast();
}

// Increment to next valid pattern for a block (handles edge cases automatically)
UBYTE increment_block_pattern_ext(UBYTE block_index) BANKED
{
    if (block_index >= TOTAL_BLOCKS)
        return 0;

    UBYTE current_pattern = current_level_code.platform_patterns[block_index];
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;

    // Get next valid pattern (automatically skips invalid ones)
    UBYTE new_pattern = get_next_valid_pattern_ext(current_pattern, block_x);

    // Apply the new pattern
    apply_valid_pattern_to_block_ext(block_index, new_pattern);

    return new_pattern;
}

// Decrement to previous valid pattern for a block (handles edge cases automatically)
UBYTE decrement_block_pattern_ext(UBYTE block_index) BANKED
{
    if (block_index >= TOTAL_BLOCKS)
        return 0;

    UBYTE current_pattern = current_level_code.platform_patterns[block_index];
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;

    // Get previous valid pattern (automatically skips invalid ones)
    UBYTE new_pattern = get_previous_valid_pattern_ext(current_pattern, block_x);

    // Apply the new pattern
    apply_valid_pattern_to_block_ext(block_index, new_pattern);

    return new_pattern;
}

// TILEMAP APPLICATION FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Apply pattern changes to the actual tilemap
void apply_pattern_to_tilemap_ext(UBYTE block_index, UBYTE pattern_id) BANKED
{
    if (block_index >= TOTAL_BLOCKS || pattern_id >= PLATFORM_PATTERN_COUNT)
    {
        return;
    }

    // Calculate block position in tilemap
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;
    UBYTE block_y = block_index / SEGMENTS_PER_ROW;
    UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

    // Get the pattern data
    UBYTE pattern = PLATFORM_PATTERNS[pattern_id];

    // Clear the segment first
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
    {
        for (UBYTE j = 0; j < SEGMENT_HEIGHT; j++)
        {
            replace_meta_tile(segment_x + i, segment_y + j, 0, 1);
        }
    }

    // Apply the pattern with proper end cap logic
    apply_pattern_with_endcaps_ext(segment_x, segment_y, pattern, block_index);
}

// Apply pattern with proper end cap logic considering neighboring segments
void apply_pattern_with_endcaps_ext(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED
{
    // This function applies a pattern with proper platform end caps
    // considering neighboring blocks for seamless platform connections

    UBYTE has_left_neighbor = has_adjacent_platform(block_index, -1);
    UBYTE has_right_neighbor = has_adjacent_platform(block_index, +1);

    // Since platforms are only on the second row (y+1) of each segment, apply only to that row
    UBYTE platform_y = segment_y + 1; // The actual platform row (odd row)

    apply_row_platforms_ext(segment_x, platform_y, pattern, has_left_neighbor, has_right_neighbor);
}

// Apply patterns using the same brush logic as manual painting
void apply_pattern_with_brush_logic_ext(UBYTE block_index, UBYTE pattern_id) BANKED
{
    if (block_index >= TOTAL_BLOCKS || pattern_id >= PLATFORM_PATTERN_COUNT)
    {
        return;
    }

    // Calculate block position in tilemap
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;
    UBYTE block_y = block_index / SEGMENTS_PER_ROW;
    UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

    // Get the pattern data
    UBYTE pattern = PLATFORM_PATTERNS[pattern_id];

    // The platform row is the second row of each segment (odd row)
    UBYTE platform_y = segment_y + 1;

    // First, clear any existing platform tiles in this segment
    // Use direct tile replacement to avoid level code interference
    for (UBYTE row = 0; row < SEGMENT_HEIGHT; row++)
    {
        UBYTE current_y = segment_y + row;
        for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
        {
            UBYTE tile_x = segment_x + i;
            UBYTE current_tile = get_current_tile_type(tile_x, current_y);
            if (current_tile == BRUSH_TILE_PLATFORM)
            {
                // Direct tile replacement - no level code updates
                replace_meta_tile(tile_x, current_y, TILE_EMPTY, 1);
            }
        }
    }

    // Place platforms using direct tile replacement (paint system approach)
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
    {
        if (pattern & (1 << (4 - i))) // Check if this position should have a platform
        {
            UBYTE tile_x = segment_x + i;

            // Only place if the tile is currently empty
            if (get_current_tile_type(tile_x, platform_y) == BRUSH_TILE_EMPTY)
            {
                // Use TILE_PLATFORM_MIDDLE initially - rebuild_platform_row will fix end caps
                replace_meta_tile(tile_x, platform_y, TILE_PLATFORM_MIDDLE, 1);
            }
        }
    }
    
    // Special case handling for cross-block platforms
    // Check for rightmost platform (position 4) that needs to connect to right neighbor
    // Patterns with rightmost bit: 1, 9, 14, 16, 18
    if ((pattern_id == 1 || pattern_id == 9 || pattern_id == 14 || pattern_id == 16 || 
         pattern_id == 18) && block_x < (SEGMENTS_PER_ROW - 1)) 
    {
        // Make sure next block has a platform at position 0
        UBYTE next_block_x = segment_x + SEGMENT_WIDTH;
        UBYTE next_block_y = platform_y;
        
        // Check if the next block already has a platform at position 0
        UBYTE neighbor_tile = get_current_tile_type(next_block_x, next_block_y);
        if (neighbor_tile != BRUSH_TILE_PLATFORM)
        {
            // Add a platform at position 0 of the next block
            replace_meta_tile(next_block_x, next_block_y, TILE_PLATFORM_MIDDLE, 1);
        }
    }
    
    // Check for leftmost platform (position 0) that needs to connect to left neighbor
    // Patterns with leftmost bit: 2, 10, 14, 15, 17
    if ((pattern_id == 2 || pattern_id == 10 || pattern_id == 14 || pattern_id == 15 || 
         pattern_id == 17) && block_x > 0) 
    {
        // Make sure previous block has a platform at position 4
        UBYTE prev_block_x = segment_x - 1;
        UBYTE prev_block_y = platform_y;
        
        // Check if the previous block already has a platform at position 4
        UBYTE neighbor_tile = get_current_tile_type(prev_block_x, prev_block_y);
        if (neighbor_tile != BRUSH_TILE_PLATFORM)
        {
            // Add a platform at position 4 of the previous block
            replace_meta_tile(prev_block_x, prev_block_y, TILE_PLATFORM_MIDDLE, 1);
        }
    }

    // Now use the paint system's platform row rebuilding logic to ensure proper end caps
    // This is the key function that handles all the end cap logic correctly
    rebuild_platform_row(platform_y);

    // NOTE: No need to call update_single_block_code here since the parent function handles it
    // and suppression may still be active
}

// BLOCK MANAGEMENT FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Comprehensive update of all blocks that might have been affected by auto-completion
void update_all_affected_block_codes_ext(UBYTE block_index) BANKED
{
    // First, update immediate neighbors (left, right, above, below)
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

    // Above neighbor
    if (current_row > 0)
    {
        UBYTE above_neighbor = block_index - SEGMENTS_PER_ROW;
        update_single_block_code(above_neighbor);
    }

    // Below neighbor
    if (current_row < 3) // 4 total rows (0-3)
    {
        UBYTE below_neighbor = block_index + SEGMENTS_PER_ROW;
        update_single_block_code(below_neighbor);
    }

    // For horizontal auto-completion effects, we might need to check extended neighbors
    // Auto-completion can create 2-tile platforms that span across block boundaries
    // Check 2 blocks to the left and right for potential cascade effects

    // Extended left neighbor (2 blocks away)
    if ((block_index % SEGMENTS_PER_ROW) >= 2)
    {
        UBYTE extended_left = block_index - 2;
        update_single_block_code(extended_left);
    }

    // Extended right neighbor (2 blocks away)
    if ((block_index % SEGMENTS_PER_ROW) <= (SEGMENTS_PER_ROW - 3))
    {
        UBYTE extended_right = block_index + 2;
        update_single_block_code(extended_right);
    }
}

// Validate that the pattern displayed matches what was actually applied
void validate_final_pattern_match_ext(UBYTE block_index, UBYTE intended_pattern_id) BANKED
{
    if (block_index >= TOTAL_BLOCKS)
        return;

    // Extract the actual pattern from the tilemap
    UBYTE block_x = block_index % SEGMENTS_PER_ROW;
    UBYTE block_y = block_index / SEGMENTS_PER_ROW;
    UBYTE segment_x = 2 + block_x * SEGMENT_WIDTH;
    UBYTE segment_y = PLATFORM_Y_MIN + block_y * SEGMENT_HEIGHT;

    UBYTE actual_pattern = extract_chunk_pattern_ext(segment_x, segment_y);
    UBYTE actual_pattern_id = match_platform_pattern_ext(actual_pattern);

    // Update the level code to reflect what was actually painted
    current_level_code.platform_patterns[block_index] = actual_pattern_id;

    // If the actual pattern doesn't match the intended pattern,
    // it means the painting system modified it (due to length limits, etc.)
    // This is normal behavior, so we just ensure the display shows the correct pattern
    if (actual_pattern_id != intended_pattern_id)
    {
        // Mark this position for display update to show the correct pattern
        mark_display_position_for_update(block_index);
    }
}

// PLATFORM ROW APPLICATION FUNCTIONS (MOVED FROM BANK 254)
// ============================================================================

// Apply platforms for a single row with proper end cap logic
void apply_row_platforms_ext(UBYTE base_x, UBYTE y, UBYTE row_pattern, UBYTE has_left_neighbor, UBYTE has_right_neighbor) BANKED
{
    if (row_pattern == 0)
        return;

    // Find platform runs in the pattern
    UBYTE start_pos = 255;
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
    {
        UBYTE has_platform = (row_pattern & (1 << (4 - i))) != 0;

        if (has_platform && start_pos == 255)
        {
            start_pos = i; // Start of a run
        }
        else if (!has_platform && start_pos != 255)
        {
            // End of a run
            UBYTE length = i - start_pos;
            UBYTE connected_left = (start_pos == 0) ? has_left_neighbor : 0;
            UBYTE connected_right = 0; // Not connected right since we found a gap

            place_platform_run_ext(base_x + start_pos, y, length, connected_left, connected_right);
            start_pos = 255;
        }
    }

    // Handle run that extends to the end
    if (start_pos != 255)
    {
        UBYTE length = SEGMENT_WIDTH - start_pos;
        UBYTE connected_left = (start_pos == 0) ? has_left_neighbor : 0;
        UBYTE connected_right = has_right_neighbor;

        place_platform_run_ext(base_x + start_pos, y, length, connected_left, connected_right);
    }
}

// Place a run of platforms with proper end caps
void place_platform_run_ext(UBYTE start_x, UBYTE y, UBYTE length, UBYTE connected_left, UBYTE connected_right) BANKED
{
    if (length == 0)
        return;

    for (UBYTE i = 0; i < length; i++)
    {
        UBYTE tile_type;

        if (length == 1)
        {
            // Single platform tile - use appropriate end caps based on connections
            if (connected_left && connected_right)
                tile_type = PLATFORM_TILE_2; // Middle piece
            else if (connected_left)
                tile_type = PLATFORM_TILE_3; // Right end cap
            else if (connected_right)
                tile_type = PLATFORM_TILE_1; // Left end cap
            else
                tile_type = PLATFORM_TILE_2; // Isolated single tile
        }
        else if (i == 0)
        {
            // First tile in run
            tile_type = connected_left ? PLATFORM_TILE_2 : PLATFORM_TILE_1;
        }
        else if (i == length - 1)
        {
            // Last tile in run
            tile_type = connected_right ? PLATFORM_TILE_2 : PLATFORM_TILE_3;
        }
        else
        {
            // Middle tile
            tile_type = PLATFORM_TILE_2;
        }

        replace_meta_tile(start_x + i, y, tile_type, 1);
    }
}
