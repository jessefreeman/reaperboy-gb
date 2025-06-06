// paint.h
#ifndef PAINT_H
#define PAINT_H

// Metatile IDs

#define TILE_EMPTY           0
#define TILE_PLATFORM_LEFT   4
#define TILE_PLATFORM_MIDDLE 5
#define TILE_PLATFORM_RIGHT  6

#define TILE_PLAYER          20
#define TILE_ENEMY           21

#define TILE_EXIT_TOP_LEFT     16
#define TILE_EXIT_TOP_RIGHT    17
#define TILE_EXIT_BOTTOM_LEFT  32
#define TILE_EXIT_BOTTOM_RIGHT 33

#include <gbdk/platform.h>
#include "vm.h"  // for SCRIPT_CTX and script_get_arg8

// called by _callNative("paint")
void vm_paint(SCRIPT_CTX *THIS) BANKED;

// (optional) pure‐C version
void paint(UBYTE x, UBYTE y) BANKED;

// New “get_brush_tile” API (all in paint.c):
void vm_get_brush_tile(SCRIPT_CTX *THIS) BANKED;
UBYTE get_brush_tile(UBYTE x, UBYTE y) BANKED;

#endif
