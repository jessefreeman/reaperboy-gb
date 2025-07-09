#pragma bank 253

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"
#include "paint.h"
#include "code_enemy_system_validation.h"
#include "enemy_position_manager.h"

// ============================================================================
// ENEMY POSITION VALIDATION SYSTEM - Updated to use unified manager
// ============================================================================

// External function declarations
extern UBYTE get_enemy_row_from_position(UBYTE enemy_index) BANKED;
extern void delete_enemy(UBYTE x, UBYTE y) BANKED;
extern UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED;
extern UBYTE is_valid_platform_row(UBYTE y) BANKED;
extern UBYTE has_enemy_nearby(UBYTE x, UBYTE y) BANKED;
extern UBYTE has_enemy_actor_at_position(UBYTE x, UBYTE y) BANKED;

// Valid enemy positions tracking - now managed by unified system
extern UBYTE valid_enemy_positions[4][20];
extern UBYTE valid_enemy_positions_count;

// Use shared enemy position constants from enemy position manager

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert from level row (0-3) to actual y position (12, 14, 16, 18)
UBYTE get_enemy_y_from_row(UBYTE row) BANKED
{
    if (row < 4)
        return ENEMY_ROWS[row];
    return 12; // Default to first row if invalid
}

// Convert from actual y position to level row (0-3)
UBYTE get_enemy_row_from_y(UBYTE y) BANKED
{
    for (UBYTE i = 0; i < 4; i++)
    {
        if (ENEMY_ROWS[i] == y)
            return i;
    }
    return 0; // Default to row 0 if not found
}

// Check if a tile position has a platform directly below
UBYTE has_platform_below_enemy(UBYTE x, UBYTE y) BANKED
{
    // Find the immediate platform row below this enemy row
    UBYTE platform_row = y + 1; // Platform row is directly below enemy row

    // Verify this is a valid row for a platform
    if (!is_valid_platform_row(platform_row))
        return 0; // Not a valid platform row

    // Check if there's a platform tile directly at this position
    UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(x, platform_row)];
    UBYTE tile_type = get_tile_type(tile);

    // Must be exactly a platform tile (not any tile)
    return (tile_type == BRUSH_TILE_PLATFORM);
}

// Check if a position is directly below the player
UBYTE is_below_player(UBYTE x) BANKED
{
    // Player is at column 2-21 in tilemap, corresponding to 0-19 in level code
    UBYTE player_x = current_level_code.player_column + 2;
    return (x == player_x);
}

// Check if a position is above the exit
UBYTE is_above_exit(UBYTE x, UBYTE y) BANKED
{
    // Exit is typically at the right edge of platforms
    // For now, we'll just check player column + 1 (simple approximation)
    UBYTE exit_x = current_level_code.player_column + 3; // Player + 1 tile

    // Check all rows below this position for exit
    for (UBYTE check_y = y + 1; check_y <= PLATFORM_Y_MAX; check_y++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(exit_x, check_y)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_EXIT)
            return (x == exit_x);
    }

    return 0;
}

// ============================================================================
// VALIDATION SYSTEM
// ============================================================================

// Check if a position is valid for enemy placement
UBYTE is_valid_enemy_position(UBYTE x, UBYTE y) BANKED
{
    // Use the unified validation system
    return is_valid_enemy_position_unified(x, y);
}

// Check if a position is valid for an enemy at a specific index
UBYTE is_position_valid_for_enemy(UBYTE enemy_index, UBYTE x, UBYTE y) BANKED
{
    // Temporarily clear this enemy's position to avoid self-conflict
    UBYTE old_pos = current_level_code.enemy_positions[enemy_index];
    UBYTE old_row = current_level_code.enemy_rows[enemy_index];
    
    // Clear position temporarily
    current_level_code.enemy_positions[enemy_index] = 255;
    current_level_code.enemy_rows[enemy_index] = 255;
    
    // Check if position is valid
    UBYTE is_valid = is_valid_enemy_position_unified(x, y);
    
    // Restore original position
    current_level_code.enemy_positions[enemy_index] = old_pos;
    current_level_code.enemy_rows[enemy_index] = old_row;
    
    return is_valid;
}

// ============================================================================
// POSITION TRACKING SYSTEM
// ============================================================================

// Initialize the valid enemy position tracking system
void init_valid_enemy_positions(void) BANKED
{
    // Clear all tracking arrays
    for (UBYTE row = 0; row < 4; row++)
    {
        for (UBYTE col = 0; col < 20; col++)
        {
            valid_enemy_positions[row][col] = 0;
        }
    }

    valid_enemy_positions_count = 0;

    // Scan the level for valid enemy positions using unified system
    update_valid_enemy_positions_unified();
}

// Update valid enemy positions by scanning the entire level
void update_valid_enemy_positions(void) BANKED
{
    valid_enemy_positions_count = 0;

    // Check all four enemy rows
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE y = get_enemy_y_from_row(row);

        // Check all columns
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE x = PLATFORM_X_MIN + col;

            // First clear this position
            valid_enemy_positions[row][col] = 0;

            // Now check if position is valid - must have a platform directly below
            if (is_valid_enemy_position(x, y))
            {
                valid_enemy_positions[row][col] = 1;
                valid_enemy_positions_count++;
            }
        }
    }
}

// Update valid enemy positions affected by a platform change
void update_enemy_positions_for_platform(UBYTE x, UBYTE y) BANKED
{
    // Use the unified system's platform change handler
    on_platform_changed(x, y);
}

// ============================================================================
// ENEMY POSITION CYCLING
// ============================================================================

// Find the next valid position for an enemy in the level code system
void find_next_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED
{
    // Preserve the odd bit and direction bit - they should not change during cycling
    UBYTE current_odd_bit = *odd_bit;
    
    // Get current position
    UBYTE current_col = (current_level_code.enemy_positions[enemy_index] != 255) ? current_level_code.enemy_positions[enemy_index] : 0;
    UBYTE current_row = get_enemy_row_from_position(enemy_index);
    
    // Cycle through positions that match the current odd bit
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (current_row + row_offset) % 4;
        UBYTE start_col = (row_offset == 0) ? current_col + 1 : 0;
        
        for (UBYTE col = start_col; col < 20; col++)
        {
            // Only consider columns that match the current odd bit
            if ((col % 2) != current_odd_bit)
                continue;
            
            // Check if this position is in the valid positions matrix
            if (valid_enemy_positions[row][col])
            {
                // Check if any OTHER enemy is at this position
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position_excluding(x, y, enemy_index))
                {
                    // Found a valid position with the same odd bit
                    UBYTE anchor = col / 2;
                    *pos_value = 1 + row * 10 + anchor;
                    // *odd_bit and *dir_bit remain unchanged
                    return;
                }
            }
        }
    }
    
    // No valid position found
    *pos_value = 0;
}

// Find the previous valid position for an enemy in the level code system
void find_prev_valid_enemy_position_in_code(UBYTE enemy_index, UBYTE *pos_value, UBYTE *odd_bit, UBYTE *dir_bit) BANKED
{
    // Preserve the odd bit and direction bit - they should not change during cycling
    UBYTE current_odd_bit = *odd_bit;
    
    // Get current position
    UBYTE current_col = (current_level_code.enemy_positions[enemy_index] != 255) ? current_level_code.enemy_positions[enemy_index] : 19;
    UBYTE current_row = get_enemy_row_from_position(enemy_index);
    
    // Cycle backward through positions that match the current odd bit
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (4 + current_row - row_offset) % 4;
        BYTE end_col = (row_offset == 0) ? ((BYTE)current_col - 1) : 19;
        
        for (BYTE col = end_col; col >= 0; col--)
        {
            // Only consider columns that match the current odd bit
            if (((UBYTE)col % 2) != current_odd_bit)
                continue;
            
            // Check if this position is in the valid positions matrix
            if (valid_enemy_positions[row][(UBYTE)col])
            {
                // Check if any OTHER enemy is at this position
                UBYTE x = PLATFORM_X_MIN + (UBYTE)col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position_excluding(x, y, enemy_index))
                {
                    // Found a valid position with the same odd bit
                    UBYTE anchor = (UBYTE)col / 2;
                    *pos_value = 1 + row * 10 + anchor;
                    // *odd_bit and *dir_bit remain unchanged
                    return;
                }
            }
        }
    }
    
    // No valid position found
    *pos_value = 0;
}

// ============================================================================
// PLATFORM INTERACTION
// ============================================================================

// Remove enemies above a deleted platform
void remove_enemies_above_deleted_platform(UBYTE x, UBYTE y) BANKED
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

    // Check if there's an enemy actor directly above this platform
    if (has_enemy_actor_at_position(x, enemy_row))
    {
        // Remove this enemy - it's directly above the deleted platform
        delete_enemy(x, enemy_row);
    }

    // Always update the valid positions after removing a platform
    update_enemy_positions_for_platform(x, y);
}

// ============================================================================
// INTEGRATION WITH LEVEL CODE SYSTEM
// ============================================================================

// Get a valid POS41 value for an enemy
UBYTE get_valid_enemy_pos41(UBYTE enemy_index, UBYTE current_value) BANKED
{
    if (current_value == 0)
        return 0; // No enemy is always valid

    // Convert POS41 value to row and column
    UBYTE v = current_value - 1; // 0-39
    UBYTE row = v / 10;          // 0-3
    UBYTE anchor = v % 10;       // 0-9

    // Get the odd bit for this enemy
    UBYTE odd_bit = (current_level_code.enemy_directions >> enemy_index) & 1;

    // Calculate actual column: anchor*2 + odd_bit
    UBYTE col = anchor * 2 + odd_bit;

    // Check if this position is valid
    if (row < 4 && col < 20 && valid_enemy_positions[row][col])
        return current_value;

    // If not valid, find next valid position
    UBYTE pos_value = 0;
    UBYTE new_odd_bit = odd_bit;
    UBYTE dir_bit = (current_level_code.enemy_directions >> enemy_index) & 1;

    find_next_valid_enemy_position_in_code(enemy_index, &pos_value, &new_odd_bit, &dir_bit);

    return pos_value;
}
