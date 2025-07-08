# Strict Level Code Validation Fix

## Problem
The previous validation logic was too permissive - when a user tried to set an invalid enemy position (like "P" when no valid position exists), the system would fall back to keeping the old value but still process the change. This created confusion because the user could attempt to make changes that appeared to be accepted but weren't actually valid.

## Root Issue
The validation logic was using a "fallback" approach:
1. Try to validate the new position
2. If invalid, keep the old value but continue processing
3. Apply the changes anyway

This meant users could attempt invalid changes and the system would silently ignore them while still updating the display, making it seem like the change was processed.

## Solution: Strict Rejection

### New Behavior
Now when invalid input is detected, the function **immediately returns without making ANY changes**:

```c
if (is_position_valid_for_enemy(rel_index, x, y))
{
    // Position is valid, use it
    enemy_values[rel_index] = new_value;
}
else
{
    // Invalid position - don't change anything, return early
    return;
}
```

### Key Changes:
1. **Invalid Position**: If the position fails validation, return immediately - no changes applied
2. **Invalid Range**: If POS41 value is >40, return immediately - no changes applied  
3. **Invalid Mask Bits**: If trying to set direction/odd bits for non-existent enemies, return immediately
4. **Out of Bounds**: If row/column calculations are invalid, return immediately

### Mask Validation Enhancement
For direction and odd masks, the system now detects attempts to set bits for non-existent enemies:

```c
for (UBYTE i = 0; i < 5; i++)
{
    if (enemy_values[i] != 0) // Enemy exists
    {
        // Allow the direction bit for this enemy
        if (new_value & (1 << i))
            valid_dir_mask |= (1 << i);
    }
    else if (new_value & (1 << i))
    {
        // Trying to set bit for non-existent enemy - invalid change
        return;
    }
}
```

## Expected Behavior Now

### Before Fix:
- ❌ Could attempt to set enemy 2 to "P" - system would silently keep old value
- ❌ Could set direction bits for non-existent enemies - system would filter them out
- ❌ Invalid changes appeared to be processed but were actually ignored

### After Fix:
- ✅ Attempting to set enemy 2 to "P" when invalid - **entire operation rejected, no changes made**
- ✅ Attempting to set direction bits for non-existent enemies - **entire operation rejected**  
- ✅ Any invalid input immediately stops processing - **clear feedback that change was rejected**
- ✅ Only completely valid changes are accepted and applied

## User Experience Impact
- **Clear Rejection**: Invalid changes are completely rejected rather than silently ignored
- **Consistency**: Same strict validation as paint tool - if you can't paint it, you can't code it
- **No Confusion**: Failed changes don't update anything, making it clear the input was invalid
- **Data Integrity**: Level code can only contain valid, reachable enemy configurations

## Files Modified
- `plugins/TilemapEncoder/engine/src/core/code_enemy_system.c`
  - Changed all validation failures to `return` immediately instead of falling back to old values
  - Enhanced mask validation to detect and reject attempts to set bits for non-existent enemies
  - Ensures strict "all-or-nothing" validation - either the change is completely valid or completely rejected

## Result
Level code editing now enforces the same strict rules as the paint system: **if there's no valid position available, you simply cannot make the change, period.**
