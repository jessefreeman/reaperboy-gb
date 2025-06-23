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
#define PLATFORM_Y_MIN 12 // Changed from 11 to 12
#define PLATFORM_Y_MAX 19 // Changed from 20 to 19
#define PLATFORM_MIN_VERTICAL_GAP 1
#define PLATFORM_MAX_LENGTH 8

// Helper function to check if a Y coordinate is valid for platform placement
// Only rows 13, 15, 17, and 19 are allowed
UBYTE is_valid_platform_row(UBYTE y) BANKED
{
    return (y == 13 || y == 15 || y == 17 || y == 19);
}

// Selector animation state constants
#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_LEFT 2
#define SELECTOR_STATE_ENEMY_RIGHT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLATFORM_CENTER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7
#define SELECTOR_STATE_PLAYER 8

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
    // Suppress unused parameter warning
    (void)idx;

    uint16_t varId = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE aliveEnemyCount = 0;

    // Hard code the player ID to 0
    UBYTE playerId = 0; //*(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
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
    for (UBYTE yy = 10; yy <= 19; ++yy) // Updated range to match new constraints
    {
        for (UBYTE xx = 2; xx < 22; ++xx)
        {
            UBYTE tid = sram_map_data[METATILE_MAP_OFFSET(xx, yy)];
            UBYTE tt = get_tile_type(tid);

            // place player
            if (!playerPlaced && tt == BRUSH_TILE_PLAYER)
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
            } // place exit
            if (playerPlaced && !exitPlaced && xx == playerX && yy > playerRow && tt == BRUSH_TILE_PLATFORM)
            {
                UBYTE placeX = (tid == TILE_PLATFORM_RIGHT) ? xx - 1 : xx;
                actor_t *ex = &actors[exitId];
                ex->pos.x = TO_FP(placeX * 8);
                ex->pos.y = TO_FP((yy - 1) * 8);
                activate_actor(ex);

                replace_meta_tile(placeX, yy - 1, TILE_EXIT_BOTTOM_LEFT, 1);
                replace_meta_tile(placeX + 1, yy - 1, TILE_EXIT_BOTTOM_RIGHT, 1);
                replace_meta_tile(placeX, yy - 2, TILE_EXIT_TOP_LEFT, 1);
                replace_meta_tile(placeX + 1, yy - 2, TILE_EXIT_TOP_RIGHT, 1);
                exitPlaced = 1;
            }

            // place enemies
            if (ec < 6 && (tt == BRUSH_TILE_ENEMY_R || tt == BRUSH_TILE_ENEMY_L))
            {
                actor_t *e = &actors[enemies[ec++]];
                INT16 fx = (tt == BRUSH_TILE_ENEMY_L) ? (xx * 8 - 8) : (xx * 8);
                e->pos.x = TO_FP(fx);
                e->pos.y = TO_FP(yy * 8);
                actor_set_dir(e, tt == BRUSH_TILE_ENEMY_R ? DIR_RIGHT : DIR_LEFT, TRUE);
                activate_actor(e);
                replace_meta_tile(xx, yy, TILE_EMPTY, 1);
                aliveEnemyCount++;
            }

            if (playerPlaced && exitPlaced && ec >= 6)
                break;
        }
        if (playerPlaced && exitPlaced && ec >= 6)
            break;
    }

    for (; ec < 6; ++ec)
        deactivate_actor(&actors[enemies[ec]]);

    script_memory[varId] = aliveEnemyCount;
}

UBYTE get_brush_tile_pos(UBYTE x, UBYTE y) BANKED
{
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
}

UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED
{
    // Check bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX ||
        y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
        return 0;

    // Check if this is a valid row for platform placement
    if (!is_valid_platform_row(y))
        return 0;

    // Must be empty tile
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != BRUSH_TILE_EMPTY)
        return 0;

    // Check vertical conflicts
    if (check_platform_vertical_conflict(x, y))
        return 0;

    return 1;
}

UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED
{
    if (!can_place_platform(x, y))
        return 0;

    UBYTE left = (x > PLATFORM_X_MIN)
                     ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)])
                     : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX)
                      ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)])
                      : BRUSH_TILE_EMPTY;

    // Connecting to existing platform(s)
    if (left == BRUSH_TILE_PLATFORM && right == BRUSH_TILE_PLATFORM)
        return SELECTOR_STATE_PLATFORM_CENTER;
    else if (left == BRUSH_TILE_PLATFORM)
        return SELECTOR_STATE_PLATFORM_RIGHT;
    else if (right == BRUSH_TILE_PLATFORM)
        return SELECTOR_STATE_PLATFORM_LEFT;

    // New platform (need adjacent empty space)
    if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
        can_place_platform(x + 1, y))
        return SELECTOR_STATE_NEW_PLATFORM;

    // Can't place a valid platform
    return 0;
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
        // Only check conflicts on valid platform rows
        if (!is_valid_platform_row(cy))
            continue;
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, cy)]) == BRUSH_TILE_PLATFORM)
            return 1;
    }
    return 0;
}

void rebuild_platform_row(UBYTE y) BANKED
{
    UBYTE seq_start = 255, current_len = 0;
    for (UBYTE i = PLATFORM_X_MIN; i <= PLATFORM_X_MAX + 1; ++i)
    {
        UBYTE is_plat = (i <= PLATFORM_X_MAX) && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(i, y)]) == BRUSH_TILE_PLATFORM;
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
    // Check bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX ||
        y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
        return;

    // Check if this is a valid row for platform placement
    if (!is_valid_platform_row(y))
        return;

    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);

    // If there's already a platform here, delete it
    if (current_tile_type == BRUSH_TILE_PLATFORM)
    {
        replace_meta_tile(x, y, TILE_EMPTY, 1);
        rebuild_platform_row(y);
        return;
    }

    // If it's not empty, can't place anything
    if (current_tile_type != BRUSH_TILE_EMPTY)
        return;

    // Check vertical conflicts for placing new platform
    if (check_platform_vertical_conflict(x, y))
        return;

    // Check if the next tile would also have vertical conflicts (for 2-tile platforms)
    if (x < PLATFORM_X_MAX && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]) == BRUSH_TILE_EMPTY && check_platform_vertical_conflict(x + 1, y))
        return;

    UBYTE left = (x > PLATFORM_X_MIN)
                     ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)])
                     : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX)
                      ? get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)])
                      : BRUSH_TILE_EMPTY;

    // If adjacent to existing platform, connect to it
    if (left == BRUSH_TILE_PLATFORM || right == BRUSH_TILE_PLATFORM)
    {
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
    }
    // If we can place a 2-tile platform, do it
    else if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY)
    {
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    }
    else
    {
        // Can't place a valid platform
        return;
    }

    rebuild_platform_row(y);
}

void vm_paint(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint(x, y);
}

// void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
// {
//     UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
//     UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
//     script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_pos(x, y);
// }

// void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED
// {
//     UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
//     UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
//     script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = 3; // get_brush_preview_tile(x, y);
// }

// Helper function to get brush tile state
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED
{
    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);

    // Handle different tile types
    switch (current_tile_type)
    {
    case BRUSH_TILE_EMPTY:
        // Check if we can place a platform here
        return get_platform_placement_type(x, y);

    case BRUSH_TILE_PLATFORM:
        // Can delete existing platform
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_PLAYER:
        return SELECTOR_STATE_PLAYER;

    case BRUSH_TILE_ENEMY_L:
        return SELECTOR_STATE_ENEMY_LEFT;

    case BRUSH_TILE_ENEMY_R:
        return SELECTOR_STATE_ENEMY_RIGHT;

    case BRUSH_TILE_EXIT:
        // Can't interact with exit tiles
        return SELECTOR_STATE_DEFAULT;

    default:
        return SELECTOR_STATE_DEFAULT;
    }
}

// VM wrapper function
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_state(x, y);
}
