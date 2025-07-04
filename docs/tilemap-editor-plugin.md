# Tilemap Editor Plugin

## Overview

The Tilemap Editor plugin provides advanced in-game level editing capabilities for GB Studio platformers. It allows users to interactively design levels by placing platforms, enemies, and player positions directly in the game environment, with automatic validation and real-time feedback.

## Key Components

### 1. Event System

The plugin provides several GB Studio events that can be added to your game scenes:

| Event                  | Purpose                                 | Parameters                          |
| ---------------------- | --------------------------------------- | ----------------------------------- |
| **Setup Paint Actors** | Configure actor IDs to use for painting | Player ID, Exit ID, Enemy IDs (1-6) |
| **Enable Editor**      | Activate the editor mode                | None                                |
| **Paint Tile**         | Handle painting at cursor position      | Position X, Position Y              |
| **Get Brush Tile**     | Get preview information at position     | Position X, Position Y              |
| **Cycle Character**    | Change the active tool/brush            | None                                |

### 2. Core Engine Files

The plugin's functionality is implemented through several core engine files:

| File                         | Purpose                                         |
| ---------------------------- | ----------------------------------------------- |
| **paint.h/c**                | Main painting logic and entity management       |
| **tile_utils.h/c**           | Tile-specific utilities and metatile operations |
| **code_level_core.h**        | Core level constants and structure definitions  |
| **code_platform_system.h/c** | Platform-specific validation and manipulation   |
| **code_enemy_system.h/c**    | Enemy placement and validation                  |
| **code_player_system.h/c**   | Player positioning and related constraints      |

### 3. Integration Points

The plugin seamlessly integrates with other systems:

- **Level Code System**: Updates level codes in real-time as tiles are painted
- **Code Entry Mode**: Allows unrestricted drawing during code entry with post-cleanup
- **Metatile System**: Uses standard GB Studio metatile format for visual representation

## Workflow

### Setup and Initialization

```
1. Add "Setup Paint Actors" event to scene start
   - Specify actor IDs to use for player, exit, and enemies
   - This disables all assigned actors for a clean initial state

2. Optionally load existing level with "Load Level Code"

3. Add "Enable Editor" event when player toggles edit mode
   - This prepares the editor state
```

### Editing Process

```
1. Player navigates with d-pad
   - Cursor moves to select position

2. Player uses A/B buttons:
   - A: Paint at current position
   - B: Delete at current position

3. Player uses SELECT:
   - Cycles between different tools (platform, player, enemy)

4. Real-time feedback:
   - Preview shows valid/invalid placement
   - Level code updates with each change
```

### Saving/Loading

```
1. Use "Save Level Code" event to store design to variables

2. Use "Load Level Code" event to restore from variables

3. Level codes can be displayed on screen for manual copying
```

## Technical Implementation

### Selector States

The editor uses multiple brush states to control what will be placed:

```c
#define SELECTOR_STATE_DEFAULT 0
#define SELECTOR_STATE_DELETE 1
#define SELECTOR_STATE_ENEMY_RIGHT 2
#define SELECTOR_STATE_ENEMY_LEFT 3
#define SELECTOR_STATE_NEW_PLATFORM 4
#define SELECTOR_STATE_PLAYER 5
#define SELECTOR_STATE_PLATFORM_LEFT 6
#define SELECTOR_STATE_PLATFORM_RIGHT 7
```

### Actor Management

The plugin maintains a list of actor IDs used for visual representation:

```c
// Global actor IDs for paint operations
UBYTE paint_player_id = 0;
UBYTE paint_exit_id = 1;
UBYTE paint_enemy_ids[5] = {2, 3, 4, 5, 6};
UBYTE paint_enemy_slots_used[5] = {0, 0, 0, 0, 0}; // Track usage
```

### Paint Operation Flow

The main paint function coordinates the placement logic:

```c
void paint(UBYTE x, UBYTE y) {
    // 1. Validate position is within bounds
    // 2. Check if platform can be placed
    // 3. Handle platform extension and merging
    // 4. Update metatiles and visual representation
    // 5. Update level code display
}
```

## Best Practices

### Performance

1. **Limit Enemy Count**: The system supports up to 5 enemies for optimal performance
2. **Zone-Based Updates**: Changes trigger targeted level code updates
3. **Selective Refreshing**: Only affected characters are redrawn

### Validation

1. **Platform Rules**: Always enforced (2-8 tile length)
2. **Enemy Placement**: Must be above platforms
3. **Player Position**: Single player with proper validation

### Integration

1. **Existing Projects**: Can be added to any GB Studio platformer
2. **Custom Features**: Can be extended with additional tile types
3. **Exit Events**: Remember to disable editor when player exits the scene

## Future Enhancements

- Support for additional entity types
- Multi-layer editing capabilities
- Undo/redo functionality
- Custom tile pattern definitions

## API Reference

See the individual system documentation files for detailed API references:

- [Platform Paint System](platform-paint-system.md)
- [Level Code System](level-code-system.md)
- [Code Entry Mode](code-entry-mode.md)
