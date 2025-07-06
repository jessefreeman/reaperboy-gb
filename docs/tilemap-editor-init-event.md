# GB Studio Event: Initialize Tilemap Editor

This describes how to properly initialize the tilemap editor to restore player position and level data from memory.

## Primary Method: Enable Editor Event

The **"Enable Editor"** event now automatically handles tilemap editor initialization including player positioning.

### Event Structure

```
Event: "Enable Editor" (from TilemapEditor group)
```

### Event Details

**Purpose**: 
- Deactivates player, exit, and enemy actors
- Initializes the paint editor system
- Restores level state from existing tilemap OR from memory
- **NEW**: Automatically positions player marker and actor correctly

**Actions performed**:
1. Deactivates actors that will be controlled by the editor
2. Initializes enemy and player tracking systems
3. Checks if map is empty and initializes default level if needed
4. **If map has content**: Updates level code from tilemap AND calls `update_player_actor_position()`
5. Forces complete level code display update

### Usage

**Scene On Init** (Recommended):
```
Scene: "Tilemap Editor"
├── On Init:
│   └── Enable Editor
└── Actors:
    ├── Player Actor (will be positioned automatically)
    └── Exit Actor (will be positioned automatically)
```

## Alternative Method: Initialize Tilemap Editor from Memory Event

For cases where you specifically want to restore from memory state (not current tilemap):

```
Event: "Initialize Tilemap Editor from Memory"
└── Call Native: vm_init_tilemap_editor_from_memory
```

## Key Differences

- **"Enable Editor"**: Initializes from current tilemap state, with proper player positioning
- **"Initialize from Memory"**: Forces restoration from C memory state, ignoring current tilemap

## Technical Notes

- The "Enable Editor" event now includes automatic player positioning
- Player marker tile is placed at row 11 for editor visualization
- Player actor is moved to row 0 for gameplay positioning  
- Exit sprite is positioned relative to the player
- All positioning happens automatically when the editor loads

## Integration

This works seamlessly with:
- Level code restoration system
- Platform painting with proper end caps
- Enemy positioning system
- All existing tilemap editor functionality

**The tilemap editor now properly initializes with correct player positioning when using the standard "Enable Editor" event.**
