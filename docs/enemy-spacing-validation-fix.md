# Enemy Spacing Validation Fix

## Problem
The enemy spacing validation (1-tile minimum gap between enemies) was not working because the validation functions were still checking for background tiles instead of enemy actors.

## Root Cause
The `has_enemy_nearby()` function was checking for `BRUSH_TILE_ENEMY_L` and `BRUSH_TILE_ENEMY_R` background tiles, but since enemies are now actors only, this always returned `false`, effectively disabling spacing validation.

## Solution

### 1. Fixed `has_enemy_nearby()` function in `paint_entity.c`

**Before** (tile-based):
```c
UBYTE current_tile = get_current_tile_type(x, y);
if (current_tile == BRUSH_TILE_ENEMY_L || current_tile == BRUSH_TILE_ENEMY_R)
    return 1;
```

**After** (actor-based):
```c
if (has_enemy_actor_at_position(x, y))
    return 1;
```

### 2. Updated validation logic for enemy flipping

Modified `can_paint_enemy_right()` to allow painting over existing enemies for flipping:
```c
// Exception: if there's already an enemy at this exact position, allow it (for flipping)
if (has_enemy_nearby(x, y) && !has_enemy_actor_at_position(x, y))
    return 0;
```

### 3. Fixed platform deletion validation

Updated `remove_enemies_above_deleted_platform()` in `code_enemy_system_validation.c` to use actor-based detection instead of tile-based detection.

## Validation Rules Now Enforced

### Paint Tool:
1. **New enemy placement**: Must have at least 1 tile gap from any existing enemy
2. **Enemy flipping**: Allowed on existing enemy positions (ignores spacing for same position)
3. **Enemy deletion**: Allowed regardless of spacing

### Level Code Editing:
1. **Manual position changes**: Uses same `has_enemy_nearby()` validation
2. **Position validation**: Automatically finds valid positions if invalid ones are entered
3. **Platform deletion**: Automatically removes enemies above deleted platforms

### Spacing Check Pattern:
```
Current position: X = enemy, O = empty, ? = checking

? O ?    <- Check left and right neighbors
O X O    <- Current enemy position  
? ? ?    <- Platform below (not checked for spacing)
```

## Files Modified
- `plugins/TilemapPainter/engine/src/core/paint_entity.c`
  - `has_enemy_nearby()` - Updated to use actor detection
  - `can_paint_enemy_right()` - Added exception for enemy flipping
- `plugins/TilemapEncoder/engine/src/core/code_enemy_system_validation.c`
  - `remove_enemies_above_deleted_platform()` - Updated to use actor detection
  - Added external declaration for `has_enemy_actor_at_position()`

## Expected Behavior
- ✅ Enemies cannot be placed adjacent to each other (1-tile minimum gap)
- ✅ Existing enemies can be flipped in place without spacing restrictions
- ✅ Brush preview respects spacing validation
- ✅ Level code editing validates spacing and suggests valid positions
- ✅ Platform deletion removes enemies above and maintains spacing rules

## Testing Checklist
- [ ] Try placing enemies next to each other - should be blocked
- [ ] Try flipping existing enemy direction - should work
- [ ] Delete platform with enemy above - enemy should be removed
- [ ] Edit level code with invalid adjacent positions - should auto-correct
- [ ] Brush should show unavailable when hovering next to existing enemies

The 1-tile spacing rule is now properly enforced in both paint mode and level code editing mode using the new actor-based enemy system.
