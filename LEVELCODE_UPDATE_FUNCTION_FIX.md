# Level Code Update Function Fix - Critical Discovery

## Problem Root Cause Discovered

The issue with adjacent block level codes not updating was caused by calling a non-existent function in our pattern application code, and using a function that only updates specific zones rather than all affected zones.

### The Function Mix-Up

In our pattern application, we were calling:

```c
update_level_code_for_paint(check_x, current_y);
```

While this function **does exist** in `paint.c`, it has a very specific behavior:

- It only updates the **specific zone** where the paint action occurred
- It doesn't check or update **adjacent zones** that might have been affected by auto-completion
- It's designed for single-tile paint actions, not pattern-wide changes

### What `update_level_code_for_paint()` Actually Does

Looking at the function in `paint.c` (lines 922-989):

```c
void update_level_code_for_paint(UBYTE x, UBYTE y) BANKED
{
    // ... enemy/player logic ...

    // For platform operations, update affected zones
    if (y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX) {
        // Get the zone index for THIS position only
        UBYTE zone_index = get_zone_index_from_tile(x, y);
        if (zone_index != 255) {
            // Mark only the SPECIFIC zone position for update
            mark_display_position_for_update(zone_index);
            // Extract and update ONLY this zone
            // ...
        }
    }
}
```

**Key insight**: This function is designed for manual painting where you click one tile at a time. It only updates the zone containing the clicked position, not adjacent zones that might be affected by auto-completion.

## Solution: Use Complete Update for Pattern Application

For pattern application, we need to ensure ALL zones are updated to capture any auto-completion effects. The solution is to use `update_complete_level_code()` which:

- Re-extracts platform data for **ALL zones**
- Updates the entire level code display
- Captures any auto-completion or merging effects

### Fixed Code

```c
// After applying the pattern, update all level codes to ensure perfect synchronization
// This captures any auto-completion or platform changes that affected adjacent blocks
update_complete_level_code();
```

## Why This Fix Works

### ✅ **Complete Re-extraction**

- `update_complete_level_code()` calls `extract_platform_data()`
- `extract_platform_data()` scans the entire tilemap and re-extracts patterns for all 16 blocks
- No zone is missed, including adjacent zones affected by auto-completion

### ✅ **Handles All Auto-Completion Cases**

- **Pattern 1**: Creates 2-tile platform extending into next block → Next block's level code updated
- **Platform merging**: When patterns merge with existing platforms across zones → All affected zones updated
- **8-tile limit enforcement**: When platforms are capped or modified → All zones reflect the final state

### ✅ **Performance vs Accuracy Trade-off**

- `update_level_code_for_paint()`: Fast, but only updates one zone
- `update_complete_level_code()`: Slightly slower, but guarantees accuracy
- For pattern application (which is less frequent than individual paint), accuracy is more important

## Example Scenario: Pattern `1` Auto-Completion

### Before Fix (using zone-specific update)

1. Apply pattern `1` to block 0
2. `paint()` creates 2-tile platform: block 0 position 4 + block 1 position 0
3. `update_level_code_for_paint()` only updates block 0 (where paint was initiated)
4. Block 1 level code remains unchanged ❌

### After Fix (using complete update)

1. Apply pattern `1` to block 0
2. `paint()` creates 2-tile platform: block 0 position 4 + block 1 position 0
3. `update_complete_level_code()` re-extracts patterns for ALL blocks
4. Block 0 shows pattern `1`, Block 1 shows pattern `2` (left edge platform) ✅

## Manual Painting vs Pattern Application

### Manual Painting

- Uses `update_level_code_for_paint()` → Updates only the clicked zone
- This works fine because manual painting is one tile at a time
- Auto-completion effects are usually within the same zone or immediately adjacent

### Pattern Application

- Uses `update_complete_level_code()` → Updates all zones
- This is necessary because patterns can affect multiple zones simultaneously
- Auto-completion and merging effects can ripple across multiple adjacent zones

## Files Modified

- `plugins/TilemapEditor/engine/src/core/code_gen.c` - Replaced zone-specific updates with complete update

## Result

Pattern application now maintains perfect level code synchronization:

- ✅ **Current block**: Shows the applied pattern
- ✅ **Adjacent blocks**: Show updated patterns reflecting auto-completion tiles
- ✅ **All blocks**: Level codes always match actual tilemap content

When you apply pattern `1` and it auto-completes into the next block, both blocks immediately show the correct level code patterns!
