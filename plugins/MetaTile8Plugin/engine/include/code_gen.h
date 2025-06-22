#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <gbdk/platform.h>
#include "vm.h"

extern UWORD current_code[20];

UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UWORD match_platform_pattern(UWORD pattern) BANKED;

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED;
void display_code_tile(UWORD pattern_index, UBYTE i) BANKED;
void vm_update_code(SCRIPT_CTX *THIS) BANKED;
void draw_segment_ids(void) BANKED;
void update_zone_code(UBYTE zone_index) BANKED;
void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED;

// Debug functions for pattern testing
void vm_debug_next_pattern(SCRIPT_CTX *THIS) BANKED;
void vm_debug_prev_pattern(SCRIPT_CTX *THIS) BANKED;
void vm_debug_reset_pattern(SCRIPT_CTX *THIS) BANKED;

#endif
