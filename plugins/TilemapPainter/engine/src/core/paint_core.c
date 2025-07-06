#pragma bank 254

#include <gbdk/platform.h>
#include "paint_core.h"
#include "paint_platform.h"
#include "paint_entity.h"
#include "paint_ui.h"
#include "meta_tiles.h"
#include "tile_utils.h"
#include "code_gen.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"

// External data references
extern UBYTE __at(SRAM_MAP_DATA_PTR) sram_map_data[];

// External functions used by paint operations
extern void reconstruct_tilemap_from_level_code(void) BANKED;
extern void mark_display_position_for_update(UBYTE position) BANKED;
extern void display_complete_level_code(void) BANKED;
extern void display_selective_level_code_fast(void) BANKED;
extern UBYTE extract_chunk_pattern(UBYTE x, UBYTE y) BANKED;
extern UBYTE match_platform_pattern(UBYTE pattern) BANKED;
extern UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED;
extern void extract_player_data(void) BANKED;
extern void extract_enemy_data(void) BANKED;
extern void extract_platform_data(void) BANKED;
extern void update_valid_enemy_positions(void) BANKED;
extern UBYTE is_block_suppressed(UBYTE zone_index) BANKED;
extern UBYTE get_suppress_display_updates(void) BANKED;
extern void force_complete_level_code_display(void) BANKED;
extern void init_enemy_system(void) BANKED;
extern void update_valid_player_positions(void) BANKED;
extern void init_column_platform_tracking(void) BANKED;
extern void update_complete_level_code(void) BANKED;

// External reference to level code structure
extern level_code_t current_level_code;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Inline utility for repeated boundary checks
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y) BANKED
{
    return (x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
            y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX);
}

// Cached tile access to reduce repeated lookups
UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED
{
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
}

// ============================================================================
// MAIN PAINT FUNCTION
// ============================================================================

void paint(UBYTE x, UBYTE y) BANKED
{
    // Player placement on row 11
    if (y == 11)
    {
        paint_player(x, y);
        return;
    }

    // Cache current tile type to avoid repeated lookups
    UBYTE current_tile_type = get_current_tile_type(x, y);

    // Enemy state transitions: right -> left -> delete -> new position
    if (current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        paint_enemy_left(x, y);
        return;
    }
    else if (current_tile_type == BRUSH_TILE_ENEMY_L)
    {
        delete_enemy(x, y);
        return;
    }

    // New enemy placement - cycle through valid positions if needed
    // Only try to place an enemy if in a valid row for enemies
    if (y == 12 || y == 14 || y == 16 || y == 18)
    {
        paint_enemy_right(x, y);
        // Don't return here - continue to check for platform placement
    }

    // Early exit for invalid platform areas
    if (!is_within_platform_bounds(x, y) || !is_valid_platform_row(y))
    {
        return;
    }

    // Platform deletion
    if (current_tile_type == BRUSH_TILE_PLATFORM)
    {
        remove_enemies_above_platform(x, y);
        replace_meta_tile(x, y, TILE_EMPTY, 1);
        rebuild_platform_row(y);

        // Update player position tracking when platform is deleted
        update_column_platform_deleted(x, y);

        update_level_code_for_paint(x, y); // Smart update
        return;
    }

    // Platform placement - early exit conditions
    if (current_tile_type != BRUSH_TILE_EMPTY ||
        check_platform_vertical_conflict(x, y))
    {
        return;
    }

    // Cache adjacent tile types
    UBYTE left = (x > PLATFORM_X_MIN) ? get_current_tile_type(x - 1, y) : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_current_tile_type(x + 1, y) : BRUSH_TILE_EMPTY;

    if (left == BRUSH_TILE_PLATFORM || right == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return; // Cannot exceed 8-tile platform limit
        }

        // Connect to existing platform
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);

        // Update player position tracking when platform is painted
        update_column_platform_painted(x, y);
    }
    else if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
             !check_platform_vertical_conflict(x + 1, y))
    {
        // Check if creating a 2-tile platform would exceed limits after auto-merge
        if (would_2tile_platform_exceed_limit(x, y))
        {
            return; // Cannot create 2-tile platform - would exceed limit after merge
        }

        // Create new 2-tile platform
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);

        // Update player position tracking when platforms are painted
        update_column_platform_painted(x, y);
        update_column_platform_painted(x + 1, y);
    }
    else
    {
        return; // Can't place platform
    }
    rebuild_platform_row(y);
    update_level_code_for_paint(x, y); // Smart update
}

// ============================================================================
// SMART UPDATE FUNCTIONS
// ============================================================================

void update_level_code_for_paint(UBYTE x, UBYTE y) BANKED
{
    // For player painting (row 11), update enemy/player data
    if (y == 11)
    {
        // Extract player data
        extract_player_data();

        // Update valid enemy positions as player position affects enemy validity
        update_valid_enemy_positions();

        // Mark enemy/player data positions for update (positions 16-23)
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
        return;
    }

    // For enemy operations, update enemy data
    UBYTE current_tile_type = get_current_tile_type(x, y);
    if (current_tile_type == BRUSH_TILE_ENEMY_L || current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        // For any enemy-related operation, extract enemy data and update display
        extract_enemy_data();
        extract_player_data(); // Player data might have changed too

        // Mark enemy/player data positions for update (positions 16-23)
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
        return;
    }

    // For platform operations, update valid enemy positions
    if (current_tile_type == BRUSH_TILE_PLATFORM ||
        y == 13 || y == 15 || y == 17 || y == 19) // Platform rows
    {
        // Update platform data
        extract_platform_data();

        // Update valid enemy positions - platforms affect where enemies can be placed
        update_valid_enemy_positions();

        // Update enemy data in case enemies were removed when deleting platforms
        extract_enemy_data();

        // Mark platform and enemy data for update
        for (UBYTE i = 0; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
        return;
    }

    // For platform operations, update affected zones
    if (y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX &&
        x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
        is_valid_platform_row(y))
    {
        // Check if pattern updates are suppressed (during programmatic painting)
        if (get_suppress_display_updates())
        {
            return; // Skip pattern updates during programmatic painting
        }

        // Platform operations can affect multiple zones on the same row due to auto-completion
        // Update all zones on this row to be safe
        UBYTE row_index = (y - PLATFORM_Y_MIN) / SEGMENT_HEIGHT;

        for (UBYTE col = 0; col < SEGMENTS_PER_ROW; col++)
        {
            UBYTE zone_index = row_index * SEGMENTS_PER_ROW + col;

            // CRITICAL: Check if this specific block is suppressed
            if (is_block_suppressed(zone_index))
                continue;

            // Extract the current pattern for this zone
            UBYTE segment_x = 2 + col * SEGMENT_WIDTH;
            UBYTE segment_y = PLATFORM_Y_MIN + row_index * SEGMENT_HEIGHT;

            // Get the pattern and update the level code
            UBYTE pattern = extract_chunk_pattern(segment_x, segment_y);
            current_level_code.platform_patterns[zone_index] = match_platform_pattern(pattern);

            // Mark this position for display update
            mark_display_position_for_update(zone_index);
        }

        // Use fast selective update (only if display updates are not suppressed)
        if (!get_suppress_display_updates())
        {
            display_selective_level_code_fast();
        }
        return;
    }

    // Check if display updates are globally suppressed (during pattern application)
    if (get_suppress_display_updates())
    {
        return; // Skip display updates during pattern application
    }

    // Fallback to complete update for other cases
    display_complete_level_code();
}

// ============================================================================
// DEFAULT LEVEL INITIALIZATION
// ============================================================================

// Check if the map is empty (no platforms, player, or enemies)
UBYTE is_map_empty(void) BANKED
{
    // Check for any platform tiles
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            if (get_current_tile_type(x, y) == BRUSH_TILE_PLATFORM)
            {
                return 0; // Found a platform, map is not empty
            }
        }
    }

    // Check for player tile
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_current_tile_type(x, 11) == BRUSH_TILE_PLAYER)
        {
            return 0; // Found player, map is not empty
        }
    }

    // Check for enemy tiles
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            UBYTE tile_type = get_current_tile_type(x, y);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                return 0; // Found enemy, map is not empty
            }
        }
    }

    return 1; // Map is empty
}

// Initialize with default level code: "000000000120000090000000"
// This creates a 2-tile platform in the middle with player positioned above it
void init_default_level_code(void) BANKED
{
    // Clear all platform patterns
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = 0;
    }

    // Set pattern 1 at position 9 and pattern 2 at position 10
    // Block 9 covers columns 5-9, pattern 1 places platform at position 4 (column 9)
    // Block 10 covers columns 10-14, pattern 2 places platform at position 3 (column 13)
    // This creates two separate single platforms - let's use pattern 3 instead for a continuous platform
    current_level_code.platform_patterns[9] = 3; // Two platforms at positions 3-4 (columns 8-9)

    // Set player column to 8 (above the first platform tile)
    current_level_code.player_column = 8;

    // Clear enemy data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    // Apply the patterns to the tilemap
    reconstruct_tilemap_from_level_code();

    // Place the player
    UBYTE player_x = current_level_code.player_column + 2; // Convert to tile coordinates
    replace_meta_tile(player_x, 11, TILE_PLAYER, 1);
    move_player_actor_to_tile(paint_player_id, player_x, 11);

    // Position the exit sprite
    position_exit_for_player(player_x, 11);

    // Update the level code display
    force_complete_level_code_display();
}
