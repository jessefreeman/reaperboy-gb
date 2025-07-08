#pragma bank 254

#include <gbdk/platform.h>
#include "paint_entity.h"
#include "paint_core.h"
#include "paint_platform.h"
#include "meta_tiles.h"
#include "tile_utils.h"
#include "code_level_core.h"

// External actor array from GB Studio engine
extern actor_t actors[];

// Forward declarations for actor management functions
extern void activate_actor(actor_t *actor) BANKED;
extern void deactivate_actor(actor_t *actor) BANKED;
extern void actor_set_dir(actor_t *actor, UBYTE dir, UBYTE moving) BANKED;
extern void extract_player_data(void) BANKED;

// Include reference to level code structure
extern level_code_t current_level_code;

// ============================================================================
// PAINT ACTOR STORAGE - Global actor IDs for paint operations
// ============================================================================

UBYTE paint_player_id = 0;
UBYTE paint_exit_id = 1;
UBYTE paint_enemy_ids[MAX_PAINT_ENEMIES] = {2, 3, 4, 5, 6};
UBYTE paint_enemy_slots_used[MAX_PAINT_ENEMIES] = {0, 0, 0, 0, 0}; // Track which slots are in use

// FIFO enemy pool implementation - tracks paint order
UBYTE enemy_paint_order[MAX_PAINT_ENEMIES]; // Order in which enemies were painted (oldest first)
UBYTE enemy_paint_count = 0;                // Number of enemies currently painted
UBYTE next_paint_slot = 0;                  // Next slot to use when painting (cycles 0-4)

// ============================================================================
// ENTITY VALIDATION
// ============================================================================

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

UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED
{
    // Check the same tile
    UBYTE current_tile = get_current_tile_type(x, y);
    if (current_tile == BRUSH_TILE_ENEMY_L || current_tile == BRUSH_TILE_ENEMY_R)
        return 1;

    // Check tile to the left
    if (x > PLATFORM_X_MIN)
    {
        UBYTE left_tile = get_current_tile_type(x - 1, y);
        if (left_tile == BRUSH_TILE_ENEMY_L || left_tile == BRUSH_TILE_ENEMY_R)
            return 1;
    }

    // Check tile to the right
    if (x < PLATFORM_X_MAX)
    {
        UBYTE right_tile = get_current_tile_type(x + 1, y);
        if (right_tile == BRUSH_TILE_ENEMY_L || right_tile == BRUSH_TILE_ENEMY_R)
            return 1;
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

    // Allow enemies only on standard rows where they are scanned: 12, 14, 16, 18
    if (y != 12 && y != 14 && y != 16 && y != 18)
        return 0;

    // Must have a platform directly below (strict 1:1 relationship)
    if (!has_platform_directly_below(x, y))
        return 0;

    // Don't allow enemies directly below the player
    // Player is at column 2-21 in tilemap, corresponding to 0-19 in level code
    UBYTE player_x = current_level_code.player_column + 2;
    if (x == player_x)
        return 0;

    // Don't allow multiple enemies in the same position or adjacent
    if (has_enemy_nearby(x, y))
        return 0;

    return 1;
}

// Find the next valid position for an enemy in the paint system
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y) BANKED
{
    // Start from current position
    UBYTE start_x = *x;
    UBYTE start_y = *y;
    UBYTE found = 0;

    // Check all standard enemy rows (these match ENEMY_ROWS in validation.c)
    const UBYTE enemy_rows[4] = {12, 14, 16, 18};

    // Check all standard enemy rows
    for (UBYTE row_index = 0; row_index < 4; row_index++)
    {
        UBYTE check_y = enemy_rows[row_index];

        // On the current row, start from the next column
        UBYTE start_col = (check_y == start_y) ? start_x + 1 : PLATFORM_X_MIN;

        // Scan this row
        for (UBYTE check_x = start_col; check_x <= PLATFORM_X_MAX; check_x++)
        {
            if (can_paint_enemy_right(check_x, check_y))
            {
                *x = check_x;
                *y = check_y;
                found = 1;
                return;
            }
        }
    }

    // If we get here and haven't found a valid position,
    // start over from the beginning
    if (!found)
    {
        for (UBYTE row_index = 0; row_index < 4; row_index++)
        {
            UBYTE check_y = enemy_rows[row_index];

            for (UBYTE check_x = PLATFORM_X_MIN; check_x <= PLATFORM_X_MAX; check_x++)
            {
                // Skip the original starting position since we already checked from there
                if (check_x == start_x && check_y == start_y)
                    continue;

                if (can_paint_enemy_right(check_x, check_y))
                {
                    *x = check_x;
                    *y = check_y;
                    return;
                }
            }
        }
    }

    // If we get here, we couldn't find any valid position, so leave the coordinates unchanged
}

// ============================================================================
// ENTITY MOVEMENT AND MANAGEMENT
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

    if (enemy_paint_count < MAX_PAINT_ENEMIES)
    {
        // We have available slots, use the next unused one
        slot_to_use = next_paint_slot;

        // Add this slot to the paint order queue
        enemy_paint_order[enemy_paint_count] = slot_to_use;
        enemy_paint_count++;

        // Move to next slot for future painting
        next_paint_slot = (next_paint_slot + 1) % MAX_PAINT_ENEMIES;
    }
    else
    {
        // All slots are used, reuse the oldest (first in queue)
        slot_to_use = enemy_paint_order[0];

        // Shift the queue left to remove the oldest
        for (UBYTE i = 0; i < MAX_PAINT_ENEMIES - 1; i++)
        {
            enemy_paint_order[i] = enemy_paint_order[i + 1];
        }

        // Add the reused slot to the back of the queue
        enemy_paint_order[MAX_PAINT_ENEMIES - 1] = slot_to_use;
        // enemy_paint_count stays at MAX_PAINT_ENEMIES
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
    if (enemy_paint_count < MAX_PAINT_ENEMIES)
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
    for (UBYTE i = 0; i < MAX_PAINT_ENEMIES; i++)
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
    // Find the corresponding enemy row for this platform
    // Platforms are at rows 13, 15, 17, 19, and enemies are at rows 12, 14, 16, 18
    UBYTE enemy_row = 255;

    if (y == 13)
        enemy_row = 12;
    else if (y == 15)
        enemy_row = 14;
    else if (y == 17)
        enemy_row = 16;
    else if (y == 19)
        enemy_row = 18;

    // If this isn't a platform row that would affect enemies, exit
    if (enemy_row == 255)
        return;

    // Check if there's an enemy actor at this position and remove it
    UBYTE found_enemy = 0;
    for (UBYTE i = 0; i < MAX_PAINT_ENEMIES; i++)
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

            if (actor_tile_x == x && actor_tile_y == enemy_row)
            {
                // Found enemy at this position - remove it
                deactivate_actor(enemy);
                paint_enemy_slots_used[i] = 0; // Mark slot as available for reuse

                // Remove from paint order and add to front for immediate reuse
                remove_enemy_from_paint_order(i);
                add_enemy_to_front_of_paint_order(i);
                found_enemy = 1;
                break;
            }
        }
    }

    if (found_enemy)
    {
        // Directly update level code structure to remove the enemy
        remove_enemy_from_level_code(x, enemy_row);
        
        // Mark enemy positions for display update
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
    }
}

// ============================================================================
// ENTITY PAINTING
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
    // If this position is not valid, simply return without taking any action
    if (!can_paint_enemy_right(x, y))
    {
        return;
    }

    // Check if there's already an enemy actor at this position
    UBYTE found_enemy = 0;
    for (UBYTE i = 0; i < MAX_PAINT_ENEMIES; i++)
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
                // Found existing enemy - change direction to right
                if (enemy->dir != DIRECTION_RIGHT)
                {
                    // When changing to right direction, remove the offset
                    enemy->pos.x = TO_FP(x * 8);
                    actor_set_dir(enemy, DIRECTION_RIGHT, TRUE);
                    
                    // Directly update level code structure
                    update_enemy_direction_in_level_code(x, y, DIRECTION_RIGHT);
                }
                found_enemy = 1;
                break;
            }
        }
    }

    if (!found_enemy)
    {
        // Don't draw background tile - enemies are actors only

        // Get next enemy from FIFO pool
        UBYTE enemy_slot = get_next_enemy_slot_from_pool();

        // If this slot was already in use, clear its old actor and deactivate it
        if (paint_enemy_slots_used[enemy_slot])
        {
            actor_t *old_enemy = &actors[paint_enemy_ids[enemy_slot]];
            deactivate_actor(old_enemy);
        }

        // Set up the new enemy
        actor_t *enemy = &actors[paint_enemy_ids[enemy_slot]];
        enemy->pos.x = TO_FP(x * 8);
        enemy->pos.y = TO_FP(y * 8);
        activate_actor(enemy);
        actor_set_dir(enemy, DIRECTION_RIGHT, TRUE);
        paint_enemy_slots_used[enemy_slot] = 1; // Mark slot as used

        // Directly update level code structure
        add_enemy_to_level_code(x, y, DIRECTION_RIGHT);
    }
    
    // Mark enemy positions for display update
    for (UBYTE i = 16; i < 24; i++)
    {
        mark_display_position_for_update(i);
    }
    display_selective_level_code_fast();
}

void paint_enemy_left(UBYTE x, UBYTE y) BANKED
{
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    // Check if there's already an enemy actor at this position
    UBYTE found_enemy = 0;
    for (UBYTE i = 0; i < MAX_PAINT_ENEMIES; i++)
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
                // Found existing enemy - change direction to left
                if (enemy->dir != DIRECTION_LEFT)
                {
                    // When changing to left direction, offset the actor position
                    enemy->pos.x = TO_FP(x * 8 - 8);
                    actor_set_dir(enemy, DIRECTION_LEFT, TRUE);
                    
                    // Directly update level code structure
                    update_enemy_direction_in_level_code(x, y, DIRECTION_LEFT);
                }
                found_enemy = 1;
                break;
            }
        }
    }

    if (!found_enemy)
    {
        // If this position is not valid, simply return without taking any action
        if (!can_paint_enemy_right(x, y))
        {
            return;
        }

        // Get next enemy from FIFO pool (no background tile drawing)
        UBYTE enemy_slot = get_next_enemy_slot_from_pool();

        // If this slot was already in use, clear its old actor and deactivate it
        if (paint_enemy_slots_used[enemy_slot])
        {
            actor_t *old_enemy = &actors[paint_enemy_ids[enemy_slot]];
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

        // Directly update level code structure
        add_enemy_to_level_code(x, y, DIRECTION_LEFT);
    }
    
    // Mark enemy positions for display update
    for (UBYTE i = 16; i < 24; i++)
    {
        mark_display_position_for_update(i);
    }
    display_selective_level_code_fast();
}

void delete_enemy(UBYTE x, UBYTE y) BANKED
{
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;

    // Check if there's an enemy actor at this position to delete
    UBYTE found_enemy = 0;
    for (UBYTE i = 0; i < MAX_PAINT_ENEMIES; i++)
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
                found_enemy = 1;
                break;
            }
        }
    }

    if (found_enemy)
    {
        // Directly update level code structure
        remove_enemy_from_level_code(x, y);
        
        // Mark enemy positions for display update
        for (UBYTE i = 16; i < 24; i++)
        {
            mark_display_position_for_update(i);
        }
        display_selective_level_code_fast();
    }
}

// ============================================================================
// EXIT POSITIONING
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

    // Clear any existing exit tiles from the map (to clean up old painted tiles)
    clear_existing_exit_tiles();

    // Position the exit actor only - no background tile painting
    move_actor_to_tile(paint_exit_id, exit_x, exit_y);
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
// DIRECT LEVEL CODE UPDATE FOR ENEMY PAINTING
// ============================================================================

#define LEVEL_CODE_MAX_ENEMIES 5 // Must match the level code structure limit

// Convert tilemap coordinates to enemy position in level code
void add_enemy_to_level_code(UBYTE x, UBYTE y, UBYTE direction) BANKED
{
    // Convert tilemap coordinates to column and row
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;
    
    UBYTE col = x - PLATFORM_X_MIN; // Convert to 0-19 range
    UBYTE row = 255; // Invalid row by default
    
    // Determine which enemy row this corresponds to
    if (y == 12) row = 0;
    else if (y == 14) row = 1;  
    else if (y == 16) row = 2;
    else if (y == 18) row = 3;
    else return; // Not a valid enemy row
    
    // Find an empty enemy slot (only use first 5 slots to match level code)
    UBYTE enemy_slot = 255;
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] == 255)
        {
            enemy_slot = i;
            break;
        }
    }
    
    if (enemy_slot == 255)
        return; // No empty slots available
    
    // Add enemy to level code
    current_level_code.enemy_positions[enemy_slot] = col;
    current_level_code.enemy_rows[enemy_slot] = row;
    
    // Set direction bit (1 = left, 0 = right)
    if (direction == DIRECTION_LEFT)
    {
        current_level_code.enemy_directions |= (1 << enemy_slot);
    }
    else
    {
        current_level_code.enemy_directions &= ~(1 << enemy_slot);
    }
}

// Remove enemy from level code at specific position
void remove_enemy_from_level_code(UBYTE x, UBYTE y) BANKED
{
    // Convert tilemap coordinates to column and row
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;
    
    UBYTE col = x - PLATFORM_X_MIN; // Convert to 0-19 range
    UBYTE row = 255; // Invalid row by default
    
    // Determine which enemy row this corresponds to
    if (y == 12) row = 0;
    else if (y == 14) row = 1;
    else if (y == 16) row = 2;
    else if (y == 18) row = 3;
    else return; // Not a valid enemy row
    
    // Find and remove enemy at this position (only check first 5 slots)
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] == col && 
            current_level_code.enemy_rows[i] == row)
        {
            // Remove enemy
            current_level_code.enemy_positions[i] = 255;
            current_level_code.enemy_rows[i] = 255;
            current_level_code.enemy_directions &= ~(1 << i);
            break;
        }
    }
}

// Update enemy direction in level code
void update_enemy_direction_in_level_code(UBYTE x, UBYTE y, UBYTE direction) BANKED
{
    // Convert tilemap coordinates to column and row
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return;
    
    UBYTE col = x - PLATFORM_X_MIN; // Convert to 0-19 range
    UBYTE row = 255; // Invalid row by default
    
    // Determine which enemy row this corresponds to
    if (y == 12) row = 0;
    else if (y == 14) row = 1;
    else if (y == 16) row = 2;
    else if (y == 18) row = 3;
    else return; // Not a valid enemy row
    
    // Find and update enemy direction at this position (only check first 5 slots)
    for (UBYTE i = 0; i < LEVEL_CODE_MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] == col && 
            current_level_code.enemy_rows[i] == row)
        {
            // Update direction bit (1 = left, 0 = right)
            if (direction == DIRECTION_LEFT)
            {
                current_level_code.enemy_directions |= (1 << i);
            }
            else
            {
                current_level_code.enemy_directions &= ~(1 << i);
            }
            break;
        }
    }
}
