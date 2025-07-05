# Plugin Migration Task List

## Phase 1: Plugin Structure Setup

**Estimated Time**: 2-3 hours  
**Goal**: Create the basic directory structure for both new plugins

### Task 1.1: Create TilemapPainter Plugin Structure

- [ ] Create `plugins/TilemapPainter/` directory
- [ ] Create `plugins/TilemapPainter/events/` directory
- [ ] Create `plugins/TilemapPainter/engine/` directory
- [ ] Create `plugins/TilemapPainter/engine/include/` directory
- [ ] Create `plugins/TilemapPainter/engine/src/` directory
- [ ] Create `plugins/TilemapPainter/engine/src/core/` directory

### Task 1.2: Create LevelCodeSystem Plugin Structure

- [ ] Create `plugins/LevelCodeSystem/` directory
- [ ] Create `plugins/LevelCodeSystem/events/` directory
- [ ] Create `plugins/LevelCodeSystem/engine/` directory
- [ ] Create `plugins/LevelCodeSystem/engine/include/` directory
- [ ] Create `plugins/LevelCodeSystem/engine/src/` directory
- [ ] Create `plugins/LevelCodeSystem/engine/src/core/` directory

### Task 1.3: Create Basic Plugin Files

- [ ] Create `plugins/TilemapPainter/README.md` (placeholder)
- [ ] Create `plugins/LevelCodeSystem/README.md` (placeholder)

**Validation**: Directory structure matches target architecture

---

## Phase 2: File Migration - TilemapPainter

**Estimated Time**: 2-3 hours  
**Goal**: Move all paint-related files to TilemapPainter plugin

### Task 2.1: Copy Paint-Related Events

- [ ] Copy `eventSetupPaintActors.js` to `TilemapPainter/events/`
- [ ] Copy `eventEnableEditor.js` to `TilemapPainter/events/`
- [ ] Copy `eventPaintTile.js` to `TilemapPainter/events/`
- [ ] Copy `eventGetBrushTile.js` to `TilemapPainter/events/`
- [ ] Copy `eventCycleCharacter.js` to `TilemapPainter/events/`

### Task 2.2: Copy Paint-Related Headers

- [ ] Copy `paint.h` to `TilemapPainter/engine/include/`
- [ ] Copy `paint_core.h` to `TilemapPainter/engine/include/`
- [ ] Copy `paint_platform.h` to `TilemapPainter/engine/include/`
- [ ] Copy `paint_entity.h` to `TilemapPainter/engine/include/`
- [ ] Copy `paint_ui.h` to `TilemapPainter/engine/include/`
- [ ] Copy `paint_vm.h` to `TilemapPainter/engine/include/`
- [ ] Copy `tile_utils.h` to `TilemapPainter/engine/include/`

### Task 2.3: Copy Paint-Related Sources

- [ ] Copy `paint.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_clean.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_core.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_entity.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_platform.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_stub.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_ui.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `paint_vm.c` to `TilemapPainter/engine/src/core/`
- [ ] Copy `tile_utils.c` to `TilemapPainter/engine/src/core/`

### Task 2.4: Create TilemapPainter engine.json

- [ ] Create new `engine.json` with paint-specific settings:
  - `PLATFORM_MAX_LENGTH`
  - `MAX_ENEMIES`
  - `PLATFORM_Y_MIN`
  - `PLATFORM_Y_MAX`

**Validation**: All paint files copied, TilemapPainter compiles independently

---

## Phase 3: File Migration - LevelCodeSystem

**Estimated Time**: 2-3 hours  
**Goal**: Move all level code related files to LevelCodeSystem plugin

### Task 3.1: Copy Level Code Headers

- [ ] Copy `code_level_core.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_platform_system.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_platform_system_ext.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_player_system.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_enemy_system.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_enemy_system_validation.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_persistence.h` to `LevelCodeSystem/engine/include/`
- [ ] Copy `code_gen.h` to `LevelCodeSystem/engine/include/`

### Task 3.2: Copy Level Code Sources

- [ ] Copy `code_level_core.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_platform_system.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_platform_system_ext.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_player_system.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_enemy_system.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_enemy_system_validation.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_persistence.c` to `LevelCodeSystem/engine/src/core/`
- [ ] Copy `code_gen.c` to `LevelCodeSystem/engine/src/core/`

### Task 3.3: Create LevelCodeSystem engine.json

- [ ] Create new `engine.json` with code-specific settings:
  - `LEVEL_CODE_DISPLAY_LENGTH`

**Validation**: All level code files copied, LevelCodeSystem compiles independently

---

## Phase 4: Dependency Cleanup

**Estimated Time**: 2-3 hours  
**Goal**: Remove cross-dependencies and ensure clean separation

### Task 4.1: Clean TilemapPainter Dependencies

- [ ] Review all `#include` statements in TilemapPainter files
- [ ] Remove references to `code_*.h` files
- [ ] Update `paint.h` to only include paint-related headers
- [ ] Ensure no level code function calls in paint files
- [ ] Test compilation of TilemapPainter standalone

### Task 4.2: Clean LevelCodeSystem Dependencies

- [ ] Review all `#include` statements in LevelCodeSystem files
- [ ] Remove references to `paint*.h` files (except shared utilities)
- [ ] Ensure no paint function calls in code files
- [ ] Test compilation of LevelCodeSystem standalone

### Task 4.3: Handle Shared Dependencies

- [ ] Verify `tile_utils.h/c` works in both plugins
- [ ] Check for any other shared code that needs duplication
- [ ] Ensure no shared mutable state between plugins

### Task 4.4: Update Event Groups

- [ ] Update TilemapPainter events to use group `["TilemapPainter"]`
- [ ] Keep LevelCodeSystem events separate (when created later)

**Validation**: Both plugins compile independently, no cross-dependencies

---

## Phase 5: Documentation Update

**Estimated Time**: 1-2 hours  
**Goal**: Update all documentation to reflect the new plugin structure

### Task 5.1: Create TilemapPainter README

- [ ] Write comprehensive README for TilemapPainter plugin
- [ ] Document all events and their usage
- [ ] Include setup instructions
- [ ] Add examples and workflow

### Task 5.2: Create LevelCodeSystem README

- [ ] Write comprehensive README for LevelCodeSystem plugin
- [ ] Document engine configuration
- [ ] Include technical details about level code format
- [ ] Add usage examples (for future events)

### Task 5.3: Update Main Plugin README

- [ ] Update `plugins/README.md` to reference both new plugins
- [ ] Add migration notes for existing users
- [ ] Include compatibility information

### Task 5.4: Update Documentation References

- [ ] Review all files in `docs/` directory
- [ ] Update references to point to correct plugins
- [ ] Add new documentation sections as needed

**Validation**: All documentation is accurate and complete

---

## Phase 6: Testing & Validation

**Estimated Time**: 2-3 hours  
**Goal**: Comprehensive testing to ensure no functionality is broken

### Task 6.1: Functional Testing

- [ ] Test all TilemapPainter events work correctly
- [ ] Test paint actor setup and management
- [ ] Test brush cycling and feedback
- [ ] Test tile painting and validation
- [ ] Test editor mode activation

### Task 6.2: Integration Testing

- [ ] Test with existing project files
- [ ] Verify both plugins can be used together
- [ ] Test that removing one plugin doesn't break the other
- [ ] Validate engine settings work correctly

### Task 6.3: Performance Testing

- [ ] Compare performance before/after migration
- [ ] Check memory usage of both plugins
- [ ] Verify no performance regression

### Task 6.4: Regression Testing

- [ ] Test against known working levels
- [ ] Verify all existing functionality preserved
- [ ] Check for any unexpected behavior changes

**Validation**: All tests pass, no functionality regression

---

## Phase 7: Final Cleanup & Archival

**Estimated Time**: 1 hour  
**Goal**: Clean up old files and prepare for production

### Task 7.1: Archive Original Plugin

- [ ] Rename `TilemapEditor` to `TilemapEditor_BACKUP`
- [ ] Add timestamp to backup directory name
- [ ] Create archive notes explaining the backup

### Task 7.2: Update Project References

- [ ] Update any project files that reference the old plugin
- [ ] Check for hardcoded paths that need updating
- [ ] Update build scripts if necessary

### Task 7.3: Final Validation

- [ ] Full project build test
- [ ] Documentation review
- [ ] Code quality check
- [ ] Performance final check

**Validation**: Clean, working, separated plugins ready for production

---

## Emergency Rollback Procedure

If any issues are discovered during migration:

1. **Stop Current Phase**: Do not proceed further
2. **Document Issue**: Record what went wrong and at what step
3. **Restore Backup**: Rename backup directory back to original
4. **Verify Restore**: Test that original functionality works
5. **Analyze Problem**: Determine root cause before retry
6. **Update Plan**: Modify migration plan to address issue

## Success Metrics

- [ ] Both plugins compile without errors
- [ ] All original functionality preserved
- [ ] No performance degradation
- [ ] Clean separation with no cross-dependencies
- [ ] Documentation is complete and accurate
- [ ] Migration can be completed within estimated time

## Notes

- Each phase should be completed fully before moving to the next
- Test thoroughly after each phase
- Document any issues or deviations from the plan
- Keep backups at each major phase completion
- Don't rush - clean separation is more important than speed
