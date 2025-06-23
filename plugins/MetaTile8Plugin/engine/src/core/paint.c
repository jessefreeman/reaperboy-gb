#pragma bank 254

#include <gbdk/platform.h>
#include "paint.h" // ← Make sure your prototypes are visible first
#include "vm.h"
#include "meta_tiles.h"
#include "tile_utils.h"
#include "gbs_types.h"
#include "actor.h"
#include "code_gen.h"

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
    // if (left == BRUSH_TILE_PLATFORM && right == BRUSH_TILE_PLATFORM)
    //     return SELECTOR_STATE_PLATFORM_CENTER;
    // else
    if (left == BRUSH_TILE_PLATFORM)
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
                    // Remove enemies above this platform before deleting it
                    remove_enemies_above_platform(seq_start, y);
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
                // Remove enemies above this platform before deleting it
                remove_enemies_above_platform(seq_start, y);
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

// Helper function to clear any existing player tile on row 11
void clear_existing_player_on_row_11(void) BANKED
{
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, 11)]) == BRUSH_TILE_PLAYER)
        {
            replace_meta_tile(x, 11, TILE_EMPTY, 1);
        }
    }
}

// Helper function to check if player can be painted at position
UBYTE can_paint_player(UBYTE x, UBYTE y) BANKED
{
    // Check if position is valid for player placement
    if (y != 11)
        return 0;

    // Check if position is within bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return 0;

    // Check if there's a platform below
    if (!has_platform_below(x, y))
        return 0;

    // Check if current tile is empty
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != BRUSH_TILE_EMPTY)
        return 0;

    return 1; // Can paint player here
}

// Helper function to paint a player tile
void paint_player(UBYTE x, UBYTE y) BANKED
{
    if (!can_paint_player(x, y))
        return;

    // Clear any existing player on row 11
    clear_existing_player_on_row_11(); // Place the new player tile
    replace_meta_tile(x, y, TILE_PLAYER, 1);

    // Update complete level code display after player change
    display_complete_level_code();
}

// Helper function to paint an enemy right tile
void paint_enemy_right(UBYTE x, UBYTE y) BANKED
{
    if (!can_paint_enemy_right(x, y))
        return; // Place the enemy tile
    replace_meta_tile(x, y, TILE_RIGHT_ENEMY, 1);

    // Update complete level code display after enemy change
    display_complete_level_code();
}

// Helper function to delete an enemy tile
void delete_enemy(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);

    // Can delete any enemy type
    if (current_tile_type == BRUSH_TILE_ENEMY_L || current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        replace_meta_tile(x, y, TILE_EMPTY, 1);

        // Update complete level code display after enemy deletion
        display_complete_level_code();
    }
}

// Helper function to paint an enemy left tile
void paint_enemy_left(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);

    // Can paint left enemy over right enemy (switching direction)    if (current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        replace_meta_tile(x, y, TILE_LEFT_ENEMY, 1);

        // Update complete level code display after enemy direction change
        display_complete_level_code();
        return;
    }

    // Can also paint on empty tiles if it would be a valid enemy position    if (current_tile_type == BRUSH_TILE_EMPTY && can_paint_enemy_right(x, y))
    {
        replace_meta_tile(x, y, TILE_LEFT_ENEMY, 1);

        // Update complete level code display after enemy placement
        display_complete_level_code();
        return;
    }
}

void paint(UBYTE x, UBYTE y) BANKED
{
    // Handle player placement on row 11
    if (y == 11)
    {
        paint_player(x, y);
        return;
    }

    // Check if we're over an existing enemy
    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
    if (current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        // Right enemy -> switch to left
        paint_enemy_left(x, y);
        return;
    }
    else if (current_tile_type == BRUSH_TILE_ENEMY_L)
    {
        // Left enemy -> delete
        delete_enemy(x, y);
        return;
    }

    // Handle enemy placement (above platforms)
    if (can_paint_enemy_right(x, y))
    {
        paint_enemy_right(x, y);
        return;
    }

    // Check bounds for platform placement
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX ||
        y < PLATFORM_Y_MIN || y > PLATFORM_Y_MAX)
        return;

    // Check if this is a valid row for platform placement
    if (!is_valid_platform_row(y))
        return; // If there's already a platform here, delete it
    if (current_tile_type == BRUSH_TILE_PLATFORM)
    {
        // Remove any enemies above this platform before deleting it
        remove_enemies_above_platform(x, y);

        replace_meta_tile(x, y, TILE_EMPTY, 1);
        rebuild_platform_row(y);

        // Update complete level code display after platform change
        display_complete_level_code();
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

    // Update complete level code display after platform change
    display_complete_level_code();
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

// Helper function to check if there's a platform below the given position
UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED
{
    // Check all rows below the current position
    for (UBYTE check_y = y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        // Only check valid platform rows
        if (is_valid_platform_row(check_y))
        {
            if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, check_y)]) == BRUSH_TILE_PLATFORM)
            {
                return 1; // Found a platform below
            }
        }
    }
    return 0; // No platform found below
}

// Helper function to get brush tile state
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED
{
    UBYTE current_tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);

    // Handle different tile types
    switch (current_tile_type)
    {
    case BRUSH_TILE_EMPTY:
        // Check if we're on row 11 (player placement row)
        if (y == 11)
        {
            // Use the exact same logic as paint_player
            if (can_paint_player(x, y))
            {
                return SELECTOR_STATE_PLAYER;
            }
            else
            {
                return SELECTOR_STATE_DEFAULT;
            }
        }

        // Check if we can place an enemy here (above platforms)
        if (can_paint_enemy_right(x, y))
        {
            return SELECTOR_STATE_ENEMY_RIGHT;
        }

        // Check if we can place a platform here
        return get_platform_placement_type(x, y);

    case BRUSH_TILE_PLATFORM:
        // Can delete existing platform
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_PLAYER:
        // If player tile is on row 11, return default (can't interact)
        if (y == 11)
        {
            return SELECTOR_STATE_DEFAULT;
        }
        return SELECTOR_STATE_PLAYER;
    case BRUSH_TILE_ENEMY_L:
        // When over a left enemy, show delete preview
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_ENEMY_R:
        // When over a right enemy, show left enemy preview for switching
        return SELECTOR_STATE_ENEMY_LEFT;

    case BRUSH_TILE_EXIT:
        // Can't interact with exit tiles
        return SELECTOR_STATE_DEFAULT;

    default:
        return SELECTOR_STATE_DEFAULT;
    }
}

// VM wrapper function - uses painting logic as source of truth
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE result = get_brush_tile_state(x, y);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = result;
}

// Helper function to count total enemies on the map
UBYTE count_enemies_on_map(void) BANKED
{
    UBYTE enemy_count = 0;

    // Scan the entire map area for enemies
    for (UBYTE yy = PLATFORM_Y_MIN; yy <= PLATFORM_Y_MAX; yy++)
    {
        for (UBYTE xx = PLATFORM_X_MIN; xx <= PLATFORM_X_MAX; xx++)
        {
            UBYTE tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(xx, yy)]);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                enemy_count++;
            }
        }
    }

    return enemy_count;
}

// Helper function to check if there's an enemy nearby (for spacing)
UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED
{
    // Check one tile to the left and right
    if (x > PLATFORM_X_MIN)
    {
        UBYTE left_tile = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)]);
        if (left_tile == BRUSH_TILE_ENEMY_L || left_tile == BRUSH_TILE_ENEMY_R)
            return 1;
    }

    if (x < PLATFORM_X_MAX)
    {
        UBYTE right_tile = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 1, y)]);
        if (right_tile == BRUSH_TILE_ENEMY_L || right_tile == BRUSH_TILE_ENEMY_R)
            return 1;
    }

    return 0;
}

// Helper function to check if enemy can be painted at position
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return 0;

    // Check if current tile is empty
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != BRUSH_TILE_EMPTY)
        return 0;

    // Check enemy limit - don't allow more than 6 enemies
    if (count_enemies_on_map() >= 6)
        return 0;

    // Check if there's a platform directly below (enemy stands on platform)
    if (y + 1 <= PLATFORM_Y_MAX && is_valid_platform_row(y + 1))
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y + 1)]) == BRUSH_TILE_PLATFORM)
        {
            // Check if there's no enemy nearby (one tile spacing rule)
            if (!has_enemy_nearby(x, y))
            {
                return 1;
            }
        }
    }

    return 0;
}

// Helper function to check if enemy can be painted at position
UBYTE can_paint_enemy_left(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within bounds
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return 0;

    // Check if current tile is empty
    if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]) != BRUSH_TILE_EMPTY)
        return 0;

    // Check if there's a platform directly below (enemy stands on platform)
    if (y + 1 <= PLATFORM_Y_MAX && is_valid_platform_row(y + 1))
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y + 1)]) == BRUSH_TILE_PLATFORM)
        {
            // Check if there's no enemy nearby (one tile spacing rule)
            if (!has_enemy_nearby(x, y))
            {
                return 1;
            }
        }
    }

    return 0;
}

// Helper function to remove enemies above a platform when platform is deleted
void remove_enemies_above_platform(UBYTE x, UBYTE y) BANKED
{
    // Check all rows above the platform position
    for (UBYTE check_y = PLATFORM_Y_MIN; check_y < y; check_y++)
    {
        // Check if there's an enemy at this position
        UBYTE tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, check_y)]);
        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
        {
            // Remove the enemy
            replace_meta_tile(x, check_y, TILE_EMPTY, 1);
        }
    }
}

// Level code generation and parsing functions
// Optimized packing: Player uses 5 bits, each enemy uses 9 bits (5+3+1)
// Total: 5 + (6 * 9) = 59 bits = 8 bytes (7.375 bytes, rounded up)

#define MAX_ENEMIES 6
#define PLAYER_X_BITS 5
#define ENEMY_X_BITS 5
#define ENEMY_Y_BITS 3
#define ENEMY_DIR_BITS 1
#define ENEMY_TOTAL_BITS (ENEMY_X_BITS + ENEMY_Y_BITS + ENEMY_DIR_BITS) // 9 bits per enemy

// Convert platform row to encoded value (13->0, 15->1, 17->2, 19->3, no enemy->7)
UBYTE encode_enemy_y(UBYTE y) BANKED
{
    switch (y)
    {
    case 13:
        return 0;
    case 15:
        return 1;
    case 17:
        return 2;
    case 19:
        return 3;
    default:
        return 7; // No enemy marker
    }
}

// Convert encoded value back to platform row
UBYTE decode_enemy_y(UBYTE encoded_y) BANKED
{
    switch (encoded_y)
    {
    case 0:
        return 13;
    case 1:
        return 15;
    case 2:
        return 17;
    case 3:
        return 19;
    default:
        return 0; // Invalid/no enemy
    }
}

// Generate level code from current map state
void generate_level_code(UBYTE *code_buffer) BANKED
{
    // Clear the buffer
    for (UBYTE i = 0; i < 8; i++)
    {
        code_buffer[i] = 0;
    }

    UBYTE bit_pos = 0;

    // Find and encode player position (row 11 only)
    UBYTE player_x = 0;
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, 11)]) == BRUSH_TILE_PLAYER)
        {
            player_x = x - PLATFORM_X_MIN; // Convert to 0-based index
            break;
        }
    }

    // Pack player X (5 bits)
    for (UBYTE bit = 0; bit < PLAYER_X_BITS; bit++)
    {
        if (player_x & (1 << bit))
        {
            code_buffer[bit_pos / 8] |= (1 << (bit_pos % 8));
        }
        bit_pos++;
    }

    // Find and encode enemies
    UBYTE enemy_count = 0;
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX && enemy_count < MAX_ENEMIES; y++)
    {
        if (!is_valid_platform_row(y))
            continue;

        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX && enemy_count < MAX_ENEMIES; x++)
        {
            UBYTE tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                // Pack enemy data (9 bits total: 5 bits X + 3 bits Y + 1 bit direction)
                UBYTE enemy_x = x - PLATFORM_X_MIN;
                UBYTE enemy_y = encode_enemy_y(y);
                UBYTE enemy_dir = (tile_type == BRUSH_TILE_ENEMY_L) ? 1 : 0;

                // Pack X (5 bits)
                for (UBYTE bit = 0; bit < ENEMY_X_BITS; bit++)
                {
                    if (enemy_x & (1 << bit))
                    {
                        code_buffer[bit_pos / 8] |= (1 << (bit_pos % 8));
                    }
                    bit_pos++;
                }

                // Pack Y (3 bits)
                for (UBYTE bit = 0; bit < ENEMY_Y_BITS; bit++)
                {
                    if (enemy_y & (1 << bit))
                    {
                        code_buffer[bit_pos / 8] |= (1 << (bit_pos % 8));
                    }
                    bit_pos++;
                }

                // Pack direction (1 bit)
                if (enemy_dir)
                {
                    code_buffer[bit_pos / 8] |= (1 << (bit_pos % 8));
                }
                bit_pos++;

                enemy_count++;
            }
        }
    }

    // Fill remaining enemy slots with "no enemy" marker (Y=7, X=0, DIR=0)
    for (; enemy_count < MAX_ENEMIES; enemy_count++)
    {
        // Skip X bits (already 0)
        bit_pos += ENEMY_X_BITS;

        // Set Y to 7 (no enemy marker - 3 bits: 111)
        for (UBYTE bit = 0; bit < ENEMY_Y_BITS; bit++)
        {
            code_buffer[bit_pos / 8] |= (1 << (bit_pos % 8));
            bit_pos++;
        }

        // Skip direction bit (already 0)
        bit_pos += ENEMY_DIR_BITS;
    }
}

// Parse level code and apply to map
void parse_level_code(UBYTE *code_buffer) BANKED
{
    UBYTE bit_pos = 0;

    // Clear existing player on row 11
    clear_existing_player_on_row_11();

    // Clear existing enemies
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            UBYTE tile_type = get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                replace_meta_tile(x, y, TILE_EMPTY, 1);
            }
        }
    }

    // Unpack player X (5 bits)
    UBYTE player_x = 0;
    for (UBYTE bit = 0; bit < PLAYER_X_BITS; bit++)
    {
        if (code_buffer[bit_pos / 8] & (1 << (bit_pos % 8)))
        {
            player_x |= (1 << bit);
        }
        bit_pos++;
    }

    // Place player if valid position
    UBYTE player_map_x = player_x + PLATFORM_X_MIN;
    if (player_map_x >= PLATFORM_X_MIN && player_map_x <= PLATFORM_X_MAX &&
        has_platform_below(player_map_x, 11))
    {
        replace_meta_tile(player_map_x, 11, TILE_PLAYER, 1);
    }

    // Unpack enemies
    for (UBYTE enemy_idx = 0; enemy_idx < MAX_ENEMIES; enemy_idx++)
    {
        // Unpack X (5 bits)
        UBYTE enemy_x = 0;
        for (UBYTE bit = 0; bit < ENEMY_X_BITS; bit++)
        {
            if (code_buffer[bit_pos / 8] & (1 << (bit_pos % 8)))
            {
                enemy_x |= (1 << bit);
            }
            bit_pos++;
        }

        // Unpack Y (3 bits)
        UBYTE enemy_y_encoded = 0;
        for (UBYTE bit = 0; bit < ENEMY_Y_BITS; bit++)
        {
            if (code_buffer[bit_pos / 8] & (1 << (bit_pos % 8)))
            {
                enemy_y_encoded |= (1 << bit);
            }
            bit_pos++;
        }

        // Unpack direction (1 bit)
        UBYTE enemy_dir = 0;
        if (code_buffer[bit_pos / 8] & (1 << (bit_pos % 8)))
        {
            enemy_dir = 1;
        }
        bit_pos++;

        // Place enemy if valid (Y != 7 means enemy exists)
        if (enemy_y_encoded != 7)
        {
            UBYTE enemy_map_x = enemy_x + PLATFORM_X_MIN;
            UBYTE enemy_map_y = decode_enemy_y(enemy_y_encoded);

            if (enemy_map_x >= PLATFORM_X_MIN && enemy_map_x <= PLATFORM_X_MAX &&
                enemy_map_y >= PLATFORM_Y_MIN && enemy_map_y <= PLATFORM_Y_MAX &&
                is_valid_platform_row(enemy_map_y))
            {
                UBYTE enemy_tile = enemy_dir ? TILE_LEFT_ENEMY : TILE_RIGHT_ENEMY;
                replace_meta_tile(enemy_map_x, enemy_map_y, enemy_tile, 1);
            }
        }
    }
}

// VM wrapper for generating level code
void vm_generate_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Get the variable ID where to store the level code
    uint16_t varId = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);

    // Generate the 8-byte level code
    UBYTE code_buffer[8];
    generate_level_code(code_buffer);

    // Store the 8 bytes in 8 consecutive variables
    for (UBYTE i = 0; i < 8; i++)
    {
        script_memory[varId + i] = code_buffer[i];
    }
}

// VM wrapper for parsing level code
void vm_parse_level_code(SCRIPT_CTX *THIS) BANKED
{
    // Get the variable ID where the level code is stored
    uint16_t varId = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);

    // Read the 8 bytes from 8 consecutive variables
    UBYTE code_buffer[8];
    for (UBYTE i = 0; i < 8; i++)
    {
        code_buffer[i] = (UBYTE)script_memory[varId + i];
    }

    // Parse and apply the level code to the map
    parse_level_code(code_buffer);
}

// Helper function to check if the level has valid player/enemy setup
UBYTE validate_level_setup(void) BANKED
{
    // Check if player exists on row 11
    UBYTE has_player = 0;
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, 11)]) == BRUSH_TILE_PLAYER)
        {
            has_player = 1;
            break;
        }
    }

    // A valid level should have a player
    return has_player;
}

// VM wrapper for level validation
void vm_validate_level_setup(SCRIPT_CTX *THIS) BANKED
{
    // Get the variable ID where to store the validation result
    uint16_t varId = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);

    // Validate and store result (1 = valid, 0 = invalid)
    UBYTE is_valid = validate_level_setup();
    script_memory[varId] = is_valid;
}

// Get level statistics (player X position and enemy count)
void get_level_stats(UBYTE *player_x, UBYTE *enemy_count) BANKED
{
    // Find player position
    *player_x = 0;
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, 11)]) == BRUSH_TILE_PLAYER)
        {
            *player_x = x;
            break;
        }
    }

    // Count enemies
    *enemy_count = count_enemies_on_map();
}

// VM wrapper for level statistics
void vm_get_level_stats(SCRIPT_CTX *THIS) BANKED
{
    // Get variable IDs for player_x and enemy_count
    uint16_t player_x_var = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);
    uint16_t enemy_count_var = *(uint16_t *)VM_REF_TO_PTR(FN_ARG1);

    UBYTE player_x, enemy_count;
    get_level_stats(&player_x, &enemy_count);

    script_memory[player_x_var] = player_x;
    script_memory[enemy_count_var] = enemy_count;
}

// Test function to verify compilation
void test_level_code_system(void) BANKED
{
    // Test generation
    UBYTE test_buffer[8];
    generate_level_code(test_buffer);

    // Test parsing
    parse_level_code(test_buffer);

    // Test validation
    UBYTE is_valid = validate_level_setup();
    (void)is_valid; // Suppress unused warning

    // Test stats
    UBYTE player_x, enemy_count;
    get_level_stats(&player_x, &enemy_count);
}
