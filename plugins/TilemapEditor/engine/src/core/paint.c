#pragma bank 254

#include <gbdk/platform.h>
#include "paint.h"
#include "vm.h"
#include "meta_tiles.h"
#include "tile_utils.h"
#include "code_gen.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"

// External actor ID declarations
extern UBYTE paint_player_id;
extern UBYTE paint_exit_id;
extern void reconstruct_tilemap_from_level_code(void) BANKED;
extern void mark_display_position_for_update(UBYTE position) BANKED;
extern void display_complete_level_code(void) BANKED;
extern void display_selective_level_code_fast(void) BANKED;
extern UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
extern UWORD match_platform_pattern(UWORD pattern) BANKED;
extern UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED;

// External reference to level code structure
extern level_code_t current_level_code;

// ============================================================================
// ACTOR SYSTEM DECLARATIONS - For GB Studio Engine Compatibility
// ============================================================================

// actor_t is already defined in gbs_types.h (included via meta_tiles.h)

// Use GB Studio standard direction constants
#define DIRECTION_DOWN 0
#define DIRECTION_RIGHT 1
#define DIRECTION_UP 2
#define DIRECTION_LEFT 3

// External actor array from GB Studio engine
extern actor_t actors[];

// Forward declarations for actor management functions
void activate_actor(actor_t *actor) BANKED;
void deactivate_actor(actor_t *actor) BANKED;
void actor_set_dir(actor_t *actor, UBYTE dir, UBYTE moving) BANKED;

// ============================================================================
// CORE CONSTANTS - Aligned with code_gen.c
// ============================================================================

// Platform-specific constants not in core headers
#define PLATFORM_MIN_VERTICAL_GAP 1
#define PLATFORM_MAX_LENGTH 8

// Note: Other constants now defined in code_level_core.h

// ============================================================================
// OPTIMIZED UTILITY FUNCTIONS
// ============================================================================

// Inline utility for repeated boundary checks
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y) BANKED
{
    return (x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
            y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX);
}

// Cached tile access to reduce repeated lookups
UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED
{
    return get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x, y)]);
}

// Convert subpixels
#define TO_FP(n) ((INT16)((n) << 4))

// ============================================================================
// PAINT ACTOR STORAGE - Global actor IDs for paint operations
// ============================================================================

UBYTE paint_player_id = 0;
UBYTE paint_exit_id = 1;
UBYTE paint_enemy_ids[5] = {2, 3, 4, 5, 6};
UBYTE paint_enemy_slots_used[5] = {0, 0, 0, 0, 0}; // Track which slots are in use

// FIFO enemy pool implementation - tracks paint order
UBYTE enemy_paint_order[5];  // Order in which enemies were painted (oldest first)
UBYTE enemy_paint_count = 0; // Number of enemies currently painted
UBYTE next_paint_slot = 0;   // Next slot to use when painting (cycles 0-4)

// ============================================================================
// VALIDATION HELPERS - Streamlined logic
// ============================================================================

UBYTE is_valid_platform_row(UBYTE y) BANKED
{
    return (y == 13 || y == 15 || y == 17 || y == 19);
}

UBYTE has_platform_below(UBYTE x, UBYTE y) BANKED
{
    for (UBYTE check_y = y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        if (is_valid_platform_row(check_y))
        {
            if (get_current_tile_type(x, check_y) == BRUSH_TILE_PLATFORM)
            {
                return 1;
            }
        }
    }
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
        if (!is_valid_platform_row(cy))
            continue;

        if (get_current_tile_type(x, cy) == BRUSH_TILE_PLATFORM)
        {
            return 1;
        }
    }
    return 0;
}

UBYTE count_enemies_on_map(void) BANKED
{
    UBYTE enemy_count = 0;
    for (UBYTE yy = PLATFORM_Y_MIN; yy <= PLATFORM_Y_MAX; yy++)
    {
        for (UBYTE xx = PLATFORM_X_MIN; xx <= PLATFORM_X_MAX; xx++)
        {
            UBYTE tile_type = get_current_tile_type(xx, yy);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                enemy_count++;
            }
        }
    }
    return enemy_count;
}

UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED
{
    if (x > PLATFORM_X_MIN)
    {
        UBYTE left_tile = get_current_tile_type(x - 1, y);
        if (left_tile == BRUSH_TILE_ENEMY_L || left_tile == BRUSH_TILE_ENEMY_R)
            return 1;
    }
    if (x < PLATFORM_X_MAX)
    {
        UBYTE right_tile = get_current_tile_type(x + 1, y);
        if (right_tile == BRUSH_TILE_ENEMY_L || right_tile == BRUSH_TILE_ENEMY_R)
            return 1;
    }
    return 0;
}

UBYTE has_enemy_below_player(UBYTE x, UBYTE y) BANKED
{
    // Check all rows below the player position for enemies
    for (UBYTE check_y = y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        UBYTE tile_type = get_current_tile_type(x, check_y);
        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
        {
            return 1;
        }
    }
    return 0;
}

UBYTE can_place_platform(UBYTE x, UBYTE y) BANKED
{
    return (is_within_platform_bounds(x, y) &&
            is_valid_platform_row(y) &&
            get_current_tile_type(x, y) == BRUSH_TILE_EMPTY &&
            !check_platform_vertical_conflict(x, y));
}

UBYTE can_paint_player(UBYTE x, UBYTE y) BANKED
{
    return (y == 11 &&
            x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
            has_platform_below(x, y) &&      // Must have platform below
            !has_enemy_below_player(x, y) && // Can't place player above enemies
            get_current_tile_type(x, y) == BRUSH_TILE_EMPTY);
}

UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED
{
    // Basic validation - must be within bounds and empty
    if (!is_within_platform_bounds(x, y))
        return 0;
    if (get_current_tile_type(x, y) != BRUSH_TILE_EMPTY)
        return 0;

    // No need to check enemy count - we use a pool system that cycles through enemies

    // Allow enemies on even rows where they are scanned: 12, 14, 16, 18
    // This removes the platform requirement temporarily for easier enemy placement testing
    if (y != 12 && y != 14 && y != 16 && y != 18)
        return 0;

    // Don't allow multiple enemies in the same position
    if (has_enemy_nearby(x, y))
        return 0;

    return 1;
}

UBYTE get_platform_placement_type(UBYTE x, UBYTE y) BANKED
{
    if (!can_place_platform(x, y))
        return SELECTOR_STATE_DEFAULT;

    // Cache adjacent tile types
    UBYTE left = (x > PLATFORM_X_MIN) ? get_current_tile_type(x - 1, y) : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_current_tile_type(x + 1, y) : BRUSH_TILE_EMPTY;
    if (left == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return SELECTOR_STATE_DEFAULT; // Cannot place - would exceed platform limit
        }
        return SELECTOR_STATE_PLATFORM_RIGHT;
    }
    if (right == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return SELECTOR_STATE_DEFAULT; // Cannot place - would exceed platform limit
        }
        return SELECTOR_STATE_PLATFORM_LEFT;
    }

    if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY && can_place_platform(x + 1, y))
    {
        // Check if creating a 2-tile platform would exceed limits after auto-merge
        if (would_2tile_platform_exceed_limit(x, y))
        {
            return SELECTOR_STATE_DEFAULT; // Cannot create - would exceed limit after merge
        }
        return SELECTOR_STATE_NEW_PLATFORM;
    }

    return SELECTOR_STATE_DEFAULT;
}

// ============================================================================
// PLATFORM LENGTH CALCULATION - Prevents exceeding 8-tile limit
// ============================================================================

UBYTE count_connected_platform_length(UBYTE x, UBYTE y) BANKED
{
    UBYTE length = 1; // Count the tile we would place

    // Count platforms to the left
    UBYTE left_x = x;
    while (left_x > PLATFORM_X_MIN)
    {
        left_x--;
        if (get_current_tile_type(left_x, y) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    // Count platforms to the right
    for (UBYTE right_x = x + 1; right_x <= PLATFORM_X_MAX; right_x++)
    {
        if (get_current_tile_type(right_x, y) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    return length;
}

// Check if placing a 2-tile platform starting at x would exceed limits after auto-merge
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y) BANKED
{
    // Check if there's a platform to the left that would merge
    UBYTE left_length = 0;
    if (x > PLATFORM_X_MIN && get_current_tile_type(x - 1, y) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the left
        UBYTE left_x = x - 1;
        while (left_x >= PLATFORM_X_MIN && get_current_tile_type(left_x, y) == BRUSH_TILE_PLATFORM)
        {
            left_length++;
            if (left_x == PLATFORM_X_MIN)
                break;
            left_x--;
        }
    }

    // Check if there's a platform to the right that would merge
    UBYTE right_length = 0;
    if (x + 2 <= PLATFORM_X_MAX && get_current_tile_type(x + 2, y) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the right
        for (UBYTE right_x = x + 2; right_x <= PLATFORM_X_MAX; right_x++)
        {
            if (get_current_tile_type(right_x, y) == BRUSH_TILE_PLATFORM)
            {
                right_length++;
            }
            else
            {
                break;
            }
        }
    }

    // Total length would be: left platforms + our 2 tiles + right platforms
    UBYTE total_length = left_length + 2 + right_length;
    return total_length > PLATFORM_MAX_LENGTH;
}

// ============================================================================
// PLATFORM RECONSTRUCTION - Optimized algorithm
// ============================================================================

void rebuild_platform_row(UBYTE y) BANKED
{
    UBYTE seq_start = 255, current_len = 0;

    for (UBYTE i = PLATFORM_X_MIN; i <= PLATFORM_X_MAX + 1; ++i)
    {
        UBYTE is_plat = (i <= PLATFORM_X_MAX) &&
                        get_current_tile_type(i, y) == BRUSH_TILE_PLATFORM;

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
                // Finalize sequence
                if (current_len == 1)
                {
                    remove_enemies_above_platform(seq_start, y);
                    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
                }
                else
                {
                    for (UBYTE j = 0; j < current_len; ++j)
                    {
                        UBYTE tx = seq_start + j;
                        UBYTE tile = (j == 0) ? TILE_PLATFORM_LEFT : (j == current_len - 1) ? TILE_PLATFORM_RIGHT
                                                                                            : TILE_PLATFORM_MIDDLE;
                        replace_meta_tile(tx, y, tile, 1);
                    }
                }
                seq_start = 255;
                current_len = 0;
            }
        }
        else if (seq_start != 255)
        {
            // End of sequence
            if (current_len == 1)
            {
                remove_enemies_above_platform(seq_start, y);
                replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
            }
            else
            {
                for (UBYTE j = 0; j < current_len; ++j)
                {
                    UBYTE tx = seq_start + j;
                    UBYTE tile = (j == 0) ? TILE_PLATFORM_LEFT : (j == current_len - 1) ? TILE_PLATFORM_RIGHT
                                                                                        : TILE_PLATFORM_MIDDLE;
                    replace_meta_tile(tx, y, tile, 1);
                }
            }
            seq_start = 255;
            current_len = 0;
        }
    }
}

// ============================================================================
// ENTITY MANAGEMENT - Clear and consistent
// ============================================================================

void move_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED
{
    actor_t *actor = &actors[actor_id];
    actor->pos.x = TO_FP(x * 8);
    actor->pos.y = TO_FP(y * 8);
    activate_actor(actor);
}

void move_player_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y) BANKED
{
    actor_t *actor = &actors[actor_id];
    // Center the player in the column by offsetting -4 pixels horizontally
    actor->pos.x = TO_FP(x * 8 - 4);
    actor->pos.y = TO_FP(y * 8);
    activate_actor(actor);
}

UBYTE get_next_enemy_slot_from_pool(void) BANKED
{
    UBYTE slot_to_use;

    if (enemy_paint_count < 5)
    {
        // We have available slots, use the next unused one
        slot_to_use = next_paint_slot;

        // Add this slot to the paint order queue
        enemy_paint_order[enemy_paint_count] = slot_to_use;
        enemy_paint_count++;

        // Move to next slot for future painting
        next_paint_slot = (next_paint_slot + 1) % 5;
    }
    else
    {
        // All slots are used, reuse the oldest (first in queue)
        slot_to_use = enemy_paint_order[0];

        // Shift the queue left to remove the oldest
        for (UBYTE i = 0; i < 4; i++)
        {
            enemy_paint_order[i] = enemy_paint_order[i + 1];
        }

        // Add the reused slot to the back of the queue
        enemy_paint_order[4] = slot_to_use;
        // enemy_paint_count stays at 5
    }

    return slot_to_use;
}

void remove_enemy_from_paint_order(UBYTE slot) BANKED
{
    // Find the slot in the paint order and remove it
    for (UBYTE i = 0; i < enemy_paint_count; i++)
    {
        if (enemy_paint_order[i] == slot)
        {
            // Shift remaining elements left
            for (UBYTE j = i; j < enemy_paint_count - 1; j++)
            {
                enemy_paint_order[j] = enemy_paint_order[j + 1];
            }
            enemy_paint_count--;
            break;
        }
    }
}

void add_enemy_to_front_of_paint_order(UBYTE slot) BANKED
{
    // Shift existing elements right
    for (UBYTE i = enemy_paint_count; i > 0; i--)
    {
        enemy_paint_order[i] = enemy_paint_order[i - 1];
    }

    // Add the slot to the front (will be next to reuse)
    enemy_paint_order[0] = slot;
    if (enemy_paint_count < 5)
    {
        enemy_paint_count++;
    }
}

void clear_enemy_tile_at_position(UBYTE x, UBYTE y) BANKED
{
    UBYTE tile_type = get_current_tile_type(x, y);
    if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
    {
        replace_meta_tile(x, y, TILE_EMPTY, 1);
        update_level_code_for_paint(x, y);
    }
}

void reset_enemy_pool(void) BANKED
{
    // Reset paint order tracking
    for (UBYTE i = 0; i < 5; i++)
    {
        enemy_paint_order[i] = 0;
        paint_enemy_slots_used[i] = 0;
    }
    enemy_paint_count = 0;
    next_paint_slot = 0;
}

void clear_existing_player_on_row_11(void) BANKED
{
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_current_tile_type(x, 11) == BRUSH_TILE_PLAYER)
        {
            replace_meta_tile(x, 11, TILE_EMPTY, 1);
        }
    }
}

void remove_enemies_above_platform(UBYTE x, UBYTE y) BANKED
{
    for (UBYTE check_y = PLATFORM_Y_MIN; check_y < y; check_y++)
    {
        UBYTE tile_type = get_current_tile_type(x, check_y);
        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
        {
            replace_meta_tile(x, check_y, TILE_EMPTY, 1);

            // Also deactivate the corresponding actor
            for (UBYTE i = 0; i < 5; i++)
            {
                if (paint_enemy_slots_used[i])
                {
                    actor_t *enemy = &actors[paint_enemy_ids[i]];
                    // Convert actor position from fixed point to tile coordinates
                    UBYTE actor_tile_x = (enemy->pos.x >> 4) / 8;
                    UBYTE actor_tile_y = (enemy->pos.y >> 4) / 8;

                    // For left-facing enemies, the tile position is offset by +1 tile
                    if (enemy->dir == DIRECTION_LEFT)
                    {
                        actor_tile_x += 1;
                    }

                    if (actor_tile_x == x && actor_tile_y == check_y)
                    {
                        deactivate_actor(enemy);
                        paint_enemy_slots_used[i] = 0; // Mark slot as available

                        // Remove from paint order and add to front for immediate reuse
                        remove_enemy_from_paint_order(i);
                        add_enemy_to_front_of_paint_order(i);
                        break;
                    }
                }
            }
        }
    }
}

// ============================================================================
// PAINTING FUNCTIONS - Unified with level code updates
// ============================================================================

void paint_player(UBYTE x, UBYTE y) BANKED
{
    if (!can_paint_player(x, y))
        return;

    clear_existing_player_on_row_11();
    replace_meta_tile(x, y, TILE_PLAYER, 1); // Move player actor to this position with centering offset
    move_player_actor_to_tile(paint_player_id, x, y);

    // Position the exit sprite based on the new player position
    position_exit_for_player(x, y);

    // --- Ensure level code and display are updated after painting the player ---
    extract_player_data();
    mark_display_position_for_update(16); // Mark char 16 for display update
    display_selective_level_code_fast();
}

void paint_enemy_right(UBYTE x, UBYTE y) BANKED
{
    if (!can_paint_enemy_right(x, y))
        return;

    replace_meta_tile(x, y, TILE_RIGHT_ENEMY, 1);

    // Get next enemy from FIFO pool
    UBYTE enemy_slot = get_next_enemy_slot_from_pool();

    // If this slot was already in use, clear its old tile and deactivate the actor
    if (paint_enemy_slots_used[enemy_slot])
    {
        actor_t *old_enemy = &actors[paint_enemy_ids[enemy_slot]];
        // Convert actor position from fixed point to tile coordinates
        UBYTE old_tile_x = (old_enemy->pos.x >> 4) / 8;
        UBYTE old_tile_y = (old_enemy->pos.y >> 4) / 8;

        // For left-facing enemies, the tile position is offset by +1 tile
        if (old_enemy->dir == DIRECTION_LEFT)
        {
            old_tile_x += 1;
        }

        // Clear the old tile
        clear_enemy_tile_at_position(old_tile_x, old_tile_y);

        deactivate_actor(old_enemy);
    }

    // Set up the new enemy
    actor_t *enemy = &actors[paint_enemy_ids[enemy_slot]];
    enemy->pos.x = TO_FP(x * 8);
    enemy->pos.y = TO_FP(y * 8);
    activate_actor(enemy);
    actor_set_dir(enemy, DIRECTION_RIGHT, TRUE);
    paint_enemy_slots_used[enemy_slot] = 1; // Mark slot as used

    update_level_code_for_paint(x, y); // Smart update
}

void paint_enemy_left(UBYTE x, UBYTE y) BANKED
{
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    UBYTE current_tile_type = get_current_tile_type(x, y);

    if (current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        replace_meta_tile(x, y, TILE_LEFT_ENEMY, 1); // Find the enemy actor at this position and change direction
        for (UBYTE i = 0; i < 5; i++)
        {
            if (paint_enemy_slots_used[i])
            {
                actor_t *enemy = &actors[paint_enemy_ids[i]];
                // Convert actor position from fixed point to tile coordinates
                UBYTE actor_tile_x = (enemy->pos.x >> 4) / 8;
                UBYTE actor_tile_y = (enemy->pos.y >> 4) / 8;

                if (actor_tile_x == x && actor_tile_y == y)
                {
                    // When changing to left direction, offset the actor position
                    enemy->pos.x = TO_FP(x * 8 - 8);
                    actor_set_dir(enemy, DIRECTION_LEFT, TRUE);
                    break;
                }
            }
        }

        update_level_code_for_paint(x, y);
    }
    else if (current_tile_type == BRUSH_TILE_EMPTY && can_paint_enemy_right(x, y))
    {
        replace_meta_tile(x, y, TILE_LEFT_ENEMY, 1);

        // Get next enemy from FIFO pool
        UBYTE enemy_slot = get_next_enemy_slot_from_pool();

        // If this slot was already in use, clear its old tile and deactivate the actor
        if (paint_enemy_slots_used[enemy_slot])
        {
            actor_t *old_enemy = &actors[paint_enemy_ids[enemy_slot]];
            // Convert actor position from fixed point to tile coordinates
            UBYTE old_tile_x = (old_enemy->pos.x >> 4) / 8;
            UBYTE old_tile_y = (old_enemy->pos.y >> 4) / 8;

            // For left-facing enemies, the tile position is offset by +1 tile
            if (old_enemy->dir == DIRECTION_LEFT)
            {
                old_tile_x += 1;
            }

            // Clear the old tile
            clear_enemy_tile_at_position(old_tile_x, old_tile_y);

            deactivate_actor(old_enemy);
        }

        // Set up the new left-facing enemy
        actor_t *enemy = &actors[paint_enemy_ids[enemy_slot]];
        // Position left-facing enemy with offset
        enemy->pos.x = TO_FP(x * 8 - 8);
        enemy->pos.y = TO_FP(y * 8);
        activate_actor(enemy);
        actor_set_dir(enemy, DIRECTION_LEFT, TRUE);
        paint_enemy_slots_used[enemy_slot] = 1; // Mark slot as used

        update_level_code_for_paint(x, y);
    }
}

void delete_enemy(UBYTE x, UBYTE y) BANKED
{
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    UBYTE current_tile_type = get_current_tile_type(x, y);

    if (current_tile_type == BRUSH_TILE_ENEMY_L || current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        replace_meta_tile(x, y, TILE_EMPTY, 1);
        // Find and deactivate the enemy actor at this position
        for (UBYTE i = 0; i < 5; i++)
        {
            if (paint_enemy_slots_used[i])
            {
                actor_t *enemy = &actors[paint_enemy_ids[i]];
                // Convert actor position from fixed point to tile coordinates
                UBYTE actor_tile_x = (enemy->pos.x >> 4) / 8;
                UBYTE actor_tile_y = (enemy->pos.y >> 4) / 8;

                // For left-facing enemies, the tile position is offset by +1 tile
                if (enemy->dir == DIRECTION_LEFT)
                {
                    actor_tile_x += 1;
                }

                if (actor_tile_x == x && actor_tile_y == y)
                {
                    deactivate_actor(enemy);
                    paint_enemy_slots_used[i] = 0; // Mark slot as available for reuse

                    // Remove from paint order and add to front for immediate reuse
                    remove_enemy_from_paint_order(i);
                    add_enemy_to_front_of_paint_order(i);
                    break;
                }
            }
        }

        update_level_code_for_paint(x, y);
    }
}

// ============================================================================
// MAIN PAINT FUNCTION - Simplified logic
// ============================================================================

void paint(UBYTE x, UBYTE y) BANKED
{
    // Player placement on row 11
    if (y == 11)
    {
        paint_player(x, y);
        return;
    }

    // Cache current tile type to avoid repeated lookups
    UBYTE current_tile_type = get_current_tile_type(x, y);

    // Enemy state transitions: right -> left -> delete
    if (current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        paint_enemy_left(x, y);
        return;
    }
    else if (current_tile_type == BRUSH_TILE_ENEMY_L)
    {
        delete_enemy(x, y);
        return;
    }

    // New enemy placement
    if (can_paint_enemy_right(x, y))
    {
        paint_enemy_right(x, y);
        return;
    }

    // Early exit for invalid platform areas
    if (!is_within_platform_bounds(x, y) || !is_valid_platform_row(y))
    {
        return;
    }

    // Platform deletion
    if (current_tile_type == BRUSH_TILE_PLATFORM)
    {
        remove_enemies_above_platform(x, y);
        replace_meta_tile(x, y, TILE_EMPTY, 1);
        rebuild_platform_row(y);
        update_level_code_for_paint(x, y); // Smart update
        return;
    }

    // Platform placement - early exit conditions
    if (current_tile_type != BRUSH_TILE_EMPTY ||
        check_platform_vertical_conflict(x, y))
    {
        return;
    }

    // Cache adjacent tile types
    UBYTE left = (x > PLATFORM_X_MIN) ? get_current_tile_type(x - 1, y) : BRUSH_TILE_EMPTY;
    UBYTE right = (x < PLATFORM_X_MAX) ? get_current_tile_type(x + 1, y) : BRUSH_TILE_EMPTY;

    if (left == BRUSH_TILE_PLATFORM || right == BRUSH_TILE_PLATFORM)
    {
        // Check if connecting would exceed 8-tile limit
        UBYTE platform_length = count_connected_platform_length(x, y);
        if (platform_length > PLATFORM_MAX_LENGTH)
        {
            return; // Cannot exceed 8-tile platform limit
        }

        // Connect to existing platform
        replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
    }
    else if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
             !check_platform_vertical_conflict(x + 1, y))
    {
        // Check if creating a 2-tile platform would exceed limits after auto-merge
        if (would_2tile_platform_exceed_limit(x, y))
        {
            return; // Cannot create 2-tile platform - would exceed limit after merge
        }

        // Create new 2-tile platform
        replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
        replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
    }
    else
    {
        return; // Can't place platform
    }
    rebuild_platform_row(y);
    update_level_code_for_paint(x, y); // Smart update
}

// ============================================================================
// BRUSH TILE STATE - Unified with paint logic
// ============================================================================

UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED
{
    // Cache current tile type to avoid repeated lookups
    UBYTE current_tile_type = get_current_tile_type(x, y);

    switch (current_tile_type)
    {
    case BRUSH_TILE_EMPTY:
        if (y == 11)
        {
            return can_paint_player(x, y) ? SELECTOR_STATE_PLAYER : SELECTOR_STATE_DEFAULT;
        }
        if (can_paint_enemy_right(x, y))
        {
            return SELECTOR_STATE_ENEMY_RIGHT;
        }
        return get_platform_placement_type(x, y);

    case BRUSH_TILE_PLATFORM:
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_PLAYER:
        return (y == 11) ? SELECTOR_STATE_DEFAULT : SELECTOR_STATE_PLAYER;

    case BRUSH_TILE_ENEMY_L:
        return SELECTOR_STATE_DELETE;

    case BRUSH_TILE_ENEMY_R:
        return SELECTOR_STATE_ENEMY_LEFT;

    case BRUSH_TILE_EXIT:
    default:
        return SELECTOR_STATE_DEFAULT;
    }
}

// ============================================================================
// MAP SETUP - Streamlined actor placement
// ============================================================================

void vm_setup_map(SCRIPT_CTX *THIS, INT16 idx) OLDCALL BANKED
{
    (void)idx;

    uint16_t varId = *(uint16_t *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE playerId = 0;
    UBYTE exitId = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    UBYTE enemies[5] = {
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG2), *(UBYTE *)VM_REF_TO_PTR(FN_ARG3),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG4), *(UBYTE *)VM_REF_TO_PTR(FN_ARG5),
        *(UBYTE *)VM_REF_TO_PTR(FN_ARG6)};

    UBYTE enemy_count = 0, playerPlaced = 0, exitPlaced = 0;
    UBYTE playerX = 0, playerRow = 0;

    // Single pass through the map
    for (UBYTE yy = 10; yy <= 19 && !(playerPlaced && exitPlaced && enemy_count >= 5); ++yy)
    {
        for (UBYTE xx = 2; xx < 22 && !(playerPlaced && exitPlaced && enemy_count >= 5); ++xx)
        {
            UBYTE tid = sram_map_data[METATILE_MAP_OFFSET(xx, yy)];
            UBYTE tt = get_tile_type(tid); // Place player
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
            }

            // Place exit (after player is placed)
            // if (playerPlaced && !exitPlaced && xx == playerX && yy > playerRow && tt == BRUSH_TILE_PLATFORM)
            // {
            //     UBYTE placeX = (tid == TILE_PLATFORM_RIGHT) ? xx - 1 : xx;
            //     actor_t *ex = &actors[exitId];
            //     ex->pos.x = TO_FP(placeX * 8);
            //     ex->pos.y = TO_FP((yy - 1) * 8);
            //     activate_actor(ex);

            //     // Place exit tiles
            //     replace_meta_tile(placeX, yy - 1, TILE_EXIT_BOTTOM_LEFT, 1);
            //     replace_meta_tile(placeX + 1, yy - 1, TILE_EXIT_BOTTOM_RIGHT, 1);
            //     replace_meta_tile(placeX, yy - 2, TILE_EXIT_TOP_LEFT, 1);
            //     replace_meta_tile(placeX + 1, yy - 2, TILE_EXIT_TOP_RIGHT, 1);
            //     exitPlaced = 1;
            // } // Place enemies
            if (enemy_count < 5 && (tt == BRUSH_TILE_ENEMY_R || tt == BRUSH_TILE_ENEMY_L))
            {
                actor_t *e = &actors[enemies[enemy_count]];
                INT16 fx = (tt == BRUSH_TILE_ENEMY_L) ? (xx * 8 - 8) : (xx * 8);
                e->pos.x = TO_FP(fx);
                e->pos.y = TO_FP(yy * 8);
                actor_set_dir(e, tt == BRUSH_TILE_ENEMY_R ? DIRECTION_RIGHT : DIRECTION_LEFT, TRUE);
                activate_actor(e);
                replace_meta_tile(xx, yy, TILE_EMPTY, 1);
                enemy_count++;
            }
        }
    }

    // Deactivate unused enemy actors
    for (UBYTE ec = enemy_count; ec < 5; ++ec)
    {
        deactivate_actor(&actors[enemies[ec]]);
    }

    script_memory[varId] = enemy_count;
}

// ============================================================================
// VM WRAPPER FUNCTIONS - Essential only
// ============================================================================

void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED
{
    paint_player_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    paint_exit_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint_enemy_ids[0] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    paint_enemy_ids[1] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);
    paint_enemy_ids[2] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG4);
    paint_enemy_ids[3] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG5);
    paint_enemy_ids[4] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG6);

    // Reset the FIFO enemy pool
    reset_enemy_pool();

    // Disable all assigned actors to prepare for level loading
    deactivate_actor(&actors[paint_player_id]);
    deactivate_actor(&actors[paint_exit_id]);

    // Deactivate enemy actors
    for (UBYTE i = 0; i < 5; i++)
    {
        deactivate_actor(&actors[paint_enemy_ids[i]]);
    }
}

void vm_paint(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint(x, y);
}

void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_state(x, y);
}

void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_state(x, y);
}

void vm_enable_editor(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;

    // Only deactivate the specific actors that the tilemap editor will be using
    // This prevents disrupting other actors in the scene

    // Deactivate player actor
    deactivate_actor(&actors[paint_player_id]);

    // Deactivate exit actor
    deactivate_actor(&actors[paint_exit_id]);

    // Deactivate enemy actors
    for (UBYTE i = 0; i < 5; i++)
    {
        deactivate_actor(&actors[paint_enemy_ids[i]]);
    }

    // Reset the FIFO enemy pool
    reset_enemy_pool();

    // Check if map is empty and initialize with default level if needed
    if (is_map_empty())
    {
        init_default_level_code();
    }
    else
    {
        // Map has content, update level code from existing tilemap
        update_complete_level_code();
    }

    // Always ensure the level code is displayed after initialization
    force_complete_level_code_display();
}

// ============================================================================
// EXIT POSITIONING SYSTEM
// ============================================================================

void position_exit_for_player(UBYTE player_x, UBYTE player_y) BANKED
{
    // Find the first platform below the player
    UBYTE exit_platform_y = 0;
    UBYTE found_platform = 0;

    for (UBYTE check_y = player_y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        if (is_valid_platform_row(check_y))
        {
            if (get_current_tile_type(player_x, check_y) == BRUSH_TILE_PLATFORM)
            {
                exit_platform_y = check_y;
                found_platform = 1;
                break;
            }
        }
    }

    if (!found_platform)
        return; // No platform found below player

    // Calculate exit position: 1 tile above the platform
    UBYTE exit_y = exit_platform_y - 1;
    UBYTE exit_x = player_x;

    // Check if player is on the right edge of a platform
    // If so, move exit left by 1 tile
    UBYTE platform_tile = sram_map_data[METATILE_MAP_OFFSET(player_x, exit_platform_y)];
    if (platform_tile == TILE_PLATFORM_RIGHT && exit_x > PLATFORM_X_MIN)
    {
        exit_x = exit_x - 1;
    }

    // Clear any existing exit tiles from the map
    clear_existing_exit_tiles();

    // Position the exit actor
    move_actor_to_tile(paint_exit_id, exit_x, exit_y);

    // Place the exit tiles (2x2 exit sprite)
    replace_meta_tile(exit_x, exit_y, TILE_EXIT_BOTTOM_LEFT, 1);
    replace_meta_tile(exit_x + 1, exit_y, TILE_EXIT_BOTTOM_RIGHT, 1);
    replace_meta_tile(exit_x, exit_y - 1, TILE_EXIT_TOP_LEFT, 1);
    replace_meta_tile(exit_x + 1, exit_y - 1, TILE_EXIT_TOP_RIGHT, 1);
}

void clear_existing_exit_tiles(void) BANKED
{
    // Scan the map for existing exit tiles and clear them
    for (UBYTE y = PLATFORM_Y_MIN - 2; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(x, y)];
            if (tile == TILE_EXIT_TOP_LEFT || tile == TILE_EXIT_TOP_RIGHT ||
                tile == TILE_EXIT_BOTTOM_LEFT || tile == TILE_EXIT_BOTTOM_RIGHT)
            {
                replace_meta_tile(x, y, TILE_EMPTY, 1);
            }
        }
    }
}

// ============================================================================
// DEFAULT LEVEL INITIALIZATION
// ============================================================================

// Check if the map is empty (no platforms, player, or enemies)
UBYTE is_map_empty(void) BANKED
{
    // Check for any platform tiles
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            if (get_current_tile_type(x, y) == BRUSH_TILE_PLATFORM)
            {
                return 0; // Found a platform, map is not empty
            }
        }
    }

    // Check for player tile
    for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
    {
        if (get_current_tile_type(x, 11) == BRUSH_TILE_PLAYER)
        {
            return 0; // Found player, map is not empty
        }
    }

    // Check for enemy tiles
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++)
    {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++)
        {
            UBYTE tile_type = get_current_tile_type(x, y);
            if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
            {
                return 0; // Found enemy, map is not empty
            }
        }
    }

    return 1; // Map is empty
}

// Initialize with default level code: "000000000120000090000000"
// This creates a 2-tile platform in the middle with player positioned above it
void init_default_level_code(void) BANKED
{
    // Clear all platform patterns
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = 0;
    }

    // Set pattern 1 at position 9 and pattern 2 at position 10
    // Block 9 covers columns 5-9, pattern 1 places platform at position 4 (column 9)
    // Block 10 covers columns 10-14, pattern 2 places platform at position 3 (column 13)
    // This creates two separate single platforms - let's use pattern 3 instead for a continuous platform
    current_level_code.platform_patterns[9] = 3; // Two platforms at positions 3-4 (columns 8-9)

    // Set player column to 8 (above the first platform tile)
    current_level_code.player_column = 8;

    // Clear enemy data
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    // Apply the patterns to the tilemap
    reconstruct_tilemap_from_level_code();

    // Place the player
    UBYTE player_x = current_level_code.player_column + 2; // Convert to tile coordinates
    replace_meta_tile(player_x, 11, TILE_PLAYER, 1);
    move_player_actor_to_tile(paint_player_id, player_x, 11);

    // Position the exit sprite
    position_exit_for_player(player_x, 11);

    // Update the level code display
    force_complete_level_code_display();
}

// ============================================================================
// SMART UPDATE FUNCTIONS - Only update affected zones
// ============================================================================

// Update level code based on what was painted at a specific position
void update_level_code_for_paint(UBYTE x, UBYTE y) BANKED
{
    // For player painting (row 11), update enemy/player data
    if (y == 11)
    {
        // Extract player data
        extract_player_data();

        // Mark enemy/player data positions for update (positions 16-23)
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
        return;
    }

    // For enemy operations, update enemy data
    UBYTE current_tile_type = get_current_tile_type(x, y);
    if (current_tile_type == BRUSH_TILE_ENEMY_L || current_tile_type == BRUSH_TILE_ENEMY_R)
    {
        // For any enemy-related operation, extract enemy data and update display
        extract_enemy_data();
        extract_player_data(); // Player data might have changed too

        // Mark enemy/player data positions for update (positions 16-23)
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
        return;
    }

    // For platform operations, update affected zones
    if (y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX &&
        x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
        is_valid_platform_row(y))
    {
        // Platform operations can affect multiple zones on the same row due to auto-completion
        // Update all zones on this row to be safe
        UBYTE row_index = (y - PLATFORM_Y_MIN) / SEGMENT_HEIGHT;

        for (UBYTE col = 0; col < SEGMENTS_PER_ROW; col++)
        {
            UBYTE zone_index = row_index * SEGMENTS_PER_ROW + col;

            // Extract the current pattern for this zone
            UBYTE segment_x = 2 + col * SEGMENT_WIDTH;
            UBYTE segment_y = PLATFORM_Y_MIN + row_index * SEGMENT_HEIGHT;

            UBYTE row0, row1;
            UWORD pattern = extract_chunk_pattern(segment_x, segment_y, &row0, &row1);
            UWORD pattern_id = match_platform_pattern(pattern);

            current_level_code.platform_patterns[zone_index] = (UBYTE)pattern_id;

            // Mark this zone for display update
            mark_display_position_for_update(zone_index);
        }

        // Use fast selective update
        display_selective_level_code_fast();
        return;
    }

    // Fallback to complete update for other cases
    display_complete_level_code();
}
