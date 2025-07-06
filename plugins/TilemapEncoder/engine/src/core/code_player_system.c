#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_player_system.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "tile_utils.h"
#include "paint.h"
#include "paint_entity.h"

// ============================================================================
// PLAYER SYSTEM DATA
// ============================================================================

// External data declarations for cross-bank access
extern const UBYTE PLATFORM_PATTERNS[];
extern UBYTE paint_player_id;

// Valid player position tracking - paint-system tied approach
UBYTE column_has_platform[20];  // 1 if column has at least one platform, 0 if not
UBYTE valid_player_columns[20]; // Cached list of valid columns for cycling
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
// VALID POSITION MANAGEMENT - PAINT SYSTEM INTEGRATION
// ============================================================================

// Initialize the column platform tracking
void init_column_platform_tracking(void) BANKED
{
    // Clear all tracking arrays
    for (UBYTE col = 0; col < 20; col++)
    {
        column_has_platform[col] = 0;
    }
    valid_player_count = 0;

    // Scan current level for existing platforms and populate the tracking
    refresh_column_platform_tracking();
}

// Refresh the entire column tracking by scanning current level
void refresh_column_platform_tracking(void) BANKED
{
    // Clear existing tracking
    for (UBYTE col = 0; col < 20; col++)
    {
        column_has_platform[col] = 0;
    }

    // Scan ALL rows and columns (2-21 in tilemap, 0-19 in level code) for ANY platform tiles
    for (UBYTE row = 0; row < 24; row++) // Check all possible rows
    {
        for (UBYTE col = 2; col < 22; col++)
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, row)];
            UBYTE tile_type = get_tile_type(tile);

            if (tile_type == BRUSH_TILE_PLATFORM)
            {
                UBYTE level_col = col - 2; // Convert to 0-19 range
                if (level_col < 20)
                {
                    column_has_platform[level_col] = 1; // Mark this column as having a platform
                }
            }
        }
    }

    // Rebuild the valid player positions list
    rebuild_valid_player_list();
}

// Rebuild the cached list of valid player positions for cycling
void rebuild_valid_player_list(void) BANKED
{
    valid_player_count = 0;

    for (UBYTE col = 0; col < 20; col++)
    {
        if (column_has_platform[col])
        {
            valid_player_columns[valid_player_count] = col;
            valid_player_count++;
        }
    }

    // Ensure we always have at least one valid position (column 0)
    if (valid_player_count == 0)
    {
        column_has_platform[0] = 1; // Force column 0 to be valid
        valid_player_columns[0] = 0;
        valid_player_count = 1;
    }
}

// Update column tracking when a platform is painted at a specific position
void update_column_platform_painted(UBYTE tilemap_col, UBYTE tilemap_row) BANKED
{
    (void)tilemap_row; // Suppress unused parameter warning

    // Convert to level column (0-19 range)
    UBYTE level_col = tilemap_col - 2;
    if (level_col < 20)
    {
        // Simply mark this column as having a platform (regardless of row)
        if (!column_has_platform[level_col])
        {
            column_has_platform[level_col] = 1;
            rebuild_valid_player_list();
        }
    }
}

// Update column tracking when a platform is deleted from a specific position
void update_column_platform_deleted(UBYTE tilemap_col, UBYTE tilemap_row) BANKED
{
    (void)tilemap_row; // Suppress unused parameter warning

    // Convert to level column (0-19 range)
    UBYTE level_col = tilemap_col - 2;
    if (level_col < 20)
    {
        // Check if this column still has ANY platforms after deletion (scan all rows)
        UBYTE still_has_platform = 0;
        for (UBYTE row = 0; row < 24; row++) // Check all possible rows
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(tilemap_col, row)];
            UBYTE tile_type = get_tile_type(tile);
            if (tile_type == BRUSH_TILE_PLATFORM)
            {
                still_has_platform = 1;
                break;
            }
        }

        // If no platforms remain in this column, mark it as invalid
        if (!still_has_platform && column_has_platform[level_col])
        {
            column_has_platform[level_col] = 0;
            rebuild_valid_player_list();
        }
    }
}

// Legacy function for compatibility - now just calls refresh
void update_valid_player_positions(void) BANKED
{
    refresh_column_platform_tracking();
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
    // Ensure we have valid positions to work with
    if (valid_player_count == 0)
    {
        // Force refresh if no valid positions
        refresh_column_platform_tracking();
        if (valid_player_count == 0)
            return 0;
    }

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

// Get the previous valid player position before the current one (with wraparound)
UBYTE get_previous_valid_player_position(UBYTE current_position) BANKED
{
    // Ensure we have valid positions to work with
    if (valid_player_count == 0)
    {
        // Force refresh if no valid positions
        refresh_column_platform_tracking();
        if (valid_player_count == 0)
            return 0;
    }

    // Find current position in the valid list
    for (UBYTE i = 0; i < valid_player_count; i++)
    {
        if (valid_player_columns[i] == current_position)
        {
            // Return previous position (with wraparound)
            if (i == 0)
                return valid_player_columns[valid_player_count - 1]; // Wrap to last
            else
                return valid_player_columns[i - 1];
        }
    }

    // Current position not found in valid list, return last valid position
    return valid_player_columns[valid_player_count - 1];
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
// CHARACTER EDITING HANDLERS
// ============================================================================

// Handle player position edit from level code
void handle_player_position_edit(UBYTE new_value) BANKED
{
    // Update player position when level code changes
    if (new_value < 20) // Valid column range 0-19
    {
        // Check if the new position is valid (has a platform)
        if (is_valid_player_position(new_value))
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
        // If new position is not valid, don't change the player position
    }
}

// ============================================================================
// PLAYER ACTOR MOVEMENT
// ============================================================================

// Move player actor to specified column and row (for gameplay)
void move_player_to_column(UBYTE x, UBYTE y) BANKED
{
    // Move the player actor to the specified tile position
    move_player_actor_to_tile(paint_player_id, x, y);
}
