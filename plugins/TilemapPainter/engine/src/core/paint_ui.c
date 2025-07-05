#pragma bank 254

#include <gbdk/platform.h>
#include "paint.h"
#include "paint_ui.h"
#include "paint_core.h"
#include "paint_platform.h"
#include "paint_entity.h"
#include "meta_tiles.h"
#include "tile_utils.h"

// ============================================================================
// BRUSH STATE FUNCTIONS
// ============================================================================

/**
 * @brief Get brush tile state for a position
 *
 * This function determines what UI state should be shown for the brush/selector
 * at the given position, based on surrounding tiles and constraints.
 *
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @return UBYTE State identifier for the brush
 */
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED
{
    // Cache current tile type to avoid repeated lookups
    UBYTE current_tile_type = get_current_tile_type(x, y);

    switch (current_tile_type)
    {
    case BRUSH_TILE_EMPTY:
        if (y == 11)
        {
            return can_paint_player(x, y) ? SELECTOR_STATE_PLAYER : SELECTOR_STATE_DEFAULT;
        }
        if (can_paint_enemy_right(x, y))
        {
            return SELECTOR_STATE_ENEMY_RIGHT;
        }
        return get_platform_placement_type(x, y);

    case BRUSH_TILE_PLATFORM:
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_PLAYER:
        return (y == 11) ? SELECTOR_STATE_DEFAULT : SELECTOR_STATE_PLAYER;

    case BRUSH_TILE_ENEMY_L:
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_ENEMY_R:
        return SELECTOR_STATE_ENEMY_LEFT;

    case BRUSH_TILE_EXIT:
    default:
        return SELECTOR_STATE_DEFAULT;
    }
}
