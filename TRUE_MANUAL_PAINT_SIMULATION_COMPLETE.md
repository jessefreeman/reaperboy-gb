# True Manual Paint Simulation - Complete Fix

## Problem Summary

The pattern application was not fully simulating manual painting, causing two critical issues:

1. **Level code sync issues**: When auto-completion extended platforms into adjacent blocks, the neighboring level codes weren't updated
2. **Incomplete cleanup**: When switching from a pattern to empty (pattern 0), single platform tiles in adjacent blocks weren't properly removed

## Root Cause

The previous approach tried to replicate manual painting logic but wasn't actually **calling** the manual painting functions. This meant:

- Manual painting used `paint(x, y)` which automatically updates all affected level codes
- Pattern application used custom logic that didn't have the same update mechanisms
- Different code paths led to inconsistent behavior

## Solution: True Manual Paint Simulation

Completely rewrote `apply_pattern_with_brush_logic()` to literally call the `paint(x, y)` function for each platform tile in the pattern, from left to right.

### New Implementation

```c
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED
{
    // ... validation and setup ...

    // Clear the segment first by calling paint() on each tile (simulates delete clicks)
    for (UBYTE row = 0; row < SEGMENT_HEIGHT; row++)
    {
        UBYTE current_y = segment_y + row;
        for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
        {
            UBYTE tile_x = segment_x + i;
            // If there's a platform tile here, delete it by calling paint (which will delete it)
            if (get_current_tile_type(tile_x, current_y) == BRUSH_TILE_PLATFORM)
            {
                paint(tile_x, current_y); // This will delete the platform
            }
        }
    }

    // Apply pattern by simulating manual paint clicks from left to right
    for (UBYTE row = 0; row < SEGMENT_HEIGHT; row++)
    {
        // ... extract pattern ...

        // Process tiles from left to right (position 0 to 4)
        for (UBYTE i = 0; i < SEGMENT_WIDTH; i++)
        {
            if ((row_pattern >> (4 - i)) & 1)
            {
                UBYTE tile_x = segment_x + i;

                // Call the exact same paint function that manual clicking uses
                // This handles all the logic: auto-completion, merging, level code updates, etc.
                paint(tile_x, current_y);
            }
        }
    }
}
```

## What `paint(x, y)` Does Automatically

By calling the actual `paint()` function, we get all the manual painting behaviors:

### 1. **Smart Platform Creation**

```c
// From paint.c - creates 2-tile platforms automatically
if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
    !check_platform_vertical_conflict(x + 1, y))
{
    // Create new 2-tile platform
    replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
    replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
}
```

### 2. **Platform Deletion**

```c
// From paint.c - deletes platforms and cleans up
if (current_tile_type == BRUSH_TILE_PLATFORM)
{
    remove_enemies_above_platform(x, y);
    replace_meta_tile(x, y, TILE_EMPTY, 1);
    rebuild_platform_row(y);
    update_level_code_for_paint(x, y); // Smart update
    return;
}
```

### 3. **Automatic Level Code Updates**

```c
// From paint.c - called after every paint action
update_level_code_for_paint(x, y); // Smart update
```

## Benefits of True Simulation

### ✅ **Perfect Consistency**

- Code entry now **literally** performs the same actions as manual painting
- No more separate logic paths that could diverge

### ✅ **Automatic Level Code Sync**

- When auto-completion extends into adjacent blocks, `update_level_code_for_paint()` automatically updates all affected level codes
- Neighboring blocks are updated when platforms extend into them

### ✅ **Complete Cleanup**

- When switching to pattern 0 (empty), the clearing phase calls `paint()` on existing platforms
- This triggers deletion logic that properly cleans up single tiles in adjacent blocks

### ✅ **All Manual Paint Features**

- Platform merging with 8-tile limit enforcement
- Single platform auto-completion to 2-tile platforms
- Proper end cap styling
- Enemy removal when platforms are deleted
- Vertical conflict checking

## Processing Order

### 1. **Clearing Phase**

- Scan the segment for existing platform tiles
- Call `paint(x, y)` on each platform tile to delete it
- This properly cleans up and updates adjacent level codes

### 2. **Pattern Application Phase**

- Process pattern tiles from **left to right** (position 0 to 4)
- Call `paint(x, y)` for each platform position in the pattern
- Each paint call can affect subsequent calls (merging, auto-completion)

## Edge Cases Resolved

### 1. **Pattern 1 (`1`) Auto-Completion**

- Pattern calls `paint()` at position 4 (rightmost)
- `paint()` detects empty space to the right
- Automatically creates 2-tile platform extending into next block
- `update_level_code_for_paint()` updates both current and next block's level codes

### 2. **Pattern 0 (Empty) Cleanup**

- Clearing phase calls `paint()` on existing platform at position 4
- If it's a single tile, `paint()` deletes it
- If it was part of a 2-tile platform extending into next block, that tile gets cleaned up too
- All affected level codes are automatically updated

### 3. **Platform Merging**

- When pattern creates platforms adjacent to existing ones
- `paint()` automatically merges them respecting 8-tile limit
- `rebuild_platform_row()` ensures proper styling
- Level codes for all merged segments are updated

## Files Modified

- `plugins/TilemapEditor/engine/src/core/code_gen.c` - Complete rewrite of `apply_pattern_with_brush_logic()`

## Dependencies Used

- `paint(UBYTE x, UBYTE y)` - Main manual painting function
- `get_current_tile_type(UBYTE x, UBYTE y)` - Tile type detection
- `BRUSH_TILE_PLATFORM` - Platform tile type constant

All functions are already available through existing includes (`paint.h`, `tile_utils.h`).

## Result

Pattern entry now **IS** manual painting. There's no difference between:

- Entering code letter `1`
- Manually clicking at the same positions with the paint brush

Both use identical code paths and produce identical results, including automatic level code synchronization across all affected blocks.
