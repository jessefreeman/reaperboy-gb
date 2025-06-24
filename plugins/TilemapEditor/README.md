# Tilemap Editor Plugin

This plugin provides enhanced tilemap editing capabilities for GB Studio.

## Events

### Enable Editor

Use this event when entering the tilemap editor to prepare the editor state.

**Parameters:** None

**Usage:**

- Call this event when entering the tilemap editor mode
- Deactivates only the actors assigned to the tilemap editor (player, exit, and 6 enemies)
- Resets paint actor slot tracking
- Leaves all other scene actors untouched
- Must be called after "Setup Paint Actors" so it knows which actors to manage

### Setup Paint Actors

Use this event to configure which actors should be moved when painting entities in the tilemap editor.

**Parameters:**

- Player Actor: The actor ID to use for the player
- Exit Actor: The actor ID to use for the exit
- Enemy 1-6: The actor IDs to use for enemies

**Usage:**

1. Call this event before using the paint tool
2. When painting a player, the specified player actor will be moved to that position
3. When painting enemies, available enemy actors will be moved to those positions
4. When deleting enemies, the corresponding actors will be deactivated

### Setup Map Actors

Sets up actors based on tiles already placed in the tilemap.

**Note:** The new "Setup Paint Actors" event is recommended for real-time painting workflows, while "Setup Map Actors" is better for loading pre-designed levels.

## Recommended Workflow

For best results when using the tilemap editor, follow this sequence:

1. **Setup Actors**: Call "Setup Paint Actors" with your desired actor IDs (do this once at scene start)
2. **Enter Editor Mode**: Call "Enable Editor" when toggling into editor mode - this clears the scene
3. **Start Painting**: Use the paint tool - actors will move to painted positions in real-time
4. **Exit Editor**: Optionally call "Setup Map Actors" to finalize actor positions based on the final tilemap

This workflow separates the one-time actor configuration from the editor state management, giving you clean control over when actors are cleared and when they're configured.
