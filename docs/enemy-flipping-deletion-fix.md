# Enemy Flipping and Deletion Fix - Implementation Details

## Problem Identified
After the refactoring to make level code the single source of truth for enemy data, the enemy flipping and deletion functionality broke because:

1. **Core painting logic** was checking for `BRUSH_TILE_ENEMY_R` and `BRUSH_TILE_ENEMY_L` background tiles
2. **Validation functions** were rejecting painting over "occupied" positions 
3. Since enemies are now stored only as actors (not background tiles), these checks always failed

## Solution Implemented

### 1. Added Enemy Actor Detection Functions
**File**: `paint_entity.c` and `paint_entity.h`

```c
// Check if there's an enemy actor at the specified position
UBYTE has_enemy_actor_at_position(UBYTE x, UBYTE y) BANKED;

// Get the direction of an enemy actor at the specified position  
UBYTE get_enemy_actor_direction_at_position(UBYTE x, UBYTE y) BANKED;
```

These functions:
- Iterate through active enemy actors
- Convert actor positions from fixed-point to tile coordinates
- Handle left-facing enemy offset correction (+1 tile)
- Return presence and direction information

### 2. Updated Core Painting Logic
**File**: `paint_core.c`

**Before** (tile-based):
```c
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
```

**After** (actor-based):
```c
if (has_enemy_actor_at_position(x, y))
{
    UBYTE enemy_dir = get_enemy_actor_direction_at_position(x, y);
    if (enemy_dir == DIRECTION_RIGHT)
    {
        paint_enemy_left(x, y);
        return;
    }
    else if (enemy_dir == DIRECTION_LEFT)
    {
        delete_enemy(x, y);
        return;
    }
}
```

### 3. Fixed Validation Logic
**File**: `paint_entity.c` - `can_paint_enemy_right()`

**Before**:
```c
if (get_current_tile_type(x, y) != BRUSH_TILE_EMPTY)
    return 0;
```

**After**:
```c
// Allow painting over enemies (for flipping/deleting) or empty tiles
UBYTE current_tile = get_current_tile_type(x, y);
if (current_tile != BRUSH_TILE_EMPTY && !has_enemy_actor_at_position(x, y))
    return 0;
```

## Behavior Restored

### Enemy Painting Cycle
1. **First click**: Paint right-facing enemy → Creates actor + updates level code
2. **Second click** (same position): Flip to left-facing → Updates actor direction + level code  
3. **Third click** (same position): Delete enemy → Removes actor + updates level code
4. **Fourth click** (same position): Paint new right-facing enemy (cycle repeats)

### Technical Flow
1. **Click detection**: `paint_core.c` receives paint command
2. **Actor check**: `has_enemy_actor_at_position()` detects existing enemy
3. **Direction check**: `get_enemy_actor_direction_at_position()` determines current direction
4. **State transition**: Calls appropriate function (`paint_enemy_left()` or `delete_enemy()`)
5. **Level code update**: Each function updates both actor and level code data

## Files Modified
- `plugins/TilemapPainter/engine/src/core/paint_entity.c`
- `plugins/TilemapPainter/engine/include/paint_entity.h`  
- `plugins/TilemapPainter/engine/src/core/paint_core.c`

## Compatibility Notes
- Brush interface (`BRUSH_TILE_ENEMY_L/R`) remains unchanged for UI purposes
- Background tile system still used for platforms, player, and other elements
- Only enemy storage/detection logic was changed to use actors
- All existing enemy painting functions (`paint_enemy_right()`, `paint_enemy_left()`, `delete_enemy()`) work unchanged

## Testing Checklist
- [x] Enemy flipping: right → left → delete → new right
- [x] Actor position detection with left-facing offset handling
- [x] Level code updates during each state transition  
- [x] Validation allows painting over existing enemies
- [x] No compilation errors
- [ ] End-to-end testing in GB Studio environment

The fix restores the expected enemy interaction behavior while maintaining the new actor-based architecture and level code as the single source of truth.
