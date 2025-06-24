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

1. Call this event at scene start to configure actor assignments
2. Automatically disables all assigned actors to provide a clean state
3. Ready for "Load Level Code" event to populate the level
4. When painting, the specified actors will be moved to painted positions
5. When deleting, the corresponding actors will be deactivated

### Setup Map Actors

Sets up actors based on tiles already placed in the tilemap.

**Note:** The new "Setup Paint Actors" event is recommended for real-time painting workflows, while "Setup Map Actors" is better for loading pre-designed levels.

## Recommended Workflow

For best results when using the tilemap editor, follow this sequence:

1. **Initialize System**: Call "Setup Paint Actors" with your desired actor IDs (at scene start)
   - This configures which actors to use AND disables them for clean initialization
2. **Load Level** (Optional): Call "Load Level Code" to populate a saved level
3. **Enter Editor Mode**: Call "Enable Editor" when toggling into editor mode
   - This clears only the assigned actors (redundant after setup, but safe for re-entry)
4. **Start Painting**: Use the paint tool - actors will move to painted positions in real-time
5. **Exit Editor**: Optionally call "Setup Map Actors" to finalize actor positions

The "Setup Paint Actors" event now provides complete initialization, automatically disabling assigned actors so the level starts clean and ready for either manual painting or level code loading.
