# Plugin Migration Plan: TilemapEditor Separation

## Overview

This document outlines the plan to separate the current monolithic **TilemapEditor** plugin into two focused, independent plugins while maintaining all existing functionality.

## Current State

The **TilemapEditor** plugin currently contains:

- Interactive tilemap painting system
- Level code generation and persistence
- Mixed responsibilities that make maintenance difficult

## Target Architecture

### Plugin 1: TilemapPainter

**Responsibility**: Interactive level editing and real-time painting

- Focus: User interaction, visual feedback, tilemap manipulation
- Dependencies: None (standalone)
- Use Case: In-game level editors, interactive design tools

### Plugin 2: LevelCodeSystem

**Responsibility**: Level serialization, persistence, and code manipulation

- Focus: Data persistence, level encoding/decoding, save/load operations
- Dependencies: None (standalone)
- Use Case: Level sharing, save systems, procedural generation

## Migration Principles

1. **Zero Breaking Changes**: All existing functionality must continue to work
2. **Backward Compatibility**: Existing projects should not require changes
3. **Clean Separation**: No cross-plugin dependencies
4. **Independent Testing**: Each plugin should be testable in isolation
5. **Clear Documentation**: Updated docs for both plugins

## File Organization Plan

### Current Structure

```
plugins/TilemapEditor/
├── events/
│   ├── eventSetupPaintActors.js     [PAINT]
│   ├── eventEnableEditor.js         [PAINT]
│   ├── eventPaintTile.js            [PAINT]
│   ├── eventGetBrushTile.js         [PAINT]
│   └── eventCycleCharacter.js       [PAINT]
├── engine/
│   ├── engine.json                  [MIXED]
│   ├── include/
│   │   ├── paint*.h                 [PAINT]
│   │   ├── code_*.h                 [CODE]
│   │   └── tile_utils.h             [SHARED]
│   └── src/core/
│       ├── paint*.c                 [PAINT]
│       ├── code_*.c                 [CODE]
│       └── tile_utils.c             [SHARED]
└── README.md                        [MIXED]
```

### Target Structure

```
plugins/TilemapPainter/
├── events/
│   ├── eventSetupPaintActors.js
│   ├── eventEnableEditor.js
│   ├── eventPaintTile.js
│   ├── eventGetBrushTile.js
│   └── eventCycleCharacter.js
├── engine/
│   ├── engine.json                  [Paint-specific settings]
│   ├── include/
│   │   ├── paint*.h
│   │   └── tile_utils.h
│   └── src/core/
│       ├── paint*.c
│       └── tile_utils.c
└── README.md

plugins/LevelCodeSystem/
├── events/
│   └── [To be created later]
├── engine/
│   ├── engine.json                  [Code-specific settings]
│   ├── include/
│   │   └── code_*.h
│   └── src/core/
│       └── code_*.c
└── README.md
```

## Component Classification

### TilemapPainter Components

**Events**:

- `eventSetupPaintActors.js` - Configure paint actors
- `eventEnableEditor.js` - Activate editor mode
- `eventPaintTile.js` - Handle tile painting
- `eventGetBrushTile.js` - Get brush preview
- `eventCycleCharacter.js` - Cycle paint tools

**Engine Headers**:

- `paint.h` - Main paint umbrella header
- `paint_core.h` - Core painting logic
- `paint_platform.h` - Platform validation
- `paint_entity.h` - Entity placement
- `paint_ui.h` - UI feedback system
- `paint_vm.h` - VM wrappers for painting
- `tile_utils.h` - Tile utilities (shared)

**Engine Sources**:

- `paint*.c` - All painting implementation files
- `tile_utils.c` - Tile utilities (shared)

**Engine Settings**:

- `PLATFORM_MAX_LENGTH` - Maximum platform length
- `MAX_ENEMIES` - Maximum enemies per level
- `PLATFORM_Y_MIN` - Platform Y start row
- `PLATFORM_Y_MAX` - Platform Y end row

### LevelCodeSystem Components

**Events**:

- [To be created in future phase]

**Engine Headers**:

- `code_level_core.h` - Core level structure
- `code_platform_system.h` - Platform pattern management
- `code_platform_system_ext.h` - Extended platform functions
- `code_player_system.h` - Player positioning
- `code_enemy_system.h` - Enemy management
- `code_enemy_system_validation.h` - Enemy validation
- `code_persistence.h` - Save/load functionality
- `code_gen.h` - Level code generation

**Engine Sources**:

- `code_*.c` - All level code implementation files

**Engine Settings**:

- `LEVEL_CODE_DISPLAY_LENGTH` - Display length for codes

## Dependency Management

### Shared Dependencies

- `tile_utils.h/c` - Will be copied to both plugins for independence
- Core GB Studio APIs - Both plugins use standard GB Studio functions

### Interface Points

- Both plugins will operate on the same tilemap data
- No direct plugin-to-plugin communication required
- Each plugin maintains its own state independently

## Risk Mitigation

### Backup Strategy

- Create complete backup of current working plugin
- Maintain original plugin until migration is complete
- Test each phase independently

### Validation Approach

- Functional testing after each migration phase
- Regression testing against existing projects
- Performance benchmarking

### Rollback Plan

- Keep original plugin structure intact during migration
- Document all changes for easy reversal
- Test rollback procedures

## Success Criteria

1. **Functionality Preservation**: All existing features work identically
2. **Clean Separation**: No cross-plugin dependencies
3. **Independent Operation**: Each plugin works standalone
4. **Documentation Complete**: Updated docs for both plugins
5. **Performance Maintained**: No performance degradation
6. **Backward Compatible**: Existing projects unchanged

## Timeline Estimates

- **Phase 1** (Plugin Structure): 2-3 hours
- **Phase 2** (File Migration): 3-4 hours
- **Phase 3** (Dependency Cleanup): 2-3 hours
- **Phase 4** (Documentation): 1-2 hours
- **Phase 5** (Testing & Validation): 2-3 hours

**Total Estimated Time**: 10-15 hours

## Migration Validation

### Test Cases

1. **Paint Functionality**: All painting operations work correctly
2. **Level Code Generation**: Code generation functions properly
3. **Actor Management**: Paint actors behave as expected
4. **UI Feedback**: Brush preview and validation work
5. **Integration**: Both plugins work together when both installed

### Validation Checklist

- [ ] All existing events compile without errors
- [ ] No missing dependencies in either plugin
- [ ] Engine settings properly distributed
- [ ] Documentation updated and accurate
- [ ] Example projects still work
- [ ] Performance is maintained or improved

## Next Steps

1. Review and approve this migration plan
2. Create backup of current plugin
3. Begin Phase 1: Plugin Structure Setup
4. Execute task list systematically
5. Validate each phase before proceeding

## Notes

- This migration focuses on **separation without breaking changes**
- Missing VM wrapper events will be addressed in a future phase
- The goal is clean, maintainable code with preserved functionality
- Each plugin will be independently versioned and documented
