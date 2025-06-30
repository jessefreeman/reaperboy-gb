# Platform Pattern System Optimization

## Overview

The platform pattern matching system has been optimized to work with single-row platform configurations instead of the previous 5x2 block system. Since platforms are only rendered on odd rows (the second row of each 2-row segment), the system now uses simplified 5-bit patterns instead of 10-bit patterns.

## Key Changes

### 1. Pattern Data Structure Simplified

**Before:**

- Used `UWORD` (16-bit) patterns with 10 bits of data
- Top 5 bits (9-5) for first row, bottom 5 bits (4-0) for second row
- 21 predefined patterns

**After:**

- Uses `UBYTE` (8-bit) patterns with 5 bits of data
- Only bottom 5 bits (4-0) for the platform row
- 32 comprehensive patterns covering all possible 5-bit combinations

### 2. Function Signatures Updated

**Before:**

```c
UWORD extract_chunk_pattern(UBYTE x, UBYTE y, UBYTE *row0, UBYTE *row1) BANKED;
UWORD match_platform_pattern(UWORD pattern) BANKED;
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UWORD pattern, UBYTE block_index) BANKED;
```

**After:**

```c
UBYTE extract_chunk_pattern(UBYTE x, UBYTE y) BANKED;
UBYTE match_platform_pattern(UBYTE pattern) BANKED;
void apply_pattern_with_endcaps(UBYTE segment_x, UBYTE segment_y, UBYTE pattern, UBYTE block_index) BANKED;
```

### 3. Pattern Extraction Logic

**Before:**

- Checked both rows of a 2-row segment
- Returned complex 10-bit pattern

**After:**

- Only checks the platform row (y+1) of the segment
- Returns simple 5-bit pattern representing platform positions

### 4. Pattern Array Expansion

The new pattern array includes all 32 possible 5-bit combinations (0b00000 to 0b11111), providing comprehensive coverage of platform configurations:

```c
const UBYTE PLATFORM_PATTERNS[] = {
    0b00000, // UID 0: Empty
    0b00001, // UID 1: Single platform at position 4
    // ... (32 total patterns)
    0b11111  // UID 31: Full platform coverage
};
```

### 5. Display Character Mapping

Updated to support 32 patterns instead of 21:

- Characters 0-9 for patterns 0-9
- Characters A-V for patterns 10-31

## Performance Benefits

1. **Reduced Memory Usage:**

   - Pattern data reduced from 16-bit to 8-bit
   - Simpler pattern matching with fewer bit operations

2. **Faster Pattern Extraction:**

   - Only reads 5 tiles instead of 10 tiles per segment
   - Eliminates unnecessary row checking

3. **Improved Cache Efficiency:**

   - Smaller data structures fit better in memory
   - Fewer memory accesses per operation

4. **Simplified Logic:**
   - Eliminates row-specific pattern extraction
   - Single-row focus aligns with actual platform rendering

## Compatibility

The optimization maintains full compatibility with the existing level code system and painting logic. All external interfaces remain the same, with only internal implementation details changed for efficiency.

## Files Modified

- `code_platform_system.h` - Updated function signatures and constants
- `code_platform_system.c` - Complete pattern system rewrite
- `paint.c` - Updated external function declarations and usage

## Testing Recommendations

1. Verify pattern extraction correctly identifies all platform configurations
2. Test pattern application maintains proper platform rendering
3. Confirm level code encoding/decoding works with new pattern IDs
4. Validate edge cases with neighboring block interactions
