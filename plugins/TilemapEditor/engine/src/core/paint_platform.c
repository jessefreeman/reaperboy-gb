#pragma bank 254

#include <gbdk/platform.h>
#include "paint_platform.h"
#include "paint_core.h"
#include "paint_entity.h"
#include "paint_ui.h" // Include for SELECTOR_STATE_* constants
#include "meta_tiles.h"
#include "tile_utils.h"
#include "code_level_core.h"

// Forward declarations for platform-related functions
extern void update_column_platform_deleted(UBYTE x, UBYTE y) BANKED;
extern void update_column_platform_painted(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// PLATFORM VALIDATION FUNCTIONS
// ============================================================================

UBYTE is_valid_platform_row(UBYTE y) BANKED
{
    return (y == 13 || y == 15 || y == 17 || y == 19);
}

UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED
{
    for (UBYTE check_y = y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        if (is_valid_platform_row(check_y))
        {
            if (get_current_tile_type(x, check_y) == BRUSH_TILE_PLATFORM)
            {
                return 1;
            }
        }
    }
    return 0;
}

// Strict version for enemy placement that requires the platform to be directly below
UBYTE has_platform_directly_below(UBYTE x, UBYTE y) BANKED
{
    // Find the immediate platform row below this enemy row
    UBYTE platform_row = y + 1; // Platform row is directly below enemy row

    // Verify this is a valid row for a platform
    if (!is_valid_platform_row(platform_row))
        return 0; // Not a valid platform row

    // Check if there's a platform tile directly at this position
    UBYTE tile_type = get_current_tile_type(x, platform_row);

    // Must be exactly a platform tile (not any tile)
    return (tile_type == BRUSH_TILE_PLATFORM);
}

UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y) BANKED
{
    for (BYTE dy = -PLATFORM_MIN_VERTICAL_GAP; dy <= PLATFORM_MIN_VERTICAL_GAP; ++dy)
    {
        if (dy == 0)
            continue;

        BYTE cy = y + dy;
        if (cy < PLATFORM_Y_MIN || cy > PLATFORM_Y_MAX)
            continue;
        if (!is_valid_platform_row(cy))
            continue;

        if (get_current_tile_type(x, cy) == BRUSH_TILE_PLATFORM)
        {
            return 1;
        }
    }
    return 0;
}

UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED
{
    return (is_within_platform_bounds(x, y) &&
            is_valid_platform_row(y) &&
            get_current_tile_type(x, y) == BRUSH_TILE_EMPTY &&
            !check_platform_vertical_conflict(x, y));
}

UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED
{
    if (!can_place_platform(x, y))
        return SELECTOR_STATE_DEFAULT;

    // Cache adjacent tile types
    UBYTE left = (x > PLATFORM_X_MIN) ? get_current_tile_type(x - 1, y) : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_current_tile_type(x + 1, y) : BRUSH_TILE_EMPTY;
    if (left == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return SELECTOR_STATE_DEFAULT; // Cannot place - would exceed platform limit
        }
        return SELECTOR_STATE_PLATFORM_RIGHT;
    }
    if (right == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return SELECTOR_STATE_DEFAULT; // Cannot place - would exceed platform limit
        }
        return SELECTOR_STATE_PLATFORM_LEFT;
    }

    if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY && can_place_platform(x + 1, y))
    {
        // Check if creating a 2-tile platform would exceed limits after auto-merge
        if (would_2tile_platform_exceed_limit(x, y))
        {
            return SELECTOR_STATE_DEFAULT; // Cannot create - would exceed limit after merge
        }
        return SELECTOR_STATE_NEW_PLATFORM;
    }

    return SELECTOR_STATE_DEFAULT;
}

// ============================================================================
// PLATFORM LENGTH CALCULATION
// ============================================================================

UBYTE count_connected_platform_length(UBYTE x, UBYTE y) BANKED
{
    UBYTE length = 1; // Count the tile we would place

    // Count platforms to the left
    UBYTE left_x = x;
    while (left_x > PLATFORM_X_MIN)
    {
        left_x--;
        if (get_current_tile_type(left_x, y) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    // Count platforms to the right
    for (UBYTE right_x = x + 1; right_x <= PLATFORM_X_MAX; right_x++)
    {
        if (get_current_tile_type(right_x, y) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    return length;
}

// Check if placing a 2-tile platform starting at x would exceed limits after auto-merge
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y) BANKED
{
    // Check if there's a platform to the left that would merge
    UBYTE left_length = 0;
    if (x > PLATFORM_X_MIN && get_current_tile_type(x - 1, y) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the left
        UBYTE left_x = x - 1;
        while (left_x >= PLATFORM_X_MIN && get_current_tile_type(left_x, y) == BRUSH_TILE_PLATFORM)
        {
            left_length++;
            if (left_x == PLATFORM_X_MIN)
                break;
            left_x--;
        }
    }

    // Check if there's a platform to the right that would merge
    UBYTE right_length = 0;
    if (x + 2 <= PLATFORM_X_MAX && get_current_tile_type(x + 2, y) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the right
        for (UBYTE right_x = x + 2; right_x <= PLATFORM_X_MAX; right_x++)
        {
            if (get_current_tile_type(right_x, y) == BRUSH_TILE_PLATFORM)
            {
                right_length++;
            }
            else
            {
                break;
            }
        }
    }

    // Total length would be: left platforms + our 2 tiles + right platforms
    UBYTE total_length = left_length + 2 + right_length;
    return total_length > PLATFORM_MAX_LENGTH;
}

// ============================================================================
// PLATFORM RECONSTRUCTION
// ============================================================================

void rebuild_platform_row(UBYTE y) BANKED
{
    UBYTE seq_start = 255, current_len = 0;

    for (UBYTE i = PLATFORM_X_MIN; i <= PLATFORM_X_MAX + 1; ++i)
    {
        UBYTE is_plat = (i <= PLATFORM_X_MAX) &&
                        get_current_tile_type(i, y) == BRUSH_TILE_PLATFORM;

        if (is_plat)
        {
            if (seq_start == 255)
            {
                seq_start = i;
                current_len = 1;
            }
            else
            {
                current_len++;
            }

            if (current_len == PLATFORM_MAX_LENGTH || i == PLATFORM_X_MAX)
            {
                // Finalize sequence
                if (current_len == 1)
                {
                    remove_enemies_above_platform(seq_start, y);
                    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
                }
                else
                {
                    for (UBYTE j = 0; j < current_len; ++j)
                    {
                        UBYTE tx = seq_start + j;
                        UBYTE tile = (j == 0) ? TILE_PLATFORM_LEFT : (j == current_len - 1) ? TILE_PLATFORM_RIGHT
                                                                                            : TILE_PLATFORM_MIDDLE;
                        replace_meta_tile(tx, y, tile, 1);
                    }
                }
                seq_start = 255;
                current_len = 0;
            }
        }
        else if (seq_start != 255)
        {
            // End of sequence
            if (current_len == 1)
            {
                remove_enemies_above_platform(seq_start, y);
                replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
            }
            else
            {
                for (UBYTE j = 0; j < current_len; ++j)
                {
                    UBYTE tx = seq_start + j;
                    UBYTE tile = (j == 0) ? TILE_PLATFORM_LEFT : (j == current_len - 1) ? TILE_PLATFORM_RIGHT
                                                                                        : TILE_PLATFORM_MIDDLE;
                    replace_meta_tile(tx, y, tile, 1);
                }
            }
            seq_start = 255;
            current_len = 0;
        }
    }
}
