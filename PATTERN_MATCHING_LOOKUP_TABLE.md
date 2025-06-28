# Platform Pattern Matching Lookup Table Design

## Overview

This document explores the design of a lookup table for pattern matching in the reaperboy-metatiles system. The goal is to determine which patterns can be validly placed adjacent to each other without creating invalid platform configurations.

## Current System Analysis

### Pattern Structure

- **Format**: 10-bit patterns where top row = bits 9-5, bottom row = bits 4-0
- **Segment Size**: 5 tiles wide × 2 tiles high
- **Position Mapping**: Position 0 = leftmost (bit 4), Position 4 = rightmost (bit 0)
- **Pattern Count**: 21 patterns (UIDs 0-20)

### Platform Rules

1. **Minimum Length**: 2 tiles (no single-tile platforms allowed)
2. **Maximum Length**: 8 tiles (platform cannot exceed 8 connected tiles)
3. **Connectivity**: Platforms connect horizontally across segment boundaries

### Edge Connectivity System

The system already has edge connectivity data in `PATTERN_EDGE_CONNECTIVITY[]`:

- **TL**: Top Left (bit 9 - position 0, top row)
- **TR**: Top Right (bit 5 - position 4, top row)
- **BL**: Bottom Left (bit 4 - position 0, bottom row)
- **BR**: Bottom Right (bit 0 - position 4, bottom row)

## Edge Case Analysis

### Case 1: Empty Tile at Pattern End

**Scenario**: Current pattern has empty tile at the rightmost position (position 4)
**Rule**: Can only match with patterns that have:

- Empty tile at leftmost position (position 0), OR
- At least 2 connected tiles starting from position 0

**Reasoning**: If the current pattern ends with empty space, the next pattern cannot start with a single isolated tile, as this would create an invalid 1-tile platform.

### Case 2: Single Tile at Pattern End

**Scenario**: Current pattern ends with exactly 1 tile at position 4 (pattern ending with "01")
**Rule**: Can only match with patterns that have at least 1 tile at position 0
**Reasoning**: The single tile at the end must connect to at least one tile at the beginning of the next pattern to form a valid 2+ tile platform.

### Case 3: Platform Length Validation

**Scenario**: Two patterns when combined create a platform longer than 8 tiles
**Rule**: Such combinations are invalid and must be rejected
**Reasoning**: Platform maximum length constraint must be enforced.

## Pattern Analysis Examples

Let's analyze some specific pattern combinations:

### Valid Combination Example

- **Pattern A**: `0b0000000011` (two tiles at positions 3-4)
- **Pattern B**: `0b0000011000` (two tiles at positions 2-3)
- **Connection**: Pattern A ends with platform, Pattern B starts with empty space
- **Result**: Valid - no invalid single tiles created

### Invalid Combination Example

- **Pattern A**: `0b0000000001` (single tile at position 4)
- **Pattern B**: `0b0000010000` (single tile at position 3)
- **Connection**: Pattern A ends with single tile, Pattern B has no tile at position 0
- **Result**: Invalid - would create isolated single tile

## Pattern Compatibility Matrix

This matrix shows which patterns can be validly placed to the RIGHT of each pattern. ✓ = Compatible, ✗ = Invalid

### Pattern Data Reference

```
UID  Pattern      Description                    Edges  Top Row  Bottom Row
 0   0b0000000000 Empty                         0b0000  .....    .....
 1   0b0000000001 Single at pos 4 bottom       0b0001  .....    ....X
 2   0b0000010000 Single at pos 3 top          0b0000  ..X..    .....
 3   0b0000000011 Two at pos 3-4 bottom        0b0001  .....    ...XX
 4   0b0000011000 Two at pos 2-3 top           0b0000  ..XX.    .....
 5   0b0000000110 Two at pos 2-3 bottom        0b0000  .....    ..XX.
 6   0b0000001100 Two at pos 1-2 bottom        0b0000  .....    .XX..
 7   0b0000000111 Three at pos 2-4 bottom      0b0001  .....    ..XXX
 8   0b0000011100 Three at pos 1-3 top         0b0000  .XXX.    .....
 9   0b0000001101 Gapped at pos 1-2,4 bottom   0b0001  .....    .XX.X
10   0b0000010110 Gapped at pos 1,3-4 bottom   0b0001  .....    .X.XX
11   0b0000001110 Three at pos 1-3 bottom      0b0000  .....    .XXX.
12   0b0000001111 Four at pos 1-4 bottom       0b0001  .....    .XXXX
13   0b0000011110 Four at pos 1-4 top          0b0010  .XXXX    .....
14   0b0000010001 Two isolated at pos 0,4      0b0011  .....    X...X
15   0b0000010011 Three at pos 0,3-4 bottom    0b0011  .....    X..XX
16   0b0000011001 Three at pos 0,2-3 bottom    0b0010  .....    X.XX.
17   0b0000010111 Four at pos 0,2-4 bottom     0b0011  .....    X.XXX
18   0b0000011101 Four at pos 0-2,4 bottom     0b0011  .....    XXX.X
19   0b0000011011 Four at pos 0-1,3-4 bottom   0b0011  .....    XX.XX
20   0b0000011111 Full coverage bottom         0b0011  .....    XXXXX
```

### Compatibility Matrix

**Usage**: Find the intersection of Left Pattern (Y-axis) and Right Pattern (X-axis). ✓ = Compatible, ✗ = Invalid

```
      RIGHT PATTERN →
LEFT   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F  G  H  I  J  K
 ↓
 0    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 1    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 2    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 3    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 4    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 5    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 6    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 7    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 8    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 9    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 A    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 B    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 C    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 D    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 E    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 F    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 G    ✓  ✗  ✓  ✗  ✓  ✓  ✓  ✗  ✓  ✗  ✗  ✓  ✗  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓
 H    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 I    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 J    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
 K    ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✓  ✗  ✓  ✓  ✓  ✓  ✗  ✗  ✗  ✗
```

**Character Mapping**: 0-9 (patterns 0-9), A-K (patterns 10-20)

**How to Read**:

- **Left Pattern 0 + Right Pattern 1**: Find row 0, column 1 = ✗ (Invalid - single tile isolation)
- **Left Pattern C + Right Pattern K**: Find row C, column K = ✗ (Invalid - platform too long)
- **Left Pattern 7 + Right Pattern C**: Find row 7, column C = ✗ (Invalid - would create 9-tile platform)

### Compatibility Rules Applied

1. **Empty Ending + Single Start = Invalid**: Patterns ending with empty space (pos 4) cannot connect to patterns starting with single tiles (pos 0), as this creates invalid isolated platforms.

2. **Single Ending + Any Start = Valid**: Patterns ending with single tiles (pos 4) can connect to any pattern starting with platforms (pos 0), forming valid 2+ tile platforms.

3. **Platform Length Validation**: Patterns that would create platforms longer than 8 tiles when combined are invalid:

   - Pattern 7 (..XXX) + Patterns C,H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern 9 (.XX.X) + Patterns C,H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern A (.X.XX) + Patterns C,H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern C (.XXXX) + Patterns C,H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern E (X...X) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern F (X..XX) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern H (X.XXX) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern I (XXX.X) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern J (XX.XX) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)
   - Pattern K (XXXXX) + Patterns H,I,J,K = Invalid (would create 9+ tile platforms)

4. **Platform Ending + Platform Start = Valid**: Any platform ending can connect to any platform start (unless it violates length constraint).

5. **Platform Ending + Empty Start = Valid**: Platform endings can connect to empty starts without creating invalid single tiles.

### Pattern Legend

- **Sngl R**: Single tile, right position (bottom)
- **Sngl T**: Single tile, top position
- **Two R/M/L**: Two tiles, right/middle/left positions
- **Thr R/M/T**: Three tiles, right/middle/top positions
- **Gap R**: Gapped patterns with right edge
- **Four R/T**: Four tiles, right/top positions
- **Iso LR**: Isolated tiles at left and right
- **L+2R**: Left tile + 2 right tiles
- **Full**: Complete 5-tile coverage

## Lookup Table Design

### Data Structure Options

#### Option 1: 2D Boolean Array

```c
// 21x21 compatibility matrix
UBYTE pattern_compatibility[21][21];
// pattern_compatibility[left_pattern][right_pattern] = 1 if valid, 0 if invalid
```

#### Option 2: Bitfield Array

```c
// Each pattern has a 32-bit mask of compatible right-side patterns
ULONG pattern_right_compatibility[21];
// Bit N set means pattern N can be placed to the right
```

#### Option 3: Edge-Based Lookup

```c
// Lookup based on edge connectivity combinations
UBYTE edge_compatibility[16][16]; // 4-bit edge combinations
// More compact, works with any future pattern additions
```

### Recommended Approach: Edge-Based Lookup

The edge-based approach is most efficient because:

1. **Compact**: Only 16×16 = 256 bytes vs 21×21 = 441 bytes
2. **Extensible**: Works with any future pattern additions
3. **Clear Logic**: Easy to understand and debug

## Implementation Strategy

### Phase 1: Connectivity Rules Engine

Create a function that evaluates pattern compatibility based on edge analysis:

```c
UBYTE patterns_are_compatible(UBYTE left_pattern, UBYTE right_pattern) {
    UBYTE left_edges = PATTERN_EDGE_CONNECTIVITY[left_pattern];
    UBYTE right_edges = PATTERN_EDGE_CONNECTIVITY[right_pattern];

    // Extract relevant edges
    UBYTE left_top_right = (left_edges >> 2) & 1;
    UBYTE left_bottom_right = left_edges & 1;
    UBYTE right_top_left = (right_edges >> 3) & 1;
    UBYTE right_bottom_left = (right_edges >> 1) & 1;

    // Apply compatibility rules
    return check_edge_compatibility(
        left_top_right, left_bottom_right,
        right_top_left, right_bottom_left
    );
}
```

### Phase 2: Pre-computed Lookup Table

Once the rules are validated, generate a pre-computed lookup table:

```c
// Generated at compile time or initialization
const UBYTE PATTERN_COMPATIBILITY_MATRIX[21][21] = {
    // Pre-computed using the rules engine
};
```

## Validation Test Cases

### Test Set 1: Single Tile Edge Cases

1. Pattern ending with single tile + Pattern starting with empty → Invalid
2. Pattern ending with single tile + Pattern starting with single tile → Valid (forms 2-tile platform)
3. Pattern ending with single tile + Pattern starting with 2+ tiles → Valid

### Test Set 2: Platform Length Validation

1. 6-tile platform + 2-tile platform → Valid (8 tiles total)
2. 6-tile platform + 3-tile platform → Invalid (9 tiles total)
3. 4-tile platform + 4-tile platform → Valid (8 tiles total)

### Test Set 3: Empty Space Handling

1. Pattern ending with empty + Pattern starting with empty → Valid
2. Pattern ending with empty + Pattern starting with single tile → Invalid
3. Pattern ending with empty + Pattern starting with 2+ tiles → Valid

## Integration Points

### Code Generation System

- Update `apply_pattern_to_tilemap()` to validate compatibility
- Modify pattern cycling to only show valid next patterns
- Add validation to code entry mode

### User Interface

- Highlight compatible patterns when selecting
- Show incompatible patterns as disabled/grayed out
- Provide visual feedback for invalid combinations

### Pattern Selection Logic

- Update `find_next_available_pattern()` to respect compatibility
- Add compatibility checking to pattern editing functions

## Performance Considerations

### Memory Usage

- Edge-based lookup: 256 bytes
- Pattern-based lookup: 441 bytes
- Runtime rules engine: ~0 bytes (code only)

### Speed

- Lookup table: O(1) - fastest
- Rules engine: O(1) - slightly slower but more flexible
- Hybrid approach: Pre-compute common cases, fall back to rules for edge cases

## Next Steps

1. **Implement Rules Engine**: Start with a function-based approach for flexibility
2. **Generate Test Data**: Create comprehensive test cases for all edge scenarios
3. **Validate Logic**: Test against known good/bad pattern combinations
4. **Optimize Storage**: Convert to lookup table once rules are proven
5. **Integrate UI**: Add visual feedback for pattern compatibility

## Questions for Further Investigation

1. **Diagonal Connections**: Do patterns need to consider diagonal platform connections?
2. **Multi-Segment Validation**: Should we validate 3+ segment combinations?
3. **Performance Requirements**: Is lookup table speed critical or is rules engine sufficient?
4. **Dynamic Patterns**: Will new patterns be added at runtime or only at compile time?

## File References

- Pattern definitions: `plugins/TilemapEditor/engine/src/core/code_gen.c` (lines 40-60)
- Edge connectivity: `plugins/TilemapEditor/engine/src/core/code_gen.c` (lines 70-90)
- Pattern matching: `plugins/TilemapEditor/engine/src/core/code_gen.c` (lines 458-487)
- Platform rules: `plugins/TilemapEditor/engine/src/core/paint_clean_fixed.c` (lines 330-400)
