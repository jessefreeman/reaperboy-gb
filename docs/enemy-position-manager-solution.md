# Enemy Position Manager - Unified Solution

## Problem Summary

The enemy positioning logic had several issues:
1. **Inconsistent validation** between TilemapEncoder and TilemapPainter plugins
2. **Real-time platform detection** instead of cached valid positions
3. **Enemy spacing conflicts** due to different validation rules
4. **Inefficient position calculations** happening repeatedly

## Solution Architecture

### 1. Unified Enemy Position Manager (`enemy_position_manager.c/h`)

A new centralized system that provides:

- **Platform Tracking**: Cached platform positions updated when platforms change
- **Unified Validation**: Single source of truth for enemy placement rules
- **Position Cycling**: Efficient next/previous position calculation for level code editor
- **Coordinate Conversion**: Utilities for converting between different coordinate systems

### 2. Key Features

#### Platform Caching
```c
UBYTE platform_positions[4][20]; // [platform_row][column]
```
- Pre-calculates which positions have platforms
- Updates only when platforms actually change
- Eliminates real-time tile scanning

#### Unified Validation Logic
```c
UBYTE is_valid_enemy_position_unified(UBYTE x, UBYTE y)
```
- Single validation function used by both plugins
- Ensures consistent enemy placement rules
- Checks: platform below, position empty, not below player, no adjacent enemies

#### Enemy Spacing Management
```c
UBYTE has_enemy_at_adjacent_positions(UBYTE x, UBYTE y)
```
- Prevents enemies from being placed adjacent to each other
- Allows exact position replacement for direction flipping
- Maintains proper enemy spacing

### 3. Integration Points

#### With Platform System
- `on_platform_changed(x, y)` - Called when platforms are added/removed
- Automatically updates valid enemy positions
- Notifies dependent systems of changes

#### With Paint Tool
- `validate_enemy_placement(x, y)` - Validates paint operations
- Consistent with level code editor rules
- Supports both new placement and existing enemy modification

#### With Level Code Editor
- `get_next_valid_enemy_position()` / `get_prev_valid_enemy_position()`
- Efficient cycling through valid positions
- Maintains position validity during editing

### 4. Implementation Details

#### Position Coordinates
- **Enemy Rows**: 12, 14, 16, 18 (4 rows)
- **Platform Rows**: 13, 15, 17, 19 (directly below enemies)
- **Columns**: 0-19 (20 positions per row)
- **Tilemap Range**: X: 2-21, Y: 12-18

#### Position Encoding
- **POS41 System**: 1-40 values for level code
- **Row/Column Arrays**: Internal 0-based indexing
- **Odd/Even Bits**: For sub-tile positioning

#### Validation Rules
1. Must be on valid enemy row (12, 14, 16, 18)
2. Must have platform directly below
3. Position must be empty
4. Cannot be directly below player
5. Cannot be adjacent to other enemies
6. Within valid tilemap bounds

### 5. Benefits

#### Performance
- **Cached platforms** eliminate repeated tile scanning
- **Pre-calculated valid positions** speed up position cycling
- **Reduced redundant validation** calls

#### Consistency
- **Single validation logic** shared across all systems
- **Unified coordinate handling** prevents conversion errors
- **Consistent spacing rules** for all enemy operations

#### Maintainability
- **Centralized enemy logic** in one location
- **Clear separation** between platform and enemy concerns
- **Extensible design** for future enemy features

### 6. Usage Examples

#### Paint Tool Integration
```c
// In paint_entity.c
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y) BANKED
{
    return validate_enemy_placement(x, y);
}
```

#### Platform Change Notification
```c
// In paint_platform.c - after rebuilding platforms
on_platform_changed(0, y); // Recalculate valid positions
```

#### Level Code Position Cycling
```c
// In code_enemy_system_validation.c
UBYTE next_row, next_col;
if (get_next_valid_enemy_position(current_row, current_col, &next_row, &next_col))
{
    // Convert to POS41 and update
}
```

### 7. Migration Notes

#### Files Updated
- `code_enemy_system_validation.c` - Uses unified functions
- `paint_entity.c` (TilemapPainter) - Uses unified validation
- `paint_platform.c` - Notifies manager of platform changes
- `code_enemy_system.c` - Includes unified manager

#### Backward Compatibility
- Existing function signatures preserved
- Internal logic redirects to unified system
- No changes required in calling code

### 8. Testing Considerations

#### Platform Integration
- Verify enemy positions update when platforms added/removed
- Test platform-enemy relationship consistency
- Validate caching accuracy

#### Position Validation
- Test enemy spacing rules
- Verify player collision detection
- Check boundary validation

#### Performance
- Measure platform cache hit rates
- Validate position cycling efficiency
- Check memory usage of caching system

This unified solution provides a robust, efficient, and maintainable foundation for enemy positioning that works consistently across both the paint tool and level code editor systems.
