#pragma bank 254

#include "tile_utils.h"

UBYTE get_tile_type(UBYTE tile_id) BANKED {
    switch (tile_id) {
        case TILE_PLATFORM_LEFT:
        case TILE_PLATFORM_MIDDLE:
        case TILE_PLATFORM_RIGHT:
            return 1;
        case TILE_ENEMY:
            return 2;
        case TILE_EXIT_TOP_LEFT:
        case TILE_EXIT_TOP_RIGHT:
        case TILE_EXIT_BOTTOM_LEFT:
        case TILE_EXIT_BOTTOM_RIGHT:
            return 3;
        case TILE_PLAYER:
            return 4;
        default:
            return 5;
    }
}
