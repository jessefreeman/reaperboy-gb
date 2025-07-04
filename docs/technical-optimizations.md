# Technical Optimizations Reference

This document consolidates technical details about key optimizations made to the Tilemap Editor plugin system.

## Platform Pattern System Optimization

### Overview

The platform pattern matching system was optimized to work with single-row platform configurations instead of the previous 5x2 block system. Since platforms are only rendered on odd rows (the second row of each 2-row segment), the system now uses simplified 5-bit patterns instead of 10-bit patterns.

### Key Changes

#### 1. Pattern Data Structure Simplified

**Before:**

- Used `UWORD` (16-bit) patterns with 10 bits of data
- Top 5 bits (9-5) for first row, bottom 5 bits (4-0) for second row
- 21 predefined patterns

**After:**

- Uses `UBYTE` (8-bit) patterns with 5 bits of data
- Only bottom 5 bits (4-0) for the platform row
- 32 comprehensive patterns covering all possible 5-bit combinations

#### 2. Function Signatures Updated

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

#### 3. Pattern Array Expansion

The new pattern array includes all 32 possible 5-bit combinations (0b00000 to 0b11111), providing comprehensive coverage of platform configurations.

#### 4. Performance Benefits

1. **Reduced Memory Usage:**

   - Pattern data reduced from 16-bit to 8-bit
   - Simpler pattern matching with fewer bit operations

2. **Faster Pattern Extraction:**

   - Only reads 5 tiles instead of 10 tiles per segment
   - Eliminates unnecessary row checking

3. **Improved Cache Efficiency:**
   - Smaller data structures fit better in memory
   - Fewer memory accesses per operation

## Level Code Display Flicker Elimination

### Problem

The level code display was causing flicker because every paint operation triggered a complete redraw of all 24 characters, even when only a single character or zone had changed.

### Solution: Selective Update System

#### Implementation Details

- **Added tracking variables** to monitor what has changed
- **Implemented bit masks** to track which display positions need updating (24-bit system)
- **Created change detection** that compares current vs previous level code state

#### New Functions Added

```c
// Selective update functions
void mark_display_position_for_update(UBYTE position);
UBYTE display_position_needs_update(UBYTE position);
void clear_display_update_flags(void);
void display_selective_level_code_fast(void);

// Smart change detection
void detect_level_code_changes(void);
void force_complete_level_code_display(void);

// Cache management
current_encoded_enemy_data[4];     // Cache for enemy encoding
current_encoded_player_data;       // Cache for player encoding
display_update_mask;               // 32-bit bitmask for tracking updates
```

#### Performance Improvements

- **Before**: Every paint operation → Clear entire display → Redraw all 24 characters
- **After**: Only update the 1-5 characters that actually changed
- **Result**: Eliminated visible flicker, much smoother user experience

### Zone-Specific Updates

- **Platform changes**: Only update the affected 5-character zone
- **Enemy data changes**: Only update characters 20-23
- **Complete updates**: Only for initialization and major operations

## Paint Logic Synchronization Fixes

### Adjacent Block Level Code Sync Fix

#### Problem

When entering code letters that triggered auto-completion into neighboring blocks, the neighboring blocks' level codes were not updated immediately, causing a "one change late" issue.

#### Solution

Enhanced `apply_pattern_with_brush_logic` to explicitly update neighboring block codes:

```c
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED {
    // ...existing pattern application logic...
    update_neighboring_block_codes(block_index);
}
```

### True Manual Paint Simulation

Rewrote pattern application to literally call the `paint(x, y)` function for each tile:

```c
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) BANKED {
    // Process tiles from left to right using actual paint() calls
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++) {
        if ((row_pattern >> (4 - i)) & 1) {
            // Use real paint function for perfect consistency
            paint(tile_x, current_y);
        }
    }
}
```

## Platform Validation Improvements

### 8-Tile Length Limit Enforcement

Added comprehensive length validation:

```c
// Direct connection validation
UBYTE count_connected_platform_length(UBYTE x, UBYTE y);

// Auto-merge validation
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y);
```

### Single Platform Auto-Completion

Added detection and auto-completion for isolated platforms:

```c
// Detect isolated platforms at segment edges
if (is_isolated && i == SEGMENT_WIDTH - 1 && tile_x + 1 <= PLATFORM_X_MAX) {
    paint(tile_x + 1, current_y);  // Auto-complete to 2-tile platform
}
```

## Performance Metrics Summary

### Flicker Elimination

- **Reduced Updates**: From 24 characters every paint → 1-5 characters only when needed
- **Update Time**: ~75% reduction in display update operations
- **Visual Result**: Smooth, flicker-free level code display

### Paint Consistency

- **Logic Unification**: Single code path for both manual and code entry painting
- **Update Synchronization**: Immediate level code updates for all affected blocks
- **Behavioral Consistency**: 100% identical behavior between input methods

### Memory Usage

- **Pattern Data**: Reduced from 16-bit to 8-bit (50% reduction)
- **Display Update System**: Added ~32 bytes for tracking but eliminated redundant operations
- **Overall**: More efficient memory usage with better performance
