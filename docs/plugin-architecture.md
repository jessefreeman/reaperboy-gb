# Plugin Architecture & Migration

## Overview

This document describes the complete plugin architecture migration from MetaTile8Plugin to TilemapEditor and the current plugin structure.

## Migration Summary

### From: MetaTile8Plugin → To: TilemapEditor

The codebase was successfully migrated to a new, cleaner plugin structure with better organization and separation of concerns.

## Current Plugin Structure

```
plugins/TilemapEditor/
├── engine/
│   ├── include/          # Header files (.h)
│   │   ├── code_gen.h    # Level code generation and persistence
│   │   ├── paint.h       # Platform painting and validation
│   │   └── tile_utils.h  # Tile type detection utilities
│   └── src/
│       └── core/         # Source files (.c)
│           ├── code_gen.c    # Complete level code system
│           ├── paint.c       # Platform paint logic with validation
│           └── tile_utils.c  # Tile type mapping functions
└── events/               # Event scripts (.js)
    ├── eventCleanupInvalidPlatforms.js
    ├── eventCycleCharacter.js
    ├── eventDeleteTile.js
    ├── eventDisableCodeEntryMode.js
    ├── eventDrawSegments.js
    ├── eventEnableCodeEntryMode.js
    ├── eventEnableEditor.js
    ├── eventExitCodeEntryWithCleanup.js
    ├── eventGenerateDisplayLevelCode.js
    ├── eventGetBrushPreviewTile.js
    ├── eventGetBrushTile.js
    ├── eventHasSavedLevelCode.js
    ├── eventLoadLevelCode.js
    ├── eventPaintTile.js
    ├── eventSaveLevelCode.js
    ├── eventSetupMap.js
    └── eventUpdateCode.js
```

## Core Components

### Header Files (engine/include/)

#### code_gen.h

- Level code generation and persistence system
- Platform pattern definitions and constants
- Variable-based storage system declarations
- Display system function prototypes

#### paint.h

- Platform painting and validation functions
- Brush preview system declarations
- Platform length validation prototypes
- Paint operation function signatures

#### tile_utils.h

- Tile type detection utilities
- Meta tile mapping constants
- Utility function declarations for tile operations

### Source Files (engine/src/core/)

#### code_gen.c

- **Size**: ~1917 lines (optimized from ~1012 lines)
- **Features**:
  - Complete level code system with lossless 5-bit platform pattern encoding
  - Variable-based persistence across 6 global variables
  - 24-character level code display system
  - Selective update system for flicker prevention
  - Pattern application with brush logic simulation

#### paint.c

- **Features**:
  - Platform paint logic with 8-tile length limits
  - Comprehensive validation system
  - Auto-completion for single platforms
  - Platform merging and conflict detection
  - Integration with level code updates

#### tile_utils.c

- **Features**:
  - Tile type mapping functions
  - Meta tile utility operations
  - Support functions for paint and code generation systems

## Event System Architecture

### Core Events

#### Level Code Management

- **Save Level Code**: `eventSaveLevelCode.js`
- **Load Level Code**: `eventLoadLevelCode.js`
- **Check Saved Level**: `eventHasSavedLevelCode.js`

#### Code Entry Mode

- **Enable Code Entry**: `eventEnableCodeEntryMode.js`
- **Disable Code Entry**: `eventDisableCodeEntryMode.js`
- **Exit with Cleanup**: `eventExitCodeEntryWithCleanup.js`
- **Cleanup Invalid**: `eventCleanupInvalidPlatforms.js`

#### Paint Operations

- **Paint Tile**: `eventPaintTile.js`
- **Delete Tile**: `eventDeleteTile.js`
- **Get Brush State**: `eventGetBrushTile.js`
- **Get Preview State**: `eventGetBrushPreviewTile.js`

#### Display & Navigation

- **Draw Segments**: `eventDrawSegments.js`
- **Update Code Display**: `eventUpdateCode.js`
- **Generate Display**: `eventGenerateDisplayLevelCode.js`
- **Cycle Character**: `eventCycleCharacter.js`

#### Setup & Initialization

- **Setup Map**: `eventSetupMap.js`
- **Enable Editor**: `eventEnableEditor.js`

### Event Group Classification

All events belong to the `"TilemapEditor"` group for proper categorization in GB Studio's event browser.

## Integration with GB Studio

### Variable Requirements

The plugin requires 6 global variables for level code persistence:

```c
#define VAR_LEVEL_CODE_PART_1 0 // Update to match your variable IDs
#define VAR_LEVEL_CODE_PART_2 1
#define VAR_LEVEL_CODE_PART_3 2
#define VAR_LEVEL_CODE_PART_4 3
#define VAR_LEVEL_CODE_PART_5 4
#define VAR_LEVEL_CODE_PART_6 5
```

### Bank Management

The plugin uses bank 254 for core functions:

```c
#pragma bank 254
```

### VM Integration

All events compile to native VM function calls:

```javascript
export const compile = (input, helpers) => {
  const { _callNative, _addComment } = helpers;
  _addComment("Description of operation");
  _callNative("vm_function_name");
};
```

## Key Features Preserved

### ✅ Lossless Level Code System

- Complete 5-bit platform pattern encoding
- Variable-based persistence
- 24-character display system

### ✅ Platform Paint Logic

- 8-tile length limits with validation
- Auto-completion and merging
- Visual-logical consistency

### ✅ Performance Optimizations

- Selective level code display updates
- Flicker prevention system
- Smart change detection

### ✅ Code Entry Mode Support

- Unrestricted pattern drawing during entry
- Automatic cleanup on exit
- Rule enforcement system

## Migration Benefits

### Improved Organization

- **Clear Separation**: Headers, source, and events in logical directories
- **Reduced Complexity**: Removed duplicate and legacy systems
- **Better Maintainability**: Simplified architecture with single responsibility

### Enhanced Performance

- **Optimized Code**: Removed redundant functions and systems
- **Efficient Updates**: Selective display updates instead of full redraws
- **Consistent Behavior**: Unified code paths for all operations

### Developer Experience

- **Clear API**: Well-defined function signatures and responsibilities
- **Easy Integration**: Standard GB Studio event pattern
- **Comprehensive Documentation**: Each component thoroughly documented

## Future Extensibility

The current architecture supports easy extension through:

### Additional Events

- New `.js` files in `events/` directory
- Standard GB Studio event pattern
- Access to all VM functions

### Engine Extensions

- Additional `.c` files in `engine/src/core/`
- Corresponding `.h` files in `engine/include/`
- Integration with existing systems

### Feature Additions

- Pattern system extensions
- Additional validation rules
- Enhanced display options
- Save/load format enhancements

The plugin architecture is designed to be modular, maintainable, and extensible while providing a complete level editing solution for GB Studio projects.
