#pragma bank 254

#include "tile_utils.h"

UBYTE get_tile_type(UBYTE tile_id) BANKED
{
    switch (tile_id)
    {
    case TILE_PLATFORM_LEFT:
    case TILE_PLATFORM_MIDDLE:
    case TILE_PLATFORM_RIGHT:
        return BRUSH_TILE_PLATFORM;
    case TILE_RIGHT_ENEMY:
        return BRUSH_TILE_ENEMY_R;
    case TILE_LEFT_ENEMY:
        return BRUSH_TILE_ENEMY_L;
    case TILE_EXIT_TOP_LEFT:
    case TILE_EXIT_TOP_RIGHT:
    case TILE_EXIT_BOTTOM_LEFT:
    case TILE_EXIT_BOTTOM_RIGHT:
        return BRUSH_TILE_EXIT;
    case TILE_PLAYER:
        return BRUSH_TILE_PLAYER;
    case TILE_EMPTY:
        return BRUSH_TILE_EMPTY;
    default:
        return BRUSH_TILE_OTHER;
    }
}
