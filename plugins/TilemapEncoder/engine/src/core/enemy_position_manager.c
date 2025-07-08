#pragma bank 253

#include <gbdk/platform.h>
#include "enemy_position_manager.h"
#include "paint.h"

// ============================================================================
// ENEMY POSITION MANAGER - UNIFIED VALIDATION SYSTEM
// ============================================================================

// Pre-calculated valid enemy positions based on platform layout
UBYTE valid_enemy_positions[4][20]; // [row][column] - 1 if valid, 0 if not
UBYTE valid_enemy_positions_count;

// Platform positions cache - updated when platforms change
UBYTE platform_positions[4][20]; // [platform_row][column] - 1 if platform exists

// Enemy position row mapping
const UBYTE ENEMY_ROWS[4] = {12, 14, 16, 18};
const UBYTE PLATFORM_ROWS[4] = {13, 15, 17, 19};

// External function declarations
extern UBYTE get_current_tile_type(UBYTE x, UBYTE y) BANKED;
extern UBYTE is_valid_platform_row(UBYTE y) BANKED;

// ============================================================================
// PLATFORM TRACKING SYSTEM
// ============================================================================

// Update platform positions cache when platforms change
void update_platform_positions(void) BANKED
{
    // Clear platform cache
    for (UBYTE row = 0; row < 4; row++)
    {
        for (UBYTE col = 0; col < 20; col++)
        {
            platform_positions[row][col] = 0;
        }
    }

    // Scan for existing platforms
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE platform_y = PLATFORM_ROWS[row];
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE x = PLATFORM_X_MIN + col;
            UBYTE tile_type = get_current_tile_type(x, platform_y);
            
            if (tile_type == BRUSH_TILE_PLATFORM)
            {
                platform_positions[row][col] = 1;
            }
        }
    }
}

// Check if there's a platform directly below an enemy position
UBYTE has_platform_below_cached(UBYTE enemy_row, UBYTE col) BANKED
{
    if (enemy_row >= 4 || col >= 20)
        return 0;
    
    return platform_positions[enemy_row][col];
}

// ============================================================================
// ENEMY POSITION VALIDATION
// ============================================================================

// Check if a position is valid for enemy placement (unified logic)
UBYTE is_valid_enemy_position_unified(UBYTE x, UBYTE y) BANKED
{
    // Convert to array indices
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return 0;
    
    UBYTE col = x - PLATFORM_X_MIN;
    UBYTE enemy_row = 255;
    
    // Find which enemy row this corresponds to
    for (UBYTE i = 0; i < 4; i++)
    {
        if (y == ENEMY_ROWS[i])
        {
            enemy_row = i;
            break;
        }
    }
    
    if (enemy_row == 255)
        return 0; // Not a valid enemy row
    
    // Must have platform directly below
    if (!has_platform_below_cached(enemy_row, col))
        return 0;
    
    // Position must be empty
    UBYTE current_tile_type = get_current_tile_type(x, y);
    if (current_tile_type != BRUSH_TILE_EMPTY)
        return 0;
    
    // Cannot be directly below player
    UBYTE player_x = current_level_code.player_column + PLATFORM_X_MIN;
    if (x == player_x)
        return 0;
    
    // Cannot have another enemy at adjacent positions
    if (has_enemy_at_adjacent_positions(x, y))
        return 0;
    
    return 1;
}

// Check for enemies at adjacent positions (left/right only, not same position)
UBYTE has_enemy_at_adjacent_positions(UBYTE x, UBYTE y) BANKED
{
    // Check for enemies at x-1 and x+1 (adjacent positions)
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] == 255)
            continue; // No enemy in this slot
        
        UBYTE enemy_x = PLATFORM_X_MIN + current_level_code.enemy_positions[i];
        UBYTE enemy_row = current_level_code.enemy_rows[i];
        UBYTE enemy_y = ENEMY_ROWS[enemy_row];
        
        // Check if enemy is on the same row and adjacent
        if (enemy_y == y)
        {
            if (enemy_x == x - 1 || enemy_x == x + 1)
            {
                return 1; // Found adjacent enemy
            }
        }
    }
    
    return 0;
}

// Check for enemy at exact position (for replacement/flipping)
UBYTE has_enemy_at_exact_position(UBYTE x, UBYTE y) BANKED
{
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] == 255)
            continue;
        
        UBYTE enemy_x = PLATFORM_X_MIN + current_level_code.enemy_positions[i];
        UBYTE enemy_row = current_level_code.enemy_rows[i];
        UBYTE enemy_y = ENEMY_ROWS[enemy_row];
        
        if (enemy_x == x && enemy_y == y)
        {
            return 1;
        }
    }
    
    return 0;
}

// Check for enemy at exact position, excluding a specific enemy index (for movement validation)
UBYTE has_enemy_at_exact_position_excluding(UBYTE x, UBYTE y, UBYTE exclude_enemy_index) BANKED
{
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        // Skip the enemy we're moving
        if (i == exclude_enemy_index)
            continue;
            
        if (current_level_code.enemy_positions[i] == 255)
            continue;
        
        UBYTE enemy_x = PLATFORM_X_MIN + current_level_code.enemy_positions[i];
        UBYTE enemy_row = current_level_code.enemy_rows[i];
        UBYTE enemy_y = ENEMY_ROWS[enemy_row];
        
        if (enemy_x == x && enemy_y == y)
        {
            return 1; // Found another enemy at this position
        }
    }
    
    return 0;
}

// Check if a position is valid for a specific enemy placement (prevents stacking)
UBYTE is_valid_enemy_position_for_enemy(UBYTE x, UBYTE y, UBYTE enemy_index) BANKED
{
    // Basic validation using unified logic
    if (!is_valid_enemy_position_unified(x, y))
    {
        // Allow replacement of existing enemy at exact position
        if (has_enemy_at_exact_position(x, y))
        {
            // Check if it's the same enemy being moved
            if (current_level_code.enemy_positions[enemy_index] != 255)
            {
                UBYTE current_x = PLATFORM_X_MIN + current_level_code.enemy_positions[enemy_index];
                UBYTE current_y = ENEMY_ROWS[current_level_code.enemy_rows[enemy_index]];
                if (current_x == x && current_y == y)
                {
                    return 1; // Same enemy, allow it
                }
            }
            return 0; // Different enemy already there
        }
        return 0;
    }
    
    // Check for conflicts with other enemies (excluding the one being moved)
    if (has_enemy_at_exact_position_excluding(x, y, enemy_index))
        return 0;
    
    return 1;
}

// ============================================================================
// VALID POSITIONS SYSTEM
// ============================================================================

// Update the valid enemy positions matrix
void update_valid_enemy_positions_unified(void) BANKED
{
    // First update platform positions
    update_platform_positions();
    
    // Clear valid positions
    valid_enemy_positions_count = 0;
    for (UBYTE row = 0; row < 4; row++)
    {
        for (UBYTE col = 0; col < 20; col++)
        {
            valid_enemy_positions[row][col] = 0;
        }
    }
    
    // Calculate valid positions
    for (UBYTE row = 0; row < 4; row++)
    {
        UBYTE y = ENEMY_ROWS[row];
        for (UBYTE col = 0; col < 20; col++)
        {
            UBYTE x = PLATFORM_X_MIN + col;
            
            if (is_valid_enemy_position_unified(x, y))
            {
                valid_enemy_positions[row][col] = 1;
                valid_enemy_positions_count++;
            }
        }
    }
    
    // Fallback: If no valid positions found, allow basic positioning for code editor
    if (valid_enemy_positions_count == 0)
    {
        for (UBYTE row = 0; row < 4; row++)
        {
            for (UBYTE col = 0; col < 20; col++)
            {
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (is_valid_enemy_position_for_code(x, y))
                {
                    valid_enemy_positions[row][col] = 1;
                    valid_enemy_positions_count++;
                }
            }
        }
    }
}

// Get the next valid enemy position for cycling in level code editor
UBYTE get_next_valid_enemy_position(UBYTE current_row, UBYTE current_col, UBYTE *next_row, UBYTE *next_col) BANKED
{
    // Start searching from the position after current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (current_row + row_offset) % 4;
        UBYTE start_col = (row_offset == 0) ? current_col + 1 : 0;
        
        for (UBYTE col = start_col; col < 20; col++)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if any enemy is already at this position
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position(x, y))
                {
                    *next_row = row;
                    *next_col = col;
                    return 1; // Found next position with no enemy conflict
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// Get the previous valid enemy position for cycling in level code editor
UBYTE get_prev_valid_enemy_position(UBYTE current_row, UBYTE current_col, UBYTE *prev_row, UBYTE *prev_col) BANKED
{
    // Start searching backward from the position before current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (4 + current_row - row_offset) % 4;
        BYTE end_col = (row_offset == 0) ? ((BYTE)current_col - 1) : 19;
        
        for (BYTE col = end_col; col >= 0; col--)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if any enemy is already at this position
                UBYTE x = PLATFORM_X_MIN + (UBYTE)col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position(x, y))
                {
                    *prev_row = row;
                    *prev_col = (UBYTE)col;
                    return 1; // Found previous position with no enemy conflict
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// Get the next valid enemy position for a specific enemy (prevents stacking)
UBYTE get_next_valid_enemy_position_for_enemy(UBYTE current_row, UBYTE current_col, UBYTE enemy_index, UBYTE *next_row, UBYTE *next_col) BANKED
{
    // Start searching from the position after current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (current_row + row_offset) % 4;
        UBYTE start_col = (row_offset == 0) ? current_col + 1 : 0;
        
        for (UBYTE col = start_col; col < 20; col++)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if this position is valid for this specific enemy
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (is_valid_enemy_position_for_enemy(x, y, enemy_index))
                {
                    *next_row = row;
                    *next_col = col;
                    return 1; // Found next position
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// Get the previous valid enemy position for a specific enemy (prevents stacking)
UBYTE get_prev_valid_enemy_position_for_enemy(UBYTE current_row, UBYTE current_col, UBYTE enemy_index, UBYTE *prev_row, UBYTE *prev_col) BANKED
{
    // Start searching backward from the position before current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (4 + current_row - row_offset) % 4;
        BYTE end_col = (row_offset == 0) ? ((BYTE)current_col - 1) : 19;
        
        for (BYTE col = end_col; col >= 0; col--)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if this position is valid for this specific enemy
                UBYTE x = PLATFORM_X_MIN + (UBYTE)col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (is_valid_enemy_position_for_enemy(x, y, enemy_index))
                {
                    *prev_row = row;
                    *prev_col = (UBYTE)col;
                    return 1; // Found previous position
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// Get the next valid enemy position for a specific enemy (allows current position, prevents other enemies)
UBYTE get_next_valid_enemy_position_for_specific_enemy(UBYTE current_row, UBYTE current_col, UBYTE enemy_index, UBYTE *next_row, UBYTE *next_col) BANKED
{
    // Start searching from the position after current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (current_row + row_offset) % 4;
        UBYTE start_col = (row_offset == 0) ? current_col + 1 : 0;
        
        for (UBYTE col = start_col; col < 20; col++)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if any OTHER enemy is at this position (exclude current enemy)
                UBYTE x = PLATFORM_X_MIN + col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position_excluding(x, y, enemy_index))
                {
                    *next_row = row;
                    *next_col = col;
                    return 1; // Found next position with no other enemy conflict
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// Get the previous valid enemy position for a specific enemy (allows current position, prevents other enemies)
UBYTE get_prev_valid_enemy_position_for_specific_enemy(UBYTE current_row, UBYTE current_col, UBYTE enemy_index, UBYTE *prev_row, UBYTE *prev_col) BANKED
{
    // Start searching backward from the position before current
    for (UBYTE row_offset = 0; row_offset < 4; row_offset++)
    {
        UBYTE row = (4 + current_row - row_offset) % 4;
        BYTE end_col = (row_offset == 0) ? ((BYTE)current_col - 1) : 19;
        
        for (BYTE col = end_col; col >= 0; col--)
        {
            if (valid_enemy_positions[row][col])
            {
                // Check if any OTHER enemy is at this position (exclude current enemy)
                UBYTE x = PLATFORM_X_MIN + (UBYTE)col;
                UBYTE y = ENEMY_ROWS[row];
                
                if (!has_enemy_at_exact_position_excluding(x, y, enemy_index))
                {
                    *prev_row = row;
                    *prev_col = (UBYTE)col;
                    return 1; // Found previous position with no other enemy conflict
                }
            }
        }
    }
    
    return 0; // No valid position found
}

// ============================================================================
// POSITION CONVERSION UTILITIES
// ============================================================================

// Convert tilemap coordinates to array indices
void tilemap_to_indices(UBYTE x, UBYTE y, UBYTE *row, UBYTE *col) BANKED
{
    *col = x - PLATFORM_X_MIN;
    *row = 255;
    
    for (UBYTE i = 0; i < 4; i++)
    {
        if (y == ENEMY_ROWS[i])
        {
            *row = i;
            break;
        }
    }
}

// Convert array indices to tilemap coordinates
void indices_to_tilemap(UBYTE row, UBYTE col, UBYTE *x, UBYTE *y) BANKED
{
    *x = PLATFORM_X_MIN + col;
    *y = (row < 4) ? ENEMY_ROWS[row] : 12;
}

// Convert row/col to POS41 value for level code
UBYTE indices_to_pos41(UBYTE row, UBYTE col, UBYTE odd_bit) BANKED
{
    // Suppress unused parameter warning
    (void)odd_bit;
    
    if (row >= 4 || col >= 20)
        return 0; // Invalid
    
    UBYTE anchor = col / 2;
    return 1 + row * 10 + anchor;
}

// Convert POS41 value to row/col
void pos41_to_indices(UBYTE pos_value, UBYTE odd_bit, UBYTE *row, UBYTE *col) BANKED
{
    if (pos_value == 0)
    {
        *row = 255;
        *col = 255;
        return;
    }
    
    UBYTE v = pos_value - 1; // 0-39
    *row = v / 10;           // 0-3
    UBYTE anchor = v % 10;   // 0-9
    *col = anchor * 2 + odd_bit;
}

// ============================================================================
// INTEGRATION WITH PLATFORM SYSTEM
// ============================================================================

// Called when a platform is added or removed
void on_platform_changed(UBYTE x, UBYTE y) BANKED
{
    // Suppress unused parameter warning - x coordinate not needed for full recalculation
    (void)x;
    
    // Only update if this affects enemy positioning
    if (y == 13 || y == 15 || y == 17 || y == 19)
    {
        update_valid_enemy_positions_unified();
    }
}

// Called when paint tool places/removes enemies - validates and corrects positions
UBYTE validate_enemy_placement(UBYTE x, UBYTE y) BANKED
{
    return is_valid_enemy_position_unified(x, y) || has_enemy_at_exact_position(x, y);
}

// Enhanced validation for paint tool when enemy index is known
UBYTE validate_enemy_placement_for_enemy(UBYTE x, UBYTE y, UBYTE enemy_index) BANKED
{
    return is_valid_enemy_position_for_enemy(x, y, enemy_index);
}

// Simplified validation for code-based enemy placement (less restrictive)
UBYTE is_valid_enemy_position_for_code(UBYTE x, UBYTE y) BANKED
{
    // Basic bounds check
    if (x < PLATFORM_X_MIN || x > PLATFORM_X_MAX)
        return 0;
    
    // Check if it's a valid enemy row
    UBYTE is_enemy_row = 0;
    for (UBYTE i = 0; i < 4; i++)
    {
        if (y == ENEMY_ROWS[i])
        {
            is_enemy_row = 1;
            break;
        }
    }
    
    if (!is_enemy_row)
        return 0;
    
    // For code-based placement, we allow placement even without platforms
    // The paint system will handle proper platform validation
    return 1;
}

// ============================================================================
