#ifndef TILE_UTILS_H
#define TILE_UTILS_H

#include <gbdk/platform.h>

// Metatile IDs
#define TILE_EMPTY             0
#define TILE_PLATFORM_LEFT     4
#define TILE_PLATFORM_MIDDLE   5
#define TILE_PLATFORM_RIGHT    6
#define TILE_PLAYER            20
#define TILE_RIGHT_ENEMY       21
#define TILE_LEFT_ENEMY        22
#define TILE_EXIT_TOP_LEFT     16
#define TILE_EXIT_TOP_RIGHT    17
#define TILE_EXIT_BOTTOM_LEFT  32
#define TILE_EXIT_BOTTOM_RIGHT 33
#define TILE_0                 48

UBYTE get_tile_type(UBYTE tile_id) BANKED;

#endif
