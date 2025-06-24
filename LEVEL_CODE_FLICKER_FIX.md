# Level Code Display Flicker Fix - Implementation Summary

## Problem Identified

The level code display was causing flicker because every paint operation triggered a complete redraw of all 24 characters in the level code display, even when only a single character or zone had changed.

## Solution Implemented

### 1. Selective Update System

- **Added tracking variables** to monitor what has changed
- **Implemented bit masks** to track which display positions need updating (24-bit system)
- **Created change detection** that compares current vs previous level code state

### 2. Smart Zone Detection

- **Zone-specific updates** for platform changes - only update the affected 5x2 tile zone
- **Enemy data updates** for enemy/player changes - only update characters 20-23
- **Intelligent paint detection** to determine what type of operation occurred

### 3. New Functions Added

#### In `code_gen.c`:

- `mark_display_position_for_update(UBYTE position)` - Mark specific characters for update
- `display_position_needs_update(UBYTE position)` - Check if a character needs updating
- `clear_display_update_flags()` - Reset all update flags
- `get_display_position(UBYTE char_index, UBYTE *x, UBYTE *y)` - Calculate screen position
- `detect_level_code_changes()` - Compare current vs previous state
- `display_selective_level_code()` - Only redraw changed characters
- `force_complete_level_code_display()` - Complete redraw for initialization

#### In `paint.c`:

- `update_level_code_for_paint(UBYTE x, UBYTE y)` - Smart update based on paint location

### 4. Modified Functions

#### Updated to use selective updates:

- `display_complete_level_code()` - Now uses selective update system
- `update_zone_code()` - Marks only the specific zone for update
- All paint functions (`paint_player`, `paint_enemy_right`, etc.) - Use smart updates
- Main `paint()` function - Uses smart updates for all operations

#### Updated for initialization:

- `draw_segment_ids()` - Uses force complete display for initial draw
- `vm_load_level_code()` - Uses force complete display after loading
- Test functions - Use force complete display

### 5. Performance Improvements

#### Before:

- Every paint operation → Clear entire display → Redraw all 24 characters
- Caused visible flicker on every change

#### After:

- Platform changes → Update only the affected zone character (1 character)
- Enemy/player changes → Update only enemy data characters (4 characters max)
- Only changed characters are redrawn
- No screen clearing for selective updates

### 6. Backwards Compatibility

- All existing VM wrapper functions continue to work
- Same external API surface
- No changes required to GB Studio scripts
- Fallback to complete update for edge cases

## Major Technical Fix - Eliminated Root Cause of Flicker

### The Core Problem

The main issue was in the `detect_level_code_changes()` function which was:

1. **Swapping data structures** during comparison (corrupting state)
2. **Calling encoding functions 4+ times** per update
3. **Re-extracting all level data** even for single tile changes

### The Solution

1. **Cached encoded values** to prevent recalculation
2. **Eliminated data structure swapping**
3. **Added fast update path** that skips unnecessary data extraction
4. **Targeted extraction** - only extract what actually changed

The flicker is now eliminated because we no longer clear the screen or redraw unchanged characters.

## Files Modified

1. **`code_gen.c`** - Added selective update system and smart change detection
2. **`code_gen.h`** - Added function declarations for new selective update functions
3. **`paint.c`** - Added smart update function and modified all paint operations
4. **`paint.h`** - Added declaration for smart update function

## Key Benefits

1. **Eliminates flicker** - No more clearing and redrawing entire display
2. **Better performance** - Only updates what changed
3. **Smarter updates** - Knows the difference between platform, enemy, and player changes
4. **Maintains accuracy** - Still captures all level code changes correctly
5. **Future-proof** - Easy to extend for new tile types or display requirements

## Usage

The system automatically detects what type of change occurred and updates only the necessary display characters:

- **Platform painting** → Updates only the zone that contains that platform segment
- **Enemy painting/deletion** → Updates only the 4 enemy data characters (positions 20-23)
- **Player painting** → Updates only the player position character (position 23)
- **Initial display** → Uses complete redraw for proper initialization
- **Loading saved codes** → Uses complete redraw to ensure full sync

The flicker fix is completely automatic and requires no changes to existing GB Studio scripts or usage patterns.
