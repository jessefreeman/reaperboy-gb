# Paint Logic Toggle Fix - Critical Update

## Problem Identified

When the `paint()` function created a 2-tile platform, our pattern application logic was inadvertently deleting it immediately afterward.

### Root Cause

The `paint()` function **toggles** platform tiles:

- If you call `paint(x, y)` on an empty tile → creates a platform
- If you call `paint(x, y)` on a platform tile → **deletes** the platform

### Specific Issue Scenario

1. **Pattern calls `paint()` at position 4** (rightmost tile)
2. **`paint()` creates a 2-tile platform** extending from position 4 to 5
3. **If pattern has a tile at position 5** in the next iteration
4. **Pattern calls `paint()` at position 5**
5. **`paint()` sees existing platform and deletes it** ❌

This caused the auto-completed 2-tile platforms to be immediately destroyed.

## Solution Implemented

Added a check before calling `paint()` to skip tiles that already have the correct platform:

```c
// Process tiles from left to right (position 0 to 4)
for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
{
    if ((row_pattern >> (4 - i)) & 1)
    {
        UBYTE tile_x = segment_x + i;

        // Only call paint() if there isn't already a platform tile here
        // This prevents deleting platforms that were auto-created by previous paint() calls
        if (get_current_tile_type(tile_x, current_y) != BRUSH_TILE_PLATFORM)
        {
            // Call the exact same paint function that manual clicking uses
            paint(tile_x, current_y);
        }
    }
}
```

## How the Fix Works

### ✅ **Smart Platform Detection**

- Before calling `paint()`, check if there's already a platform at that position
- If platform exists and pattern wants a platform there → **Skip the paint call**
- If no platform exists and pattern wants a platform there → **Call paint() to create it**

### ✅ **Preserves Auto-Completion**

- When `paint()` creates a 2-tile platform, both tiles are placed
- Subsequent pattern positions that overlap the auto-completed area are skipped
- The auto-completed platform remains intact

### ✅ **Maintains All Paint Benefits**

- Still calls the real `paint()` function for actual platform creation
- Still gets all the manual paint features: merging, styling, level code updates
- Just prevents unnecessary calls that would delete existing correct platforms

## Example Scenarios

### **Pattern 1 (`1`) - Single Platform at Position 4**

**Before Fix:**

1. Call `paint()` at position 4
2. `paint()` creates 2-tile platform at positions 4-5
3. Pattern has no more tiles → **Success** ✅

**After Fix:**

1. Call `paint()` at position 4
2. `paint()` creates 2-tile platform at positions 4-5
3. Pattern has no more tiles → **Success** ✅
4. _No change needed for this pattern_

### **Pattern with Adjacent Tiles**

**Before Fix:**

1. Call `paint()` at position 3
2. `paint()` creates 2-tile platform at positions 3-4
3. Call `paint()` at position 4 _(pattern also wants tile here)_
4. `paint()` sees existing platform and **deletes it** ❌
5. **Platform destroyed!**

**After Fix:**

1. Call `paint()` at position 3
2. `paint()` creates 2-tile platform at positions 3-4
3. Check position 4: **Platform already exists** → **Skip paint call**
4. **Platform preserved** ✅

## Benefits

### 🎯 **Idempotent Behavior**

- Calling the pattern application multiple times produces the same result
- No more toggle-based destruction of correct platforms

### 🎯 **Respects Auto-Completion**

- When manual paint logic creates multi-tile platforms, they stay created
- Pattern application works with auto-completion instead of against it

### 🎯 **Maintains Manual Paint Fidelity**

- Still uses the real `paint()` function for all actual changes
- Preserves all the benefits of true manual paint simulation
- Just prevents redundant calls that would cause problems

## Files Modified

- `plugins/TilemapEditor/engine/src/core/code_gen.c` - Added platform existence check before `paint()` calls

## Dependencies

- `get_current_tile_type(UBYTE x, UBYTE y)` - To check existing tile types
- `BRUSH_TILE_PLATFORM` - Platform tile type constant

Both already available through existing includes.

## Result

Pattern application now works correctly with auto-completion:

- ✅ Single platforms auto-complete to 2-tile platforms
- ✅ Auto-completed platforms remain intact
- ✅ Pattern application is idempotent and predictable
- ✅ All manual paint benefits preserved (merging, styling, level code updates)

This fix ensures that the manual paint simulation truly works as intended, respecting the intelligent auto-completion behavior without accidentally undoing it.
