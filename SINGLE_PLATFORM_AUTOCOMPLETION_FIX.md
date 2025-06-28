# Single Platform Auto-Completion Fix

## Problem Identified

When patterns contained single isolated platform tiles (especially at position 4, the rightmost position in a segment), they were being removed by the `rebuild_platform_row()` function instead of being auto-completed into 2-tile platforms as they should be.

### Affected Patterns

Several patterns end with single platforms that should auto-complete:

- **Pattern 1** (`1`): `0b0000000001` - Single platform at position 4
- **Pattern 9** (`9`): `0b0000001101` - Platforms at positions 1-2,4 (isolated at position 4)
- **Pattern 14** (`e`): `0b0000010001` - Two isolated platforms at positions 0,4
- **Pattern 18** (`i`): `0b0000011101` - Platforms at positions 0-2,4 (isolated at position 4)

## Root Cause

The `rebuild_platform_row()` function in `paint.c` automatically removes single platform tiles:

```c
if (current_len == 1)
{
    remove_enemies_above_platform(seq_start, y);
    replace_meta_tile(seq_start, y, TILE_EMPTY, 1);
}
```

However, manual painting is "smart" because it doesn't create single platforms in the first place - it automatically creates 2-tile platforms when you paint in an empty area.

## Solution Implemented

Modified `apply_pattern_with_brush_logic()` to detect isolated single platforms at the rightmost position (position 4) and auto-complete them before calling `rebuild_platform_row()`.

### Key Changes

1. **Isolated Platform Detection**: Check if a platform tile has no adjacent neighbors within the pattern
2. **Auto-Completion Logic**: For isolated platforms at position 4 (rightmost), automatically place a second tile to the right
3. **Boundary Checking**: Ensure auto-completion doesn't exceed `PLATFORM_X_MAX`

### Code Logic

```c
// Check if this is a single isolated platform that needs auto-completion
UBYTE is_isolated = 1;

// Check if there are adjacent platforms in the pattern
if (i > 0 && ((row_pattern >> (4 - (i - 1))) & 1)) // left neighbor
{
    is_isolated = 0;
}
if (i < SEGMENT_WIDTH - 1 && ((row_pattern >> (4 - (i + 1))) & 1)) // right neighbor
{
    is_isolated = 0;
}

// If this is an isolated platform at the rightmost position, auto-complete it
if (is_isolated && i == SEGMENT_WIDTH - 1 && tile_x + 1 <= PLATFORM_X_MAX)
{
    // Place a 2-tile platform (auto-completion)
    replace_meta_tile(tile_x, current_y, TILE_PLATFORM_LEFT, 1);
    replace_meta_tile(tile_x + 1, current_y, TILE_PLATFORM_RIGHT, 1);
}
else
{
    // Place a basic platform tile - rebuild_platform_row will fix the styling
    replace_meta_tile(tile_x, current_y, TILE_PLATFORM_MIDDLE, 1);
}
```

## Expected Behavior

### Before Fix

- Pattern 1 (`1`): Single platform at position 4 → Gets removed by `rebuild_platform_row()` → Results in empty segment

### After Fix

- Pattern 1 (`1`): Single platform at position 4 → Auto-completed to 2-tile platform → Platform extends into next block → Matches manual painting behavior

## Edge Cases Handled

1. **Boundary Checking**: Auto-completion only occurs if `tile_x + 1 <= PLATFORM_X_MAX`
2. **Non-Isolated Platforms**: Only isolated single platforms are auto-completed, not platforms that are part of larger structures
3. **Position-Specific**: Only applies to position 4 (rightmost in segment) where extension to the right makes sense
4. **Pattern Integrity**: Other platform positions in the pattern remain unchanged

## Testing Recommendations

1. **Pattern 1 Test**: Enter code `1` and verify it creates a 2-tile platform extending into the next block
2. **Pattern 9 Test**: Enter code `9` and verify the isolated platform at position 4 gets auto-completed
3. **Pattern 14 Test**: Enter code `e` and verify both isolated platforms (positions 0 and 4) behave correctly
4. **Pattern 18 Test**: Enter code `i` and verify the isolated platform at position 4 gets auto-completed

## Result

Single platforms at the rightmost position of patterns now behave consistently with manual painting - they automatically become 2-tile platforms that extend into the adjacent block, just as the manual paint tool would create them.

This ensures that patterns containing single platforms work as expected and match the user's intuitive expectation that painting a single platform should result in a visible, functional 2-tile platform.
