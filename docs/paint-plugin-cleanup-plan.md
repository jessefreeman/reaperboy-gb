# TilemapPainter Plugin Cleanup Plan

## Overview

This document outlines a comprehensive cleanup plan for the TilemapPainter plugin, focusing on removing redundancy, improving maintainability, and preparing for future abstraction while maintaining all existing functionality.

## Current State Analysis

### Plugin Structure Issues

1. **Empty Stub Files**: Multiple files contain no actual code
2. **Misplaced Dependencies**: Level code generation logic mixed with paint logic
3. **Over-Modularization**: Too many small files for simple functionality
4. **Tight Coupling**: Direct calls to level code system functions
5. **Redundant Code**: Duplicate includes and external declarations

### Files in Current Structure

```
plugins/TilemapPainter/
├── engine/
│   ├── engine.json                  [KEEP - Configuration]
│   ├── include/
│   │   ├── code_level_core.h        [REMOVE - Wrong plugin]
│   │   ├── code_platform_system.h   [REMOVE - Wrong plugin]
│   │   ├── code_player_system.h     [REMOVE - Wrong plugin]
│   │   ├── paint.h                  [CLEAN - Main header]
│   │   ├── paint_core.h             [KEEP - Core functions]
│   │   ├── paint_entity.h           [KEEP - Entity management]
│   │   ├── paint_platform.h         [KEEP - Platform logic]
│   │   ├── paint_ui.h               [KEEP - UI state]
│   │   ├── paint_vm.h               [KEEP - VM wrappers]
│   │   └── tile_utils.h             [KEEP - Utilities]
│   └── src/core/
│       ├── paint.c                  [REMOVE - Empty stub]
│       ├── paint_clean.c            [REMOVE - Empty stub]
│       ├── paint_core.c             [CLEAN - Main logic]
│       ├── paint_entity.c           [KEEP - Entity functions]
│       ├── paint_platform.c         [KEEP - Platform functions]
│       ├── paint_stub.c             [REMOVE - Empty stub]
│       ├── paint_ui.c               [KEEP - UI functions]
│       ├── paint_vm.c               [CLEAN - VM wrappers]
│       └── tile_utils.c             [KEEP - Utilities]
├── events/
│   ├── eventEnableEditor.js         [KEEP]
│   ├── eventGetBrushTile.js         [KEEP]
│   ├── eventPaintTile.js            [KEEP]
│   └── eventSetupPaintActors.js     [KEEP]
└── README.md                        [CLEAN - Update docs]
```

## Proposed Solution Architecture

### Phase 1: Immediate Cleanup (Priority)

Clean up the existing TilemapPainter plugin first, maintaining all current functionality:

```
plugins/TilemapPainter/
├── engine/
│   ├── engine.json                  [CLEAN - Remove unused settings]
│   ├── include/
│   │   ├── paint.h                  [CLEAN - Consolidate interface]
│   │   ├── paint_core.h             [KEEP - Core functions]
│   │   ├── paint_entity.h           [KEEP - Entity management]
│   │   ├── paint_platform.h         [KEEP - Platform logic]
│   │   ├── paint_ui.h               [KEEP - UI state]
│   │   ├── paint_vm.h               [KEEP - VM wrappers]
│   │   └── tile_utils.h             [KEEP - Utilities]
│   └── src/core/
│       ├── paint_core.c             [CLEAN - Simplify dependencies]
│       ├── paint_entity.c           [KEEP - Entity functions]
│       ├── paint_platform.c         [KEEP - Platform functions]
│       ├── paint_ui.c               [KEEP - UI functions]
│       ├── paint_vm.c               [CLEAN - Simplify VM wrappers]
│       └── tile_utils.c             [KEEP - Utilities]
├── events/
│   ├── eventEnableEditor.js         [KEEP]
│   ├── eventGetBrushTile.js         [KEEP]
│   ├── eventPaintTile.js            [KEEP]
│   └── eventSetupPaintActors.js     [KEEP]
└── README.md                        [CLEAN - Update docs]
```

### Phase 2: Validate and Test

Test all functionality to ensure cleanup didn't break anything:

- All paint operations work correctly
- Entity placement functions properly
- Platform validation works
- VM integration intact

### Phase 3: Create ReaperBoy Shared Plugin (After Validation)

Once cleanup is validated, create shared plugin for game-specific functionality:

```
plugins/ReaperBoyShared/
├── engine/
│   ├── engine.json                  [Game-specific settings]
│   ├── include/
│   │   ├── reaperboy_level_core.h   [Level structure definitions]
│   │   ├── reaperboy_platform.h     [Platform constants and types]
│   │   ├── reaperboy_entities.h     [Entity definitions]
│   │   └── reaperboy_tiles.h        [Tile type definitions]
│   └── src/core/
│       ├── reaperboy_level_core.c   [Core level functions]
│       ├── reaperboy_platform.c     [Platform utilities]
│       ├── reaperboy_entities.c     [Entity utilities]
│       └── reaperboy_tiles.c        [Tile utilities]
└── README.md
```

### Phase 4: Abstract Interface Layer (Future)

Create abstraction layer for reusability in other games:

```
plugins/ReaperBoyShared/
├── include/
│   └── paint_interface.h            [Abstract paint interface]
```

## Detailed Cleanup Tasks

### Task 1: Remove Empty Stub Files (IMMEDIATE)

**Files to Delete:**

- `paint.c` (25 lines of comments only)
- `paint_clean.c` (51 lines of comments only)
- `paint_stub.c` (42 lines of comments only)

**Impact:** Reduces clutter, no functional impact.
**Risk:** None - these files contain no actual code.

### Task 2: Clean Up Dependencies (IMMEDIATE)

**Files to Modify:**

- `paint_core.c`: Remove unused external function declarations
- `paint_vm.c`: Remove unnecessary level code system includes
- `paint_platform.c`: Remove unnecessary level code system includes
- `paint_entity.c`: Remove unnecessary level code system includes

**Strategy:** Keep only the includes and external declarations that are actually used.

### Task 3: Consolidate Headers (IMMEDIATE)

**Current Issues:**

- `paint.h` has too many refactoring comments
- Redundant declarations across multiple headers
- Unclear interface boundaries

**Proposed Solution:**

```c
// paint.h - Clean main interface
#ifndef PAINT_H
#define PAINT_H

#include <gbdk/platform.h>
#include "vm.h"

// Core paint functions
void paint(UBYTE x, UBYTE y) BANKED;
UBYTE get_brush_tile_state(UBYTE x, UBYTE y) BANKED;

// VM interface
void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED;
void vm_enable_editor(SCRIPT_CTX *THIS) BANKED;
void vm_paint(SCRIPT_CTX *THIS) BANKED;
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED;

#endif // PAINT_H
```

### Task 4: Simplify Engine Configuration (IMMEDIATE)

**Current engine.json Issues:**

- Mixed paint and level code settings
- Unclear grouping
- Some settings may be unused

**Proposed Clean Configuration:**

```json
{
  "version": "4.2.0-e7",
  "fields": [
    {
      "key": "PLATFORM_MAX_LENGTH",
      "label": "Maximum Platform Length (tiles)",
      "group": "TilemapPainter",
      "type": "slider",
      "cType": "define",
      "defaultValue": 8,
      "min": 2,
      "max": 20,
      "description": "Maximum length for platform segments in tiles"
    },
    {
      "key": "MAX_ENEMIES",
      "label": "Maximum Enemies Per Level",
      "group": "TilemapPainter",
      "type": "slider",
      "cType": "define",
      "defaultValue": 6,
      "min": 1,
      "max": 10,
      "description": "Maximum number of enemies that can be placed in a level"
    },
    {
      "key": "PLATFORM_Y_MIN",
      "label": "Platform Y Start Row",
      "group": "TilemapPainter",
      "type": "slider",
      "cType": "define",
      "defaultValue": 12,
      "min": 8,
      "max": 20,
      "description": "Starting Y coordinate for platform placement area"
    },
    {
      "key": "PLATFORM_Y_MAX",
      "label": "Platform Y End Row",
      "group": "TilemapPainter",
      "type": "slider",
      "cType": "define",
      "defaultValue": 19,
      "min": 12,
      "max": 24,
      "description": "Ending Y coordinate for platform placement area"
    }
  ]
}
```

### Task 5: Remove Misplaced Headers (AFTER VALIDATION)

**Files to Remove from TilemapPainter (move to ReaperBoyShared later):**

- `code_level_core.h` → Keep for now, move to ReaperBoyShared in Phase 3
- `code_platform_system.h` → Keep for now, move to ReaperBoyShared in Phase 3
- `code_player_system.h` → Keep for now, move to ReaperBoyShared in Phase 3

**Impact:** Proper separation of concerns, but only after cleanup is validated.

### Task 6: Create Missing Event (IMMEDIATE)

**Missing File:**

- `eventCycleCharacter.js` - Referenced in migration plan but doesn't exist

**Action:** Determine if this event is needed or remove from documentation.

## Implementation Strategy

### Phase 1: Immediate Cleanup (1-2 hours)

**Priority: Remove obvious problems without breaking functionality**

1. **Remove Dead Code**

   - Delete empty stub files (`paint.c`, `paint_clean.c`, `paint_stub.c`)
   - Remove unused external function declarations
   - Clean up redundant includes

2. **Consolidate Headers**

   - Clean up `paint.h` by removing refactoring comments
   - Simplify interface to only essential functions
   - Remove redundant declarations

3. **Simplify Engine Configuration**
   - Keep only the settings that are actually used
   - Ensure all settings have clear descriptions
   - Remove any unused configuration options

### Phase 2: Validation and Testing (1-2 hours)

**Priority: Ensure nothing is broken**

1. **Compile Testing**

   - Verify all files compile without errors
   - Check for missing dependencies
   - Ensure no undefined symbols

2. **Functional Testing**

   - Test all paint operations
   - Verify entity placement
   - Check platform validation
   - Test VM integration

3. **Performance Testing**
   - Verify no performance degradation
   - Check memory usage
   - Validate real-time painting

### Phase 3: Create ReaperBoyShared Plugin (2-3 hours)

**Priority: Extract game-specific code after validation**

1. **Create Plugin Structure**

   - Set up ReaperBoyShared directory structure
   - Create basic engine.json with game-specific settings
   - Create README.md

2. **Move Game-Specific Code**

   - Extract level core definitions from existing headers
   - Move platform constants and types
   - Move entity definitions and tile types
   - Update includes in TilemapPainter

3. **Create Interface Layer**
   - Define clean interfaces between plugins
   - Implement callback system if needed
   - Test plugin interactions

### Phase 4: Final Validation (1-2 hours)

**Priority: Ensure separation works correctly**

1. **Integration Testing**

   - Test with existing projects
   - Verify no breaking changes
   - Check plugin dependencies

2. **Documentation Update**

   - Update README files
   - Document new plugin structure
   - Create usage examples

3. **Performance Validation**
   - Ensure no performance regression
   - Validate memory usage
   - Check build times

## Benefits of This Approach

### Immediate Benefits (Phase 1)

1. **Cleaner Codebase**: Remove 3 empty files immediately
2. **Reduced Clutter**: Clean up comments and redundant code
3. **Better Organization**: Simplified header structure
4. **Easier Maintenance**: Less code to maintain
5. **Lower Risk**: No functional changes, just cleanup

### Validation Benefits (Phase 2)

1. **Confidence Building**: Verify cleanup didn't break anything
2. **Early Problem Detection**: Catch issues before adding complexity
3. **Baseline Establishment**: Set performance benchmarks
4. **Functionality Verification**: Ensure all features still work

### Long-term Benefits (Phase 3-4)

1. **Proper Separation**: Game-specific code isolated
2. **Easier Abstraction**: Clean foundation for future abstraction
3. **Reusability**: Paint logic can be reused in other games
4. **Modularity**: Plugins can be developed independently
5. **Extensibility**: Easy to add new paint features

### Risk Mitigation Advantages

1. **Incremental Approach**: Small, safe changes first
2. **Early Validation**: Test each phase before proceeding
3. **Minimal Disruption**: Cleanup first, abstraction later
4. **Easy Rollback**: Simple to revert if issues arise

## Risk Mitigation

### Backup Strategy

1. **Complete Backup**: Create full backup before starting
2. **Incremental Testing**: Test after each phase
3. **Rollback Plan**: Document rollback procedures

### Validation Approach

1. **Unit Testing**: Test each component in isolation
2. **Integration Testing**: Test plugin interactions
3. **Regression Testing**: Ensure existing functionality works

## Success Criteria

1. **No Functional Changes**: All existing features work identically
2. **Cleaner Architecture**: Simplified, maintainable code structure
3. **Proper Separation**: Paint logic separated from level code logic
4. **Performance Maintained**: No performance degradation
5. **Documentation Complete**: Updated and accurate documentation

## Next Steps

1. **Start with Phase 1**: Begin immediate cleanup (safest approach)
2. **Remove Empty Files**: Delete stub files first
3. **Clean Headers**: Simplify paint.h interface
4. **Test Thoroughly**: Validate cleanup before proceeding
5. **Create ReaperBoyShared**: Only after cleanup is validated
6. **Document Changes**: Update all relevant documentation

This revised cleanup approach prioritizes safety and validation while achieving the same end goal. The immediate cleanup phase has minimal risk and provides instant benefits, while the abstraction phase is deferred until we're confident the cleanup is successful.
