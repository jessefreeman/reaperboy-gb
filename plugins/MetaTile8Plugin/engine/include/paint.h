// paint.h
#ifndef PAINT_H
#define PAINT_H

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
