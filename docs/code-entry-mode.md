# Code Entry Mode System

## Overview

The code entry mode system allows users to freely draw any platform pattern during code entry, including invalid single-tile platforms. When exiting code mode, the system automatically cleans up the tilemap to enforce proper platform rules.

## System Status

> **Note**: Based on current codebase analysis, this system appears to be partially implemented. The event files exist but the core functions may need verification/completion.

## Key Features

1. **Unrestricted Drawing**: During code entry mode, users can draw any pattern including single-tile platforms
2. **Proper Visual Display**: Platforms show correct end caps even during code entry (left/middle/right tiles)
3. **Automatic Cleanup**: When exiting code mode, invalid patterns are automatically corrected
4. **Rule Enforcement**: Platforms must be 2-8 tiles long after cleanup
5. **Seamless Transition**: The level code display is updated to reflect the final corrected patterns

## Available Events

### Exit Code Entry & Cleanup

- **Event ID**: `EVENT_EXIT_CODE_ENTRY_WITH_CLEANUP`
- **File**: `eventExitCodeEntryWithCleanup.js`
- **Purpose**: Exits code entry mode and automatically fixes invalid platforms
- **VM Function**: `vm_exit_code_entry_mode_with_cleanup()`

### Enable Code Entry Mode

- **Event ID**: `EVENT_ENABLE_CODE_ENTRY_MODE`
- **File**: `eventEnableCodeEntryMode.js`
- **Purpose**: Allows unrestricted pattern drawing
- **VM Function**: `vm_enable_code_entry_mode()`

### Disable Code Entry Mode

- **Event ID**: `EVENT_DISABLE_CODE_ENTRY_MODE`
- **File**: `eventDisableCodeEntryMode.js`
- **Purpose**: Returns to normal validation rules
- **VM Function**: `vm_disable_code_entry_mode()`

### Cleanup Invalid Platforms

- **Event ID**: `EVENT_CLEANUP_INVALID_PLATFORMS`
- **File**: `eventCleanupInvalidPlatforms.js`
- **Purpose**: Fixes all invalid platform patterns on the tilemap
- **VM Function**: `vm_cleanup_invalid_platforms()`

## Core Functions (Implementation Status Unknown)

```c
// Mode management (paint_core.h)
void enable_code_entry_mode(void);
void disable_code_entry_mode(void);
UBYTE is_code_entry_mode_active(void);

// VM wrapper functions (paint_vm.h)
void vm_enable_code_entry_mode(void);
void vm_disable_code_entry_mode(void);
void vm_exit_code_entry_mode_with_cleanup(void);
void vm_cleanup_invalid_platforms(void);

// Unrestricted pattern application (paint_vm.h)
void apply_raw_pattern_unrestricted(UBYTE segment_x, UBYTE segment_y, UWORD pattern);

// Combined exit and cleanup (paint_vm.h)
void exit_code_entry_mode_with_cleanup(void);

// Cleanup functions (paint_platform.h)
void cleanup_invalid_platforms(void);
void fix_platform_segment_rules(UBYTE segment_x, UBYTE segment_y);
```

## Usage Workflow

### Recommended Usage Pattern

```
// Start code entry session
→ Enable Code Entry Mode

// User can now freely edit patterns
// [User interaction with level code display]
// [User cycles characters and sees immediate feedback]
// [Single-tile platforms and invalid patterns are allowed]

// End code entry session with automatic cleanup
→ Exit Code Entry & Cleanup
```

### Alternative Pattern

```
// Start code entry session
→ Enable Code Entry Mode

// [User edits patterns]

// Manual cleanup and exit
→ Cleanup Invalid Platforms
→ Disable Code Entry Mode
```

## Technical Implementation Details

### During Code Entry Mode

1. `apply_pattern_to_tilemap()` should use `apply_raw_pattern_unrestricted()` from `paint_vm.h`
2. Patterns are drawn directly without validation
3. Single-tile platforms and invalid patterns are allowed
4. Level code is updated to match exactly what was requested

### During Cleanup Process

1. `cleanup_invalid_platforms()` from `paint_platform.h` iterates through all 16 platform segments
2. Calls `fix_platform_segment_rules()` for each segment
3. This processes both rows in each segment
4. `rebuild_platform_row()` enforces the actual platform rules:
   - **Single-Tile Removal**: Converts single tiles to empty space
   - **Valid Platform Processing**: Applies correct left/middle/right tiles
   - **Length Enforcement**: Forces platform finalization at max length

## Integration with Modular Paint System

### Module Responsibilities

- **paint_core.h**: Manages code entry mode state and basic validation
- **paint_platform.h**: Contains cleanup and platform rule enforcement
- **paint_vm.h**: Provides VM wrapper functions for GB Studio events
- **paint.h**: Umbrella header providing unified access to all code entry features

## Example Use Cases

### Basic Code Entry Session

```
→ Enable Code Entry Mode
// [User freely edits level code]
→ Exit Code Entry & Cleanup
```

### Code Entry with Save/Load

```
→ Save Level Code              // Backup current state
→ Enable Code Entry Mode
// [User edits patterns]
// If user cancels:
→ Load Level Code              // Restore backup
→ Disable Code Entry Mode
// If user confirms:
→ Exit Code Entry & Cleanup
→ Save Level Code              // Save corrected version
```

### Code Import from External Source

```
→ Enable Code Entry Mode
// Apply each character of imported code
→ Exit Code Entry & Cleanup
// Verify the imported level is playable
```

## Integration with Platform Rules

### Rule Enforcement Summary

#### ✅ Single-Tile Platform Removal

- **Detection**: `current_len == 1` condition in `rebuild_platform_row()`
- **Action**: `replace_meta_tile(seq_start, y, TILE_EMPTY, 1)`
- **Safety**: Enemies above platform are removed first

#### ✅ 2-8 Tile Rule Enforcement

- **Minimum**: Single tiles removed (enforces 2+ minimum)
- **Maximum**: `PLATFORM_MAX_LENGTH = 8` enforces 8-tile maximum
- **Processing**: Valid platforms get proper left/middle/right tile styling

## Implementation Notes

> **Important**: This documentation describes the intended behavior based on the available event files. The actual implementation status of the core C functions should be verified in the current codebase before relying on this functionality.

## Key Benefits

1. **User-Friendly**: Users can experiment freely during code entry
2. **Automatic Correction**: Invalid patterns are fixed automatically
3. **Visual Feedback**: Users see exactly what their code creates
4. **Rule Compliance**: Final result always follows platform rules
5. **Seamless Integration**: Works with existing level code and paint systems
