#pragma bank 254

#include <gbdk/platform.h>
#include "paint.h" // ← Make sure your prototypes are visible first
#include "vm.h"
#include "meta_tiles.h"
#include "tile_utils.h"
#include "gbs_types.h"
#include "actor.h"

// your existing #defines…
#define PLATFORM_X_MIN 2
#define PLATFORM_X_MAX 21
#define PLATFORM_Y_MIN 11
#define PLATFORM_Y_MAX 20
#define PLATFORM_MIN_VERTICAL_GAP 1
#define PLATFORM_MAX_LENGTH 8

// pixels → subpixels
#define TO_FP(n) ((INT16)((n) << 4))

/**
 * vm_setup_map
 *   Pops:  player, exit, e1–e6 (in that order)
 *   1) finds & places player on row 10
 *   2) finds first platform below and places exit one tile above
 *   3) scans up to 6 enemy‐tiles and spawns/flips them; deactivates the rest
 */
void vm_setup_map(SCRIPT_CTX *THIS, INT16 idx) OLDCALL BANKED
{
    UBYTE playerId = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE exitId = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE enemies[6] = {
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG2),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG3),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG4),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG5),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG6),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG7)};

    UBYTE ec = 0, playerPlaced = 0, exitPlaced = 0;
    UBYTE playerX = 0, playerRow = 0;

    for (UBYTE yy = 10; yy < 20; ++yy)
    {
        for (UBYTE xx = 2; xx < 22; ++xx)
        {
            UBYTE tid = sram_map_data[METATILE_MAP_OFFSET(xx, yy)];
            UBYTE tt = get_tile_type(tid);

            // place player
            if (!playerPlaced && tt == 5)
            {
                actor_t *p = &actors[playerId];
                p->pos.x = TO_FP(xx * 8);
                p->pos.y = TO_FP(0);
                activate_actor(p);
                replace_meta_tile(xx, yy, TILE_EMPTY, 1);
                playerPlaced = 1;
                playerX = xx;
                playerRow = yy;
                continue;
            }

            // place exit
            if (playerPlaced && !exitPlaced && xx == playerX && yy > playerRow && tt == 1)
            {
                UBYTE placeX = (tid == TILE_PLATFORM_RIGHT) ? xx - 1 : xx;
                actor_t *ex = &actors[exitId];
                ex->pos.x = TO_FP(placeX * 8);
                ex->pos.y = TO_FP((yy - 1) * 8);
                activate_actor(ex);
                replace_meta_tile(placeX, yy - 1, TILE_EMPTY, 1);
                exitPlaced = 1;
            }

            // place enemies
            if (ec < 6 && (tt == 2 || tt == 3))
            {
                actor_t *e = &actors[enemies[ec++]];
                INT16 fx = (tt == 3) ? (xx * 8 - 8) : (xx * 8);
                e->pos.x = TO_FP(fx);
                e->pos.y = TO_FP(yy * 8);
                actor_set_dir(e, tt == 2 ? DIR_RIGHT : DIR_LEFT, TRUE);
                activate_actor(e);
                replace_meta_tile(xx, yy, TILE_EMPTY, 1);
            }

            if (playerPlaced && exitPlaced && ec >= 6)
                break;
        }
        if (playerPlaced && exitPlaced && ec >= 6)
            break;
    }

    for (; ec < 6; ++ec)
        deactivate_actor(&actors[enemies[ec]]);
}

UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED
{
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
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
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, cy)]) == 1)
            return 1;
    }
    return 0;
}

void rebuild_platform_row(UBYTE y) BANKED
{
    UBYTE seq_start = 255, current_len = 0;
    for (UBYTE i = PLATFORM_X_MIN; i <= PLATFORM_X_MAX + 1; ++i)
    {
        UBYTE is_plat = (i <= PLATFORM_X_MAX) && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(i, y)]) == 1;
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
                if (current_len == 1)
                {
                    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
                }
                else
                {
                    for (UBYTE j = 0; j < current_len; ++j)
                    {
                        UBYTE tx = seq_start + j;
                        if (j == 0)
                            replace_meta_tile(tx, y, TILE_PLATFORM_LEFT, 1);
                        else if (tx == PLATFORM_X_MAX || j == current_len - 1)
                            replace_meta_tile(tx, y, TILE_PLATFORM_RIGHT, 1);
                        else
                            replace_meta_tile(tx, y, TILE_PLATFORM_MIDDLE, 1);
                    }
                }
                seq_start = 255;
                current_len = 0;
            }
        }
        else if (seq_start != 255)
        {
            if (current_len == 1)
            {
                replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
            }
            else
            {
                for (UBYTE j = 0; j < current_len; ++j)
                {
                    UBYTE tx = seq_start + j;
                    if (j == 0)
                        replace_meta_tile(tx, y, TILE_PLATFORM_LEFT, 1);
                    else if (tx == PLATFORM_X_MAX || j == current_len - 1)
                        replace_meta_tile(tx, y, TILE_PLATFORM_RIGHT, 1);
                    else
                        replace_meta_tile(tx, y, TILE_PLATFORM_MIDDLE, 1);
                }
            }
            seq_start = 255;
            current_len = 0;
        }
    }
}

void paint(UBYTE x, UBYTE y) BANKED
{
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX ||
        y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
        return;
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != 5)
        return;
    if (check_platform_vertical_conflict(x, y))
        return;
    if (x < PLATFORM_X_MAX && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == 5 && check_platform_vertical_conflict(x + 1, y))
        return;

    UBYTE left = (x > PLATFORM_X_MIN)
                     ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)])
                     : 0;
    UBYTE right = (x < PLATFORM_X_MAX)
                      ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)])
                      : 0;

    if (left == 1 || right == 1)
    {
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
    }
    else if (x < PLATFORM_X_MAX && right == 5)
    {
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    }
    else
    {
        return;
    }
    rebuild_platform_row(y);
}

void delete_tile_at_pos(UBYTE x, UBYTE y, UBYTE commit) BANKED
{
    replace_meta_tile(x, y, TILE_EMPTY, commit);
    if (commit)
        rebuild_platform_row(y);
}

void vm_paint(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint(x, y);
}

void vm_delete_tile_at_pos(SCRIPT_CTX *THIS) OLDCALL BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE c = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    delete_tile_at_pos(x, y, c);
}

void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_pos(x, y);
}

void vm_move_actor_to_test(SCRIPT_CTX *THIS) OLDCALL BANKED
{
    UBYTE ai = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    actor_t *actor = &actors[ai];
    for (UBYTE yy = 10; yy < 20; ++yy)
    {
        for (UBYTE xx = 2; xx < 22; ++xx)
        {
            UBYTE tid = sram_map_data[METATILE_MAP_OFFSET(xx, yy)];
            UBYTE tt = get_tile_type(tid);
            if (tt == 2)
            {
                actor->pos.x = TO_FP(xx * 8);
                actor->pos.y = TO_FP(yy * 8);
                actor_set_dir(actor, DIR_RIGHT, TRUE);
                activate_actor(actor);
                return;
            }
            else if (tt == 3)
            {
                actor->pos.x = TO_FP(xx * 8 - 8);
                actor->pos.y = TO_FP(yy * 8);
                actor_set_dir(actor, DIR_LEFT, TRUE);
                activate_actor(actor);
                return;
            }
        }
    }
}
