#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_player_system.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "tile_utils.h"
#include "paint.h"

// ============================================================================
// PLAYER SYSTEM DATA
// ============================================================================

// Valid player position tracking
UBYTE valid_player_columns[20];
UBYTE valid_player_count = 0;

// ============================================================================
// PLAYER DATA EXTRACTION
// ============================================================================

void extract_player_data(void) BANKED
{
    for (UBYTE col = 2; col < 22; col++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, 11)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_PLAYER)
        {
            current_level_code.player_column = col - 2; // 0-based column
            return;
        }
    }
    current_level_code.player_column = 0; // Default to column 0
}

// ============================================================================
// VALID POSITION MANAGEMENT
// ============================================================================

// Update the list of valid player positions based on current platform patterns
void update_valid_player_positions(void) BANKED
{
    valid_player_count = 0;

    // Check each column (0-19) to see if it has a platform below
    for (UBYTE col = 0; col < 20; col++)
    {
        UBYTE has_platform = 0;

        // Check all 4 rows for platforms in this column
        for (UBYTE row = 0; row < 4; row++)
        {
            UBYTE block_index = row * SEGMENTS_PER_ROW + (col / SEGMENT_WIDTH);

            // Make sure we don't go out of bounds
            if (block_index >= TOTAL_BLOCKS)
                continue;

            UBYTE pattern_id = current_level_code.platform_patterns[block_index];
            if (pattern_id >= PLATFORM_PATTERN_COUNT)
                continue;

            UWORD pattern = PLATFORM_PATTERNS[pattern_id];

            // Calculate position within the 5-tile segment
            UBYTE pos_in_segment = col % SEGMENT_WIDTH;

            // Check both rows of the segment (top row: bits 9-5, bottom row: bits 4-0)
            UBYTE top_row_mask = 1 << (9 - pos_in_segment);
            UBYTE bottom_row_mask = 1 << (4 - pos_in_segment);

            if ((pattern & top_row_mask) || (pattern & bottom_row_mask))
            {
                has_platform = 1;
                break;
            }
        }

        if (has_platform && valid_player_count < 20)
        {
            valid_player_columns[valid_player_count] = col;
            valid_player_count++;
        }
    }

    // Ensure we always have at least one valid position (column 0)
    if (valid_player_count == 0)
    {
        valid_player_columns[0] = 0;
        valid_player_count = 1;
    }
}

// Check if a column is a valid player position
UBYTE is_valid_player_position(UBYTE column) BANKED
{
    for (UBYTE i = 0; i < valid_player_count; i++)
    {
        if (valid_player_columns[i] == column)
        {
            return 1;
        }
    }
    return 0;
}

// Get the next valid player position after the current one (with wraparound)
UBYTE get_next_valid_player_position(UBYTE current_position) BANKED
{
    if (valid_player_count == 0)
        return 0;

    // Find current position in the valid list
    for (UBYTE i = 0; i < valid_player_count; i++)
    {
        if (valid_player_columns[i] == current_position)
        {
            // Return next position (with wraparound)
            return valid_player_columns[(i + 1) % valid_player_count];
        }
    }

    // Current position not found in valid list, return first valid position
    return valid_player_columns[0];
}

// ============================================================================
// POSITION UTILITY FUNCTIONS
// ============================================================================

// Position player at a valid location if current position is invalid
void position_player_at_valid_location(void) BANKED
{
    if (valid_player_count > 0 && !is_valid_player_position(current_level_code.player_column))
    {
        current_level_code.player_column = valid_player_columns[0];
    }
}

// Update exit position when platforms change
void update_exit_position_after_platform_change(void) BANKED
{
    // Get current player position in tile coordinates
    UBYTE player_x = current_level_code.player_column + 2; // Convert to tile coordinates
    UBYTE player_y = 11;                                   // Player is always on row 11

    // Check if player is still in a valid position after platform changes
    if (!is_valid_player_position(current_level_code.player_column))
    {
        // Player is no longer in a valid position, move to first valid position
        if (valid_player_count > 0)
        {
            current_level_code.player_column = valid_player_columns[0];
            player_x = current_level_code.player_column + 2;

            // Update the player's visual position on the tilemap
            clear_existing_player_on_row_11();
            replace_meta_tile(player_x, player_y, TILE_PLAYER, 1);
            move_player_actor_to_tile(paint_player_id, player_x, player_y);

            // Mark player position for display update
            mark_display_position_for_update(16);
        }
    }

    // Reposition the exit based on the current player position
    position_exit_for_player(player_x, player_y);
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

// Debug function to display current valid player positions
void debug_show_valid_positions(void) BANKED
{
    update_valid_player_positions();

    // This could be used to display valid positions on screen or store in variables
    // for script access. For now, just ensure the function works.
}

// Test function to verify valid player position system works correctly
void test_valid_player_positions(void) BANKED
{
    // Set up a test pattern with platforms only in certain columns
    init_level_code();

    // Set pattern 1 (single platform at position 4) in block 0
    // This should make column 4 valid
    current_level_code.platform_patterns[0] = 1;

    // Set pattern 7 (three platforms at positions 2-4) in block 1
    // This should make columns 7, 8, 9 valid (block 1 covers columns 5-9)
    current_level_code.platform_patterns[1] = 7;

    // Update valid positions based on these patterns
    update_valid_player_positions();

    // The valid positions should now be: 4, 7, 8, 9
    // Test cycling through them
    UBYTE current_pos = 4;
    UBYTE next_pos = get_next_valid_player_position(current_pos);
    // next_pos should be 7

    next_pos = get_next_valid_player_position(next_pos);
    // next_pos should be 8

    next_pos = get_next_valid_player_position(next_pos);
    // next_pos should be 9

    next_pos = get_next_valid_player_position(next_pos);
    // next_pos should wrap back to 4
}

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_test_valid_player_positions(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    test_valid_player_positions();
}

// VM wrapper for debugging valid positions
void vm_debug_show_valid_positions(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    debug_show_valid_positions();

    // Store count and first few valid positions in script variables for debugging
    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = valid_player_count;
    if (valid_player_count > 0)
    {
        *(UWORD *)VM_REF_TO_PTR(FN_ARG1) = valid_player_columns[0];
        if (valid_player_count > 1)
            *(UWORD *)VM_REF_TO_PTR(FN_ARG2) = valid_player_columns[1];
        if (valid_player_count > 2)
            *(UWORD *)VM_REF_TO_PTR(FN_ARG3) = valid_player_columns[2];
    }
}

// ============================================================================
// CHARACTER EDITING HANDLERS
// ============================================================================

// Handle player position edit from level code
void handle_player_position_edit(UBYTE new_value) BANKED
{
    // Update player position when level code changes
    if (new_value < 20) // Valid column range 0-19
    {
        current_level_code.player_column = new_value;

        // Update the tilemap to reflect the new player position
        // Clear old player position
        for (UBYTE col = 2; col < 22; col++)
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, 11)];
            UBYTE tile_type = get_tile_type(tile);
            if (tile_type == BRUSH_TILE_PLAYER)
            {
                replace_meta_tile(col, 11, 0, 1); // Clear old position
            }
        }

        // Set new player position
        UBYTE new_col = 2 + new_value; // Convert to tilemap coordinate
        // Use appropriate player tile - assuming there's a player tile constant
        replace_meta_tile(new_col, 11, 1, 1); // Place player at new position
    }
}
