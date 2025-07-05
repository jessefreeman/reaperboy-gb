# Tilemap Editor Plugin

This plugin provides enhanced tilemap editing capabilities for GB Studio, allowing in-game level design with real-time feedback and validation.

## Core Features

- **Interactive Level Editor**: Design levels directly in the game environment
- **Platform Validation**: Automatic enforcement of platform rules (2-8 tile length)
- **Entity Placement**: Position player and enemies with automatic validation
- **Level Code System**: Compact encoding for level serialization and sharing
- **Save/Load**: Persistence through GB Studio variables
- **Code Entry Mode**: Support for direct pattern application

## Events

### Setup Paint Actors

Use this event to configure which actors should be moved when painting entities in the tilemap editor.

**Parameters:**

- Player Actor: The actor ID to use for the player
- Exit Actor: The actor ID to use for the exit
- Enemy 1-6: The actor IDs to use for enemies

**Usage:**

1. Call this event at scene start to configure actor assignments
2. Automatically disables all assigned actors to provide a clean initial state
3. Ready for "Load Level Code" event to populate the level
4. When painting, the specified actors will be moved to painted positions

### Enable Editor

Use this event when entering the tilemap editor to prepare the editor state.

**Parameters:** None

**Usage:**

- Call this event when entering the tilemap editor mode
- Deactivates only the actors assigned to the tilemap editor
- Resets paint actor slot tracking
- Must be called after "Setup Paint Actors"

### Paint Tile

Handles painting at the cursor position based on the current brush state.

**Parameters:**

- Position X: X-coordinate for painting
- Position Y: Y-coordinate for painting

**Usage:**

- Connect to player input (typically A button)
- Automatically validates placement and updates level code

### Get Brush Tile

Gets information about what would happen if painting at the current position.

**Parameters:**

- Position X: X-coordinate to check
- Position Y: Y-coordinate to check

**Usage:**

- Connect to cursor movement to provide real-time feedback
- Returns preview state that can be used to show valid/invalid placement

### Cycle Character

Changes the active brush tool between platform, player, and enemy placement modes.

**Parameters:** None

**Usage:**

- Connect to player input (typically SELECT button)
- Cycles through available tools

## Recommended Workflow

For best results when using the tilemap editor, follow this sequence:

1. **Initialize System**: Call "Setup Paint Actors" with your desired actor IDs (at scene start)
2. **Load Level** (Optional): Call "Load Level Code" to populate a saved level
3. **Enter Editor Mode**: Call "Enable Editor" when toggling into editor mode
4. **Start Painting**: Use the paint events connected to player input
5. **Save Level**: Use "Save Level Code" to persist the design

## Documentation

Complete documentation is available in the docs folder:

- [Tilemap Editor Plugin Overview](../docs/tilemap-editor-plugin.md)
- [Platform Paint System](../docs/platform-paint-system.md)
- [Level Code System](../docs/level-code-system.md)
- [Code Entry Mode](../docs/code-entry-mode.md)
