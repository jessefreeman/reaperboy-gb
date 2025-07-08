# Enemy Positioning Consistency and Spacing Validation Fix

## Problem
1. **Inconsistent Positioning**: When editing level code, enemies were being positioned using offset calculations (`PLATFORM_Y_MIN + row * SEGMENT_HEIGHT`) instead of the direct row-to-Y mapping used by the paint system.
2. **Weak Spacing Validation**: The level code editing validation was not properly enforcing the 1-tile minimum gap between enemies, allowing invalid placements.

## Solution

### Fixed Enemy Positioning in Level Code Editing
Updated `decode_enemy_position()` and `restore_enemy_actors_from_level_code()` to use the same consistent row-to-Y mapping as the paint system:

```c
// Use the same row-to-Y mapping as the paint system (no offset)
UBYTE actual_y;
if (row == 0)
    actual_y = 12;
else if (row == 1)
    actual_y = 14;
else if (row == 2)
    actual_y = 16;
else
    actual_y = 18;
```

This ensures that:
- Both left and right-facing enemies are positioned at the exact same tile coordinates
- No offset calculations that could cause positioning discrepancies
- Level code editing and paint tool use identical positioning logic

### Enhanced Spacing Validation
Updated `handle_enemy_data_edit()` to use `is_position_valid_for_enemy()` instead of `is_valid_enemy_position()`:

```c
// Use enemy-specific validation that properly handles spacing
if (is_position_valid_for_enemy(rel_index, x, y))
{
    // Position is valid, use it
    enemy_values[rel_index] = new_value;
}
else
{
    // Invalid position, use validation system to find a valid position
    enemy_values[rel_index] = get_valid_enemy_pos41(rel_index, new_value);
}
```

Benefits:
- Avoids circular dependency issues with actor detection
- Properly validates that the new position maintains 1-tile minimum gap from other enemies
- Uses the same validation logic as the paint system
- Automatically finds alternative valid positions when user input is invalid

## Files Modified
- `plugins/TilemapEncoder/engine/src/core/code_enemy_system.c`
  - Fixed positioning in `decode_enemy_position()`
  - Fixed positioning in `restore_enemy_actors_from_level_code()`
  - Enhanced validation in `handle_enemy_data_edit()`

## Result
- Enemy positioning is now 100% consistent between paint tool and level code editing
- Level code editing properly enforces 1-tile minimum spacing between enemies
- No more offset-related positioning bugs when flipping or editing enemies
- All enemy placement follows the same strict validation rules regardless of input method
