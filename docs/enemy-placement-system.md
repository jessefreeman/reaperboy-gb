# Enemy Placement System Documentation

## Overview

The enemy placement system in the ReaperBoy game engine controls where enemies can be placed in the level editor. This system ensures that enemies are only placed in valid locations that make sense from a gameplay perspective and ensures the level code system works correctly.

## Rules for Enemy Placement

### 1. Platform Requirement

- Enemies can only be placed directly above platform tiles (1:1 relationship)
- An enemy must have a platform tile directly beneath it (in the row immediately below)
- No platform elsewhere in the column will satisfy this requirement
- When a platform is deleted, any enemy directly above it is automatically deleted

### 2. Player and Exit Avoidance

- Enemies cannot be placed directly below the player
- This prevents unfair gameplay where the player would immediately collide with an enemy when dropping down
- Enemies cannot be placed above the exit to avoid blocking the level completion

### 3. Enemy Spacing

- Enemies must be at least one tile apart from each other horizontally
- This includes diagonally adjacent tiles (e.g., an enemy cannot be placed next to another enemy even if on different rows)
- This prevents enemy crowding and ensures clear enemy paths

### 4. Valid Rows

- Enemies are only valid on specific rows (12, 14, 16, 18) corresponding to the platform rows
- These rows are fixed in the engine and correspond to the standard enemy positions

### 5. Level Code Cycling

- When cycling through enemy positions in the level code editor, only valid positions can be selected
- If no valid positions remain, an enemy position cannot be selected (remains at zero/no enemy)

## Implementation Details

### Data Structures

1. **Valid Position Tracking**

   - `valid_enemy_positions[4][20]` - A 2D array tracking valid positions for each row (0-3) and column (0-19)
   - `valid_enemy_positions_count` - Total count of valid positions in the level

2. **Enemy Row Mapping**
   - `ENEMY_ROWS[4]` - Maps internal row indices (0-3) to actual Y positions (12, 14, 16, 18)

### Core Validation Functions

1. **Position Validation**

   - `is_valid_enemy_position(x, y)` - Checks if a position is valid for enemy placement
   - `is_position_valid_for_enemy(enemy_index, x, y)` - Checks if a position is valid for a specific enemy

2. **Platform Dependency**

   - `has_platform_below_enemy(x, y)` - Checks if there's a platform directly below an enemy position (strict 1:1 relationship)
   - `has_platform_below(x, y)` - In paint.c, performs the same strict check for a platform directly below
   - `remove_enemies_above_deleted_platform(x, y)` - Removes enemies when their supporting platform is deleted

3. **Player Avoidance**
   - `is_below_player(x)` - Checks if a position is directly below the player
   - `is_above_exit(x, y)` - Checks if a position is above the exit

### Position Management

1. **Position Calculation**

   - `get_enemy_y_from_row(row)` - Converts from internal row (0-3) to actual Y position (12, 14, 16, 18)
   - `get_enemy_row_from_y(y)` - Converts from actual Y position to internal row (0-3)

2. **Position Tracking**
   - `init_valid_enemy_positions()` - Initializes the validation system
   - `update_valid_enemy_positions()` - Recalculates valid positions for the entire level
   - `update_enemy_positions_for_platform(x, y)` - Updates positions when a platform changes

### Level Code Integration

1. **Position Cycling**

   - `find_next_valid_enemy_position_in_code(enemy_index, &pos_value, &odd_bit, &dir_bit)` - Finds the next valid position in level code
   - `find_prev_valid_enemy_position_in_code(enemy_index, &pos_value, &odd_bit, &dir_bit)` - Finds the previous valid position in level code
   - `find_next_valid_enemy_position(x, y)` - In paint.c, finds the next valid enemy position in the paint system

2. **POS41 Value Handling**
   - `get_valid_enemy_pos41(enemy_index, current_value)` - Gets a valid POS41 value for the level code

## Position Encoding System

The enemy positions are encoded using a numeric system:

1. **POS41 System**

   - Values 0-40 represent positions in the level
   - Value 0 means "no enemy"
   - Values 1-40 encode both row and column information
   - Formula: `pos_value = 1 + row*10 + anchor` where `anchor = col/2`

2. **Odd/Even Column Handling**

   - The "odd_mask" handles columns with odd numbers
   - Each bit represents whether an enemy is in an odd column (0 = even, 1 = odd)

3. **Direction Encoding**
   - The "dir_mask" encodes enemy facing direction
   - Each bit represents enemy direction (0 = right-facing, 1 = left-facing)

## Paint System Integration

1. **Painting Validation**

   - `can_paint_enemy_right(x, y)` - Checks if an enemy can be painted at a position
   - Enhanced with platform dependency checks and player/exit avoidance

2. **Platform Interaction**
   - When platforms are added/deleted, valid enemy positions are recalculated
   - `remove_enemies_above_platform(x, y)` - Called when deleting platforms

## Edge Cases and Limitations

1. **Maximum Enemy Count**

   - Limited to 5 enemies per level due to level code constraints
   - When this limit is reached, the oldest enemy is replaced (FIFO queue)

2. **Platform Length Limitation**

   - Platforms can't exceed 8 tiles, limiting where enemies can be placed
   - This is a hard limit in the engine

3. **Race Conditions**
   - Enemy positions are validated during platform changes to prevent orphaned enemies
   - The system ensures enemies always have a supporting platform

## How It Works During Gameplay

When the player interacts with the level editor:

1. **During Painting**

   - The brush preview only shows enemy options for positions with a platform directly below
   - Enemies automatically cycle through valid positions when placing (or when deleted)
   - When cycling, only positions with a platform directly below, at least 1 tile away from other enemies, and not below player or above exit are considered
   - If a position is invalid, the system automatically finds the next valid position

2. **During Platform Editing**

   - Deleting a platform automatically removes any enemy directly above it (strict 1:1 relationship)
   - Adding a platform creates a new valid position directly above it for enemy placement
   - Enemies can only be placed in positions that have a platform tile directly underneath

3. **During Level Code Editing**
   - When cycling through enemy positions, only positions with platforms directly below are accessible
   - Invalid positions are automatically skipped in favor of the next/previous valid position
   - The system ensures enemies can only be positioned directly above platform tiles (1:1 relationship)
   - When cycling through character values, both position (POS41) and column parity (odd/even) are updated
   - Level code is kept consistent with the visible tilemap

## Initialization

The enemy validation system is initialized:

1. When the editor is first enabled
2. When loading a level
3. After any major level changes

This ensures the validation data is always up-to-date with the current level state.
