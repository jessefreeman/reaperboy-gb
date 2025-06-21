#ifndef CODE_GEN_H
#define CODE_GEN_H

#include <gbdk/platform.h>
#include "vm.h"

extern UBYTE current_code[20];

UBYTE extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UBYTE match_platform_pattern(UBYTE row0) BANKED;

void update_code_at_chunk(UBYTE chunk_x, UBYTE chunk_y, UBYTE chunk_index) BANKED;
void display_code_tile(UBYTE pattern_index, UBYTE i) BANKED;
void vm_update_code(SCRIPT_CTX *THIS) BANKED;
void draw_segment_ids() BANKED;
void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED;

#endif
