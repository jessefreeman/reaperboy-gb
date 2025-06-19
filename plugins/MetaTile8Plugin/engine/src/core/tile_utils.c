#pragma bank 254

#include "tile_utils.h"

UBYTE get_tile_type(UBYTE tile_id) BANKED
{
    switch (tile_id)
    {
    case TILE_PLATFORM_LEFT:
    case TILE_PLATFORM_MIDDLE:
    case TILE_PLATFORM_RIGHT:
        return 1;
    case TILE_RIGHT_ENEMY:
        return 2;
    case TILE_LEFT_ENEMY:
        return 3;
    case TILE_EXIT_TOP_LEFT:
    case TILE_EXIT_TOP_RIGHT:
    case TILE_EXIT_BOTTOM_LEFT:
    case TILE_EXIT_BOTTOM_RIGHT:
        return 4;
    case TILE_PLAYER:
        return 5;
    default:
        return 6;
    }
}
