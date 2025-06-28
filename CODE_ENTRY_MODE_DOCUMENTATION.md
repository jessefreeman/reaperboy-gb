# Code Entry Mode System Documentation

## Overview

The code entry mode system allows users to freely draw any platform pattern during code entry, including invalid single-tile platforms. When exiting code mode, the system automatically cleans up the tilemap to enforce the proper platform rules (minimum 2 tiles, maximum 8 tiles).

## Key Features

1. **Unrestricted Drawing**: During code entry mode, users can draw any pattern including single-tile platforms
2. **Proper Visual Display**: Platforms show correct end caps even during code entry (left/middle/right tiles)
3. **Automatic Cleanup**: When exiting code mode, invalid patterns are automatically corrected
4. **Rule Enforcement**: Platforms must be 2-8 tiles long after cleanup
5. **Seamless Transition**: The level code display is updated to reflect the final corrected patterns

## Usage Workflow

### Entering Code Entry Mode

1. Call `Enable Code Entry Mode` event before allowing user to edit codes
2. Users can now cycle characters and see any pattern drawn immediately
3. Single-tile platforms and other invalid patterns are allowed

### Exiting Code Entry Mode

1. Call `Cleanup Invalid Platforms` event to fix all invalid patterns
2. Call `Disable Code Entry Mode` event to return to normal validation
3. The tilemap now enforces proper platform rules for future edits

## New Events

### Enable Code Entry Mode

- **Event ID**: `EVENT_ENABLE_CODE_ENTRY_MODE`
- **Purpose**: Allows unrestricted pattern drawing
- **VM Function**: `vm_enable_code_entry_mode()`

### Disable Code Entry Mode

- **Event ID**: `EVENT_DISABLE_CODE_ENTRY_MODE`
- **Purpose**: Returns to normal validation rules
- **VM Function**: `vm_disable_code_entry_mode()`

### Cleanup Invalid Platforms

- **Event ID**: `EVENT_CLEANUP_INVALID_PLATFORMS`
- **Purpose**: Fixes all invalid platform patterns on the tilemap
- **VM Function**: `vm_cleanup_invalid_platforms()`

## Technical Implementation

### Core Functions

```c
// Mode management
void enable_code_entry_mode(void);
void disable_code_entry_mode(void);
UBYTE is_code_entry_mode_active(void);

// Unrestricted pattern application
void apply_raw_pattern_unrestricted(UBYTE segment_x, UBYTE segment_y, UWORD pattern);

// Cleanup functions
void cleanup_invalid_platforms(void);
void fix_platform_segment_rules(UBYTE segment_x, UBYTE segment_y);
```

### Pattern Application Logic

- **Code Entry Mode**: Uses `apply_raw_pattern_unrestricted()` which places platform tiles with proper end caps but allows invalid patterns like single tiles
- **Normal Mode**: Uses existing `apply_raw_pattern_tiles()` + `cleanup_platform_segment()` workflow which enforces 2-8 tile rules

### Cleanup Process

1. Processes all 16 platform segments
2. Uses existing `rebuild_platform_row()` function to enforce rules
3. Re-extracts platform data to update level code
4. Forces complete display update to show corrections

## Integration Examples

### Basic Code Entry Workflow

```
1. Enable Code Entry Mode
2. [User cycles through characters and draws patterns]
3. Cleanup Invalid Platforms
4. Disable Code Entry Mode
```

### Advanced Workflow with State Management

```
1. Save current state (optional)
2. Enable Code Entry Mode
3. [User input and pattern editing]
4. Cleanup Invalid Platforms
5. Validate results
6. Disable Code Entry Mode
7. Update game state
```

## Benefits

1. **User Experience**: Users can freely experiment with patterns without restrictions
2. **Visual Feedback**: Immediate visual feedback for all pattern combinations
3. **Automatic Correction**: No manual cleanup required from users
4. **Rule Enforcement**: Maintains game balance by enforcing platform rules
5. **Backward Compatibility**: Existing editor functionality remains unchanged

## Notes

- The global flag `code_entry_mode_active` tracks the current state
- All pattern validation is bypassed during code entry mode
- Cleanup uses the existing `rebuild_platform_row()` logic for consistency
- Level code display is automatically updated after cleanup
