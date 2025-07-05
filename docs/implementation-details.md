# Tilemap Editor Implementation Guide

This guide provides detailed information for developers who want to extend or modify the Tilemap Editor plugin.

## Plugin Evolution

The Tilemap Editor evolved from earlier versions through several iterations:

1. **Initial Version**: Basic platform painting functionality
2. **MetaTile8Plugin**: Added level encoding and basic validation
3. **Current TilemapEditor**: Complete rewrite with optimized architecture and performance

The migration to the current TilemapEditor structure improved organization, performance, and extensibility while maintaining core functionality.

## Architecture Overview

The Tilemap Editor plugin follows a modular architecture with several interconnected components:

```
TilemapEditor/
│
├── events/                    # GB Studio event definitions
│   ├── eventSetupPaintActors.js
│   ├── eventEnableEditor.js
│   ├── eventPaintTile.js
│   ├── eventGetBrushTile.js
│   └── eventCycleCharacter.js
│
└── engine/                    # C implementation files
    ├── include/               # Header files
    │   ├── paint.h                    # Main paint system umbrella header
    │   ├── paint_core.h              # Core paint functionality
    │   ├── paint_platform.h          # Platform-specific operations
    │   ├── paint_entity.h            # Entity management (player/enemies)
    │   ├── paint_ui.h                # UI state and brush management
    │   ├── paint_vm.h                # VM wrapper functions
    │   ├── tile_utils.h
    │   ├── code_level_core.h
    │   ├── code_platform_system.h
    │   ├── code_enemy_system.h
    │   └── code_player_system.h
    │
    └── src/core/              # Implementation files
        ├── paint.c                   # Paint system compatibility stub
        ├── paint_core.c              # Core paint logic and main paint() function
        ├── paint_platform.c          # Platform validation and reconstruction
        ├── paint_entity.c            # Player/enemy management and positioning
        ├── paint_ui.c                # Brush state and UI feedback
        ├── paint_vm.c                # VM wrapper functions
        ├── tile_utils.c
        ├── code_platform_system.c
        ├── code_enemy_system.c
        └── code_player_system.c
```

## Key Module Responsibilities

### Paint System (Modular Architecture)

**Refactored from monolithic paint.h/c into specialized modules:**

#### Core Module (`paint_core.h/c`)

Core responsibility: Main paint coordination and utility functions.

```c
void paint(UBYTE x, UBYTE y);                  // Main painting function
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y);
UBYTE get_current_tile_type(UBYTE x, UBYTE y);
void update_level_code_for_paint(UBYTE x, UBYTE y);
```

#### Platform Module (`paint_platform.h/c`)

Core responsibility: Platform validation, placement, and reconstruction.

```c
UBYTE can_place_platform(UBYTE x, UBYTE y);
UBYTE count_connected_platform_length(UBYTE x, UBYTE y);
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y);
void rebuild_platform_row(UBYTE y);
```

#### Entity Module (`paint_entity.h/c`)

Core responsibility: Player and enemy management, positioning, and FIFO pool.

```c
void paint_player(UBYTE x, UBYTE y);           // Player placement
void paint_enemy_right/left(UBYTE x, UBYTE y); // Enemy placement
void move_actor_to_tile(UBYTE actor_id, UBYTE x, UBYTE y);
UBYTE get_next_enemy_slot_from_pool(void);
void reset_enemy_pool(void);
```

#### UI Module (`paint_ui.h/c`)

Core responsibility: Brush state management and UI feedback.

```c
UBYTE get_brush_tile_state(UBYTE x, UBYTE y);
UBYTE get_platform_placement_type(UBYTE x, UBYTE y);
```

#### VM Module (`paint_vm.h/c`)

Core responsibility: Script interface and VM wrapper functions.

```c
void vm_setup_paint_actors(SCRIPT_CTX *THIS);
void vm_paint(SCRIPT_CTX *THIS);
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS);
void vm_enable_editor(SCRIPT_CTX *THIS);
```

#### Compatibility Layer (`paint.h`)

The main paint.h header now serves as an umbrella that includes all specialized modules, maintaining backward compatibility while providing the benefits of modular organization.

### Platform System (`code_platform_system.h/c`)

Core responsibility: Handle platform-specific validation and management.

```c
// Key functions
UBYTE count_connected_platform_length(UBYTE x, UBYTE y);
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y);
void rebuild_platform_row(UBYTE y);
```

### Tile Utilities (`tile_utils.h/c`)

Core responsibility: Provide low-level metatile manipulation.

```c
// Key functions
void replace_meta_tile(UBYTE x, UBYTE y, UBYTE tile_type, UBYTE force);
UBYTE get_meta_tile(UBYTE x, UBYTE y);
UBYTE get_tile_type(UBYTE tile_id);
```

### Level Code System (`code_gen.h/c`, `code_level_core.h/c`)

Core responsibility: Handle level serialization and deserialization.

```c
// Key functions
void update_level_code_for_paint(UBYTE x, UBYTE y);
void update_complete_level_code(void);
void extract_platform_data(void);
```

### Player and Enemy Systems (`code_player_system.h/c`, `code_enemy_system.h/c`)

Core responsibility: Manage entity placement rules and validation.

```c
// Key functions (now integrated with modular paint system)
UBYTE can_paint_player(UBYTE x, UBYTE y);          // In paint_entity.h/c
UBYTE can_paint_enemy_right/left(UBYTE x, UBYTE y); // In paint_entity.h/c
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y); // In paint_entity.h/c (legacy - no longer auto-used)
```

**Note**: Entity validation functions have been moved to the paint_entity module to consolidate all entity-related logic in one place.

## Data Structures

### Level Code Structure

```c
typedef struct {
    // Platform patterns (5 bits each)
    UBYTE patterns[16];  // 16 segments, one byte each (only 5 bits used)

    // Entity positions
    UBYTE player_x;
    UBYTE player_y;
    UBYTE enemy_count;
    UBYTE enemy_positions[10]; // x,y pairs for up to 5 enemies
    UBYTE enemy_types[5];      // Direction for each enemy
} level_code_t;
```

### Tile Type Constants

```c
// Tile type constants
#define TILE_EMPTY 0
#define TILE_PLATFORM_LEFT 1
#define TILE_PLATFORM_MIDDLE 2
#define TILE_PLATFORM_RIGHT 3
#define TILE_PLAYER 4
#define TILE_ENEMY_RIGHT 5
#define TILE_ENEMY_LEFT 6
```

### Brush State Constants

```c
// Brush preview state constants
#define BRUSH_TILE_EMPTY 0
#define BRUSH_TILE_PLATFORM 1
#define BRUSH_TILE_PLAYER 2
#define BRUSH_TILE_ENEMY_RIGHT 3
#define BRUSH_TILE_ENEMY_LEFT 4
```

## Extending the Plugin

## Extending the Plugin

### Adding New Tile Types

1. Define new tile type constants in `tile_utils.h`
2. Add corresponding brush states in the appropriate paint module header:
   - **Entities (players, enemies, items)**: `paint_entity.h`
   - **Platforms/terrain**: `paint_platform.h`
   - **UI elements**: `paint_ui.h`
3. Implement validation logic in the corresponding module:
   - **Entity validation**: `paint_entity.c`
   - **Platform validation**: `paint_platform.c`
   - **UI state logic**: `paint_ui.c`
4. Update level code serialization in `code_gen.c`
5. Add VM wrapper functions in `paint_vm.c` if needed for script access

Example for adding a collectible item:

```c
// In tile_utils.h
#define TILE_COLLECTIBLE 7

// In paint.h
#define BRUSH_TILE_COLLECTIBLE 5

// In paint.c
void paint_collectible(UBYTE x, UBYTE y) {
    // Implementation
}

UBYTE can_paint_collectible(UBYTE x, UBYTE y) {
    // Validation rules
    return TRUE;
}

// In VM wrapper
void vm_paint_collectible(SCRIPT_CTX *THIS) {
    // VM binding
}
```

### Adding New Events

1. Create a new `.js` event file in the `events` directory
2. Implement corresponding C function in appropriate file
3. Add VM wrapper function

Example for adding a "Clear Level" event:

```javascript
// In eventClearLevel.js
const id = "EVENT_CLEAR_LEVEL";
const name = "Clear Level";
// Rest of event definition

// In paint.c
void clear_level(void) {
    // Implementation
}

// VM wrapper function
void vm_clear_level(SCRIPT_CTX *THIS) {
    clear_level();
}
```

### Modifying Validation Rules

Platform validation rules are primarily in these functions:

```c
// In code_platform_system.c
UBYTE count_connected_platform_length(UBYTE x, UBYTE y);
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y);
UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y);

// Constants that can be modified
#define PLATFORM_MAX_LENGTH 8        // Maximum platform length
#define PLATFORM_MIN_VERTICAL_GAP 1  // Minimum gap between platforms
```

## Performance Considerations

### Critical Paths

These functions are called frequently and are critical for performance:

1. `get_brush_tile_state()` - Called on every cursor movement
2. `paint()` - Core painting function
3. `update_level_code_for_paint()` - Called after every paint operation

### Optimization Techniques Used

1. **Cached Tile Access**: `get_current_tile_type()` reduces repeated lookups
2. **Boundary Check Inlining**: `is_within_platform_bounds()` optimizes frequent checks
3. **Zone-Based Updates**: Only update affected level code characters
4. **Dirty Region Tracking**: Smart updates for changed tiles only

### Memory Usage

- Level code structure: ~32 bytes
- Tile map: 32×32 bytes (1KB)
- Actor state: ~10 bytes per actor (7 actors = 70 bytes)

## Testing & Debugging

### Debug Features

Enable debug mode with:

```c
#define DEBUG_MODE 1
```

Debug functions available:

```c
void print_debug_info(void);         // Output current state
void validate_level_integrity(void); // Run comprehensive checks
```

### Common Issues & Solutions

1. **Platform Connection Issues**:

   - Check `rebuild_platform_row()` implementation
   - Verify tile replacement calls use proper force parameter

2. **Level Code Discrepancies**:

   - Ensure `extract_chunk_pattern()` correctly reads platform data
   - Check for boundary issues in `update_level_code_for_paint()`

3. **Actor Positioning Errors**:
   - Verify coordinates in `move_actor_to_tile()`
   - Check for invalid positions in painter functions

## Future Development

Areas identified for future enhancement:

1. **Enhanced Brush System**: Support for multi-tile brushes and patterns
2. **Undo/Redo Support**: Command pattern implementation for operation history
3. **Optimized Rendering**: Dirty rectangle tracking for performance
4. **Additional Entity Types**: Framework for custom game entities
5. **Multi-Layer Support**: Extended system for background/foreground layers
