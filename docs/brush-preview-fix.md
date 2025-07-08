# Brush Preview Fix for Enemy Actors

## Problem
After refactoring enemies to use actors instead of background tiles, the brush preview system was broken because:

- The `get_brush_tile_state()` function was checking for `BRUSH_TILE_ENEMY_L` and `BRUSH_TILE_ENEMY_R` background tiles
- Since enemies are now actors only, these tile checks always failed
- The brush preview wasn't showing the correct enemy state (right arrow, left arrow, or X for delete)

## Solution

### Modified `get_brush_tile_state()` in `paint_ui.c`

**Before** (tile-based):
```c
case BRUSH_TILE_ENEMY_L:
    return SELECTOR_STATE_DELETE;

case BRUSH_TILE_ENEMY_R:
    return SELECTOR_STATE_ENEMY_LEFT;
```

**After** (actor-based):
```c
// Check for enemy actors first (since they're no longer stored as background tiles)
if (has_enemy_actor_at_position(x, y))
{
    UBYTE enemy_dir = get_enemy_actor_direction_at_position(x, y);
    if (enemy_dir == DIRECTION_RIGHT)
    {
        return SELECTOR_STATE_ENEMY_LEFT; // Next click will flip to left
    }
    else if (enemy_dir == DIRECTION_LEFT)
    {
        return SELECTOR_STATE_DELETE; // Next click will delete
    }
}
```

## Brush Preview States

The function now correctly returns:

### For Enemy Positions:
1. **No enemy present** + valid enemy position → `SELECTOR_STATE_ENEMY_RIGHT` (shows right arrow)
2. **Right-facing enemy present** → `SELECTOR_STATE_ENEMY_LEFT` (shows left arrow - next click flips)  
3. **Left-facing enemy present** → `SELECTOR_STATE_DELETE` (shows X - next click deletes)

### For Other Elements:
- **Empty tile** (non-enemy row) → Platform placement logic
- **Player position** → Player state logic
- **Platform tile** → `SELECTOR_STATE_DELETE` (X for deletion)

## Expected Behavior Restored

### Enemy Brush Preview Cycle:
1. **Empty valid position**: Shows right arrow → User can place right-facing enemy
2. **Right-facing enemy**: Shows left arrow → User can flip to left-facing  
3. **Left-facing enemy**: Shows X → User can delete enemy
4. **After deletion**: Back to right arrow → Cycle repeats

### Visual Feedback:
- ✅ Right arrow when placing new enemy
- ✅ Left arrow when flipping existing right-facing enemy
- ✅ X (delete) when deleting left-facing enemy
- ✅ Preview updates immediately when hovering over enemy positions

## Technical Details

### Functions Used:
- `has_enemy_actor_at_position(x, y)` - Detects if enemy actor exists at coordinates
- `get_enemy_actor_direction_at_position(x, y)` - Returns `DIRECTION_RIGHT` or `DIRECTION_LEFT`

### Constants:
- `SELECTOR_STATE_ENEMY_RIGHT` (2) - Right arrow brush preview
- `SELECTOR_STATE_ENEMY_LEFT` (3) - Left arrow brush preview  
- `SELECTOR_STATE_DELETE` (1) - X/delete brush preview

### Integration:
- Uses the same actor detection functions as the core painting system
- Consistent with the painting logic: right → left → delete → new right
- No additional headers needed (functions already declared in `paint_entity.h`)

## Files Modified
- `plugins/TilemapPainter/engine/src/core/paint_ui.c`

## Testing Checklist
- [x] Brush shows right arrow over empty valid enemy positions
- [x] Brush shows left arrow over right-facing enemies  
- [x] Brush shows X over left-facing enemies
- [x] Preview updates immediately when moving cursor
- [x] No compilation errors
- [ ] End-to-end testing in GB Studio editor

The brush preview system now correctly reflects the current enemy state and shows users what action will be performed on the next click, maintaining consistency with the new actor-based enemy system.
