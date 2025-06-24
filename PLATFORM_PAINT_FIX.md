# Platform Paint Logic Fix

## Problem Description

The original platform painting logic had a critical bug where users could create visually connected platforms that exceeded the 8-tile limit. This created an inconsistency between what the user saw and how the system treated the platforms:

1. **Visual Appearance**: Two 8-tile platforms could appear to be one continuous 16-tile platform
2. **Logical Treatment**: The `rebuild_platform_row()` function would split them into two separate 8-tile platforms
3. **Code Generation**: This led to inconsistent level code generation and potential gameplay issues

## Root Cause

The issue was in the platform placement logic in `paint.c` where two distinct problems existed:

### Problem 1: Direct Platform Extension

```c
// OLD CODE - No length checking for direct connections
if (left == BRUSH_TILE_PLATFORM || right == BRUSH_TILE_PLATFORM)
{
    // Connect to existing platform - NO LIMIT CHECK!
    replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
}
```

This allowed users to extend existing 8-tile platforms indefinitely by clicking adjacent to them.

### Problem 2: Indirect Auto-Merging

```c
// OLD CODE - No merge checking for new 2-tile platforms
else if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
         !check_platform_vertical_conflict(x + 1, y))
{
    // Create new 2-tile platform - NO MERGE LIMIT CHECK!
    replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
    replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
}
```

This allowed users to create 2-tile platforms that would auto-merge with nearby platforms during `rebuild_platform_row()`, potentially creating platforms longer than 8 tiles.

## Solution Implementation

### 1. Added Platform Length Validation

Added a new function `count_connected_platform_length()` that calculates the total length a platform would have if a tile is placed at position (x, y):

```c
UBYTE count_connected_platform_length(UBYTE x, UBYTE y) BANKED
{
    UBYTE length = 1; // Count the tile we would place

    // Count platforms to the left
    UBYTE left_x = x;
    while (left_x > PLATFORM_X_MIN)
    {
        left_x--;
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(left_x, y)]) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    // Count platforms to the right
    for (UBYTE right_x = x + 1; right_x <= PLATFORM_X_MAX; right_x++)
    {
        if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(right_x, y)]) == BRUSH_TILE_PLATFORM)
        {
            length++;
        }
        else
        {
            break;
        }
    }

    return length;
}
```

### 2. Added Auto-Merge Validation

Added a new function `would_2tile_platform_exceed_limit()` that checks if creating a 2-tile platform would result in auto-merging that exceeds the 8-tile limit:

```c
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y) BANKED
{
    // Check if there's a platform to the left that would merge
    UBYTE left_length = 0;
    if (x > PLATFORM_X_MIN && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x - 1, y)]) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the left
        UBYTE left_x = x - 1;
        while (left_x >= PLATFORM_X_MIN && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(left_x, y)]) == BRUSH_TILE_PLATFORM)
        {
            left_length++;
            if (left_x == PLATFORM_X_MIN) break;
            left_x--;
        }
    }

    // Check if there's a platform to the right that would merge
    UBYTE right_length = 0;
    if (x + 2 <= PLATFORM_X_MAX && get_tile_type(sram_map_data[METATILE_MAP_OFFSET(x + 2, y)]) == BRUSH_TILE_PLATFORM)
    {
        // Count platforms to the right
        for (UBYTE right_x = x + 2; right_x <= PLATFORM_X_MAX; right_x++)
        {
            if (get_tile_type(sram_map_data[METATILE_MAP_OFFSET(right_x, y)]) == BRUSH_TILE_PLATFORM)
            {
                right_length++;
            }
            else
            {
                break;
            }
        }
    }

    // Total length would be: left platforms + our 2 tiles + right platforms
    UBYTE total_length = left_length + 2 + right_length;
    return total_length > PLATFORM_MAX_LENGTH;
}
```

### 3. Updated Platform Placement Logic

Modified the `paint()` function to check platform length before allowing both direct connections and new platform creation:

```c
if (left == BRUSH_TILE_PLATFORM || right == BRUSH_TILE_PLATFORM)
{
    // Check if connecting would exceed 8-tile limit
    UBYTE platform_length = count_connected_platform_length(x, y);
    if (platform_length > PLATFORM_MAX_LENGTH)
    {
        return; // Cannot exceed 8-tile platform limit
    }

    // Connect to existing platform
    replace_meta_tile(x, y, TILE_PLATFORM_MIDDLE, 1);
}
else if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY &&
         !check_platform_vertical_conflict(x + 1, y))
{
    // Check if creating a 2-tile platform would exceed limits after auto-merge
    if (would_2tile_platform_exceed_limit(x, y))
    {
        return; // Cannot create 2-tile platform - would exceed limit after merge
    }

    // Create new 2-tile platform
    replace_meta_tile(x, y, TILE_PLATFORM_LEFT, 1);
    replace_meta_tile(x + 1, y, TILE_PLATFORM_RIGHT, 1);
}
```

### 4. Updated Selector State Logic

Modified `get_platform_placement_type()` to show the correct selector state based on both direct connection and auto-merge constraints:

```c
if (left == BRUSH_TILE_PLATFORM)
{
    // Check if connecting would exceed 8-tile limit
    UBYTE platform_length = count_connected_platform_length(x, y);
    if (platform_length > PLATFORM_MAX_LENGTH)
    {
        return 0; // Cannot place - would exceed platform limit
    }
    return SELECTOR_STATE_PLATFORM_RIGHT;
}
if (right == BRUSH_TILE_PLATFORM)
{
    // Check if connecting would exceed 8-tile limit
    UBYTE platform_length = count_connected_platform_length(x, y);
    if (platform_length > PLATFORM_MAX_LENGTH)
    {
        return 0; // Cannot place - would exceed platform limit
    }
    return SELECTOR_STATE_PLATFORM_LEFT;
}

if (x < PLATFORM_X_MAX && right == BRUSH_TILE_EMPTY && can_place_platform(x + 1, y))
{
    // Check if creating a 2-tile platform would exceed limits after auto-merge
    if (would_2tile_platform_exceed_limit(x, y))
    {
        return 0; // Cannot create - would exceed limit after merge
    }
    return SELECTOR_STATE_NEW_PLATFORM;
}
```

## Benefits of This Fix

### 1. **Visual-Logical Consistency**

- What the user sees matches exactly how the system treats platforms
- No more "phantom connections" between separate platforms

### 2. **Predictable Behavior**

- Users can't accidentally create invalid platform configurations
- Clear feedback when platform limits are reached

### 3. **Accurate Level Code Generation**

- Platform patterns are correctly identified and encoded
- No loss of data due to visual/logical mismatches

### 4. **Better User Experience**

- Clear constraints prevent user confusion
- Immediate feedback when limits are reached through selector states

## Technical Details

### Function Declarations Added

- Added `count_connected_platform_length()` declaration to `paint.h`
- Function is marked as `BANKED` for proper memory management

### Edge Cases Handled

- **Boundary Conditions**: Proper handling of `PLATFORM_X_MIN` and `PLATFORM_X_MAX`
- **UBYTE Underflow**: Used `while` loop instead of `for` loop to prevent unsigned underflow
- **Empty Tiles**: Only counts actual platform tiles, ignores gaps

### Performance Considerations

- **Linear Scan**: Function performs O(n) scan where n ≤ 8, so very fast
- **Called Only On Paint**: Only executed when user attempts to place a platform
- **Early Termination**: Stops scanning as soon as non-platform tile is found

## Testing Scenarios

### Scenario 1: Valid Platform Extension

- User has a 6-tile platform
- User tries to add adjacent tile → **ALLOWED** (total would be 7)
- Visual and logical platform match perfectly

### Scenario 2: Direct Extension Blocked - 8-tile Platform

- User has an 8-tile platform at positions 5-12
- User tries to click at position 4 or 13 → **BLOCKED** (total would be 9)
- Selector shows default state (no platform connection icon)

### Scenario 3: Auto-Merge Blocked - 7-tile + 2-tile = 9-tile

- User has a 7-tile platform at positions 5-11
- User tries to create 2-tile platform at positions 3-4 → **BLOCKED** (would auto-merge to 9 tiles)
- Selector shows default state (no new platform icon)

### Scenario 4: Auto-Merge Allowed - 6-tile + 2-tile = 8-tile

- User has a 6-tile platform at positions 5-10
- User tries to create 2-tile platform at positions 3-4 → **ALLOWED** (would auto-merge to exactly 8 tiles)
- Selector shows new platform icon, placement succeeds

### Scenario 5: Gap Between Platforms

- User has 8-tile platform, empty space, then another platform
- User can fill the gap → **ALLOWED** if resulting platform ≤ 8 tiles
- User cannot fill if it would create >8 tile platform

### Scenario 6: Complex Auto-Merge Prevention

- User has 3-tile platform at positions 2-4, gap, then 4-tile platform at positions 7-10
- User tries to create 2-tile platform at positions 5-6 → **BLOCKED** (would create 3+2+4=9 tile platform)
- System correctly prevents the visual connection that would break the 8-tile rule

## Integration with Level Code System

This fix ensures that the level code generation system receives consistent platform data:

- **Pattern Recognition**: All platforms are guaranteed to be ≤8 tiles
- **Encoding Accuracy**: No ambiguity about platform boundaries
- **Data Integrity**: Platform patterns match visual layout exactly

## Future Considerations

### Alternative Approaches Considered

1. **Allow >8 tile platforms**: Would require major changes to level code system
2. **Visual gaps at 8-tile boundaries**: Would be confusing to users
3. **Dynamic splitting**: Complex and could lead to unexpected behavior

The chosen approach (prevention) is the cleanest and most user-friendly solution.

### Potential Enhancements

1. **Visual Feedback**: Could add different selector states to show when platform limit is near
2. **Platform Counter**: Could display current platform length to user
3. **Smart Breaking**: Could suggest where to break long platforms into segments

## Files Modified

1. **paint.c**: Added comprehensive length validation logic with two new functions
   - `count_connected_platform_length()` - Calculates platform length for direct connections
   - `would_2tile_platform_exceed_limit()` - Prevents auto-merge violations
2. **paint.h**: Added function declarations for both new functions
3. **Documentation**: This file and updated project docs

## Verification

To verify this fix works correctly:

1. Build the project with updated `paint.c`
2. Test platform placement scenarios described above
3. Verify level code generation produces expected patterns
4. Confirm no visual-logical disconnects occur

This fix resolves the last major bug in the platform/enemy encoding system and ensures robust, lossless level code generation.
