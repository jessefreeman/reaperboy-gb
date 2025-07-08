# Enemy Code Input Brush-Based Validation Refactor

## Problem
The enemy level code system was allowing users to enter invalid enemy codes that didn't correspond to positions where the paint brush would actually allow placing enemies. This led to inconsistencies where:

- Users could enter codes that resulted in enemy positions with no platform support
- Enemies could be placed too close to each other, violating spacing rules
- Enemies could be placed in invalid rows or positions that the brush validation would reject
- The level code system and brush system had different validation logic

## Solution
Completely refactored the enemy code input system to route all validation through the paint brush system, ensuring that only codes corresponding to brush-valid positions can be entered.

## Key Changes

### 1. New Brush-Based Validation Functions (`code_enemy_system.c`)

```c
// Test if the brush would allow painting an enemy at coordinates corresponding to a POS41 value
UBYTE is_pos41_value_brush_valid(UBYTE enemy_index, UBYTE pos_value, UBYTE odd_bit) BANKED;

// Find next/previous valid positions using brush validation
UBYTE find_next_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED;
UBYTE find_prev_brush_valid_pos41(UBYTE enemy_index, UBYTE current_pos, UBYTE odd_bit) BANKED;

// Cycle through valid enemy codes
UBYTE cycle_to_next_valid_enemy_code(UBYTE char_index) BANKED;
UBYTE cycle_to_prev_valid_enemy_code(UBYTE char_index) BANKED;

// Check if a specific code value would be brush-valid
UBYTE is_enemy_code_brush_valid(UBYTE char_index, UBYTE test_value) BANKED;
```

### 2. Integration with Paint System
- Uses `can_paint_enemy_right()` as the authoritative validation function
- Temporarily clears enemy positions during validation to avoid self-conflicts
- Converts POS41 codes to actual tilemap coordinates and tests with brush system
- Ensures the same validation logic used by the paint tools

### 3. Enhanced Level Code Editing (`handle_enemy_data_edit()`)
- **Position Characters (17-21)**: Routes validation through `is_pos41_value_brush_valid()`
- **Odd Mask Character (22)**: Validates that all current enemies remain valid with new odd bits
- **Direction Mask Character (23)**: Accepts any valid BASE32 value (directions don't affect brush validation)
- **Fallback Logic**: If invalid code entered, automatically finds next valid position

### 4. Updated Character Cycling (`code_persistence.c`)
- Replaced old validation system with new brush-based functions
- **Forward/Reverse Cycling**: Uses `cycle_to_next_valid_enemy_code()` and `cycle_to_prev_valid_enemy_code()`
- **All Character Types**: Position codes, odd mask, and direction mask all use unified brush validation

## Behavior Changes

### For Main Enemy Position Codes (Characters 17-21)
- **Before**: Could enter codes that placed enemies without platform support or too close together
- **After**: Only allows codes that correspond to positions where `can_paint_enemy_right()` returns true

### For Odd Mask Code (Character 22)
- **Before**: Could change odd bits resulting in invalid final positions
- **After**: Only allows odd mask values where all current enemies remain in brush-valid positions

### For Direction Mask Code (Character 23)
- **Before**: Simple BASE32 cycling (0-31)
- **After**: Same BASE32 cycling (directions don't affect brush validation)

### Code Cycling
- **Before**: Could cycle to invalid positions that the brush wouldn't allow
- **After**: Only cycles through positions that the brush validation system accepts

## Benefits

1. **Consistency**: Level code system and brush system now use identical validation
2. **Validation**: Impossible to enter codes that result in invalid enemy positions
3. **User Experience**: Users only see codes that actually work in the game
4. **Reliability**: Eliminates discrepancies between code input and paint tools
5. **Maintainability**: Single source of truth for enemy placement validation

## Implementation Details

### Validation Flow
1. User enters/cycles enemy code
2. Code converted to tilemap coordinates (x, y)
3. Enemy temporarily removed from level code structure
4. `can_paint_enemy_right(x, y)` called to test validity
5. Enemy position restored
6. Code accepted/rejected based on brush validation result

### Performance Considerations
- Validation functions are called only during user input (not real-time)
- Temporary enemy removal/restoration is fast (just array assignments)
- Cycling functions search sequentially but will be optimized with caching in future

### Future Enhancements
- **Caching**: Pre-compute valid code ranges for faster cycling
- **Smart Search**: Use spatial indexing to find valid positions more efficiently
- **Batch Validation**: Optimize when multiple enemy codes change simultaneously

## Files Modified
- `plugins/TilemapEncoder/engine/src/core/code_enemy_system.c`
- `plugins/TilemapEncoder/engine/include/code_enemy_system.h`
- `plugins/TilemapEncoder/engine/src/core/code_persistence.c`

## Testing
- Test all enemy code input scenarios
- Verify cycling only shows valid codes
- Confirm consistency between brush painting and code entry
- Test edge cases (no valid positions, full level, etc.)
