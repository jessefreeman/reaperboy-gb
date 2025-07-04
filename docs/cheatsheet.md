# Tilemap Editor Cheatsheet

## Quick Reference

This document provides a quick reference for using the Tilemap Editor plugin.

## Event Reference

| Event                  | Purpose                       | Required Parameters           |
| ---------------------- | ----------------------------- | ----------------------------- |
| **Setup Paint Actors** | Initialize actor assignments  | Player ID, Exit ID, Enemy IDs |
| **Enable Editor**      | Activate editor mode          | None                          |
| **Paint Tile**         | Paint at cursor position      | Position X, Position Y        |
| **Get Brush Tile**     | Get brush preview information | Position X, Position Y        |
| **Cycle Character**    | Change active tool/brush      | None                          |
| **Save Level Code**    | Store level to variables      | None                          |
| **Load Level Code**    | Restore level from variables  | None                          |
| **Check Saved Level**  | Check if saved level exists   | None (returns boolean result) |

## Controls Setup

### Basic Control Flow

```
When D-Pad Pressed:
└── Move Cursor
    └── Get Brush Tile (for preview)

When A Button Pressed:
└── Paint Tile at cursor position

When B Button Pressed:
└── Delete tile at cursor position

When SELECT Pressed:
└── Cycle Character (change brush)

When START Pressed:
└── Save Level Code
```

## Tool States

The editor cycles through these brush states:

1. **Platform**: Place platform tiles
2. **Player**: Place the player character
3. **Enemy Right**: Place right-facing enemy
4. **Enemy Left**: Place left-facing enemy
5. **Delete**: Remove tiles/entities

## Level Code Format

The 24-character level code consists of:

```
PPPPPPPPPPPPPPPPEEEEXXXX
│              │    └─┬─┘
│              │      └── Player & enemy details
│              └────────── Enemy summary data
└───────────────────────── Platform patterns (hex)
```

## Platform Rules

- **Minimum Length**: 2 tiles (single-tile platforms removed)
- **Maximum Length**: 8 tiles (prevents oversized platforms)
- **Valid Platform Rows**: Y positions 13, 15, 17, 19
- **Auto-completion**: Edge platforms extend automatically
- **Merging**: Adjacent platforms merge when total ≤ 8 tiles

## Enemy Rules

- **Maximum Count**: 5 enemies per level
- **Placement**: Must be above platform tiles
- **FIFO Replacement**: When exceeding 5 enemies, oldest is replaced
- **No Stacking**: Cannot place multiple enemies at same position

## Player Rules

- **Single Player**: Only one player allowed per level
- **Valid Placement**: Must be above platform
- **Exit Position**: Exit actor automatically positioned based on player

## Common Issues

1. **Actors not appearing**

   - Verify actor IDs match in Setup Paint Actors event
   - Check that Enable Editor event was called

2. **Platform not connecting**

   - Ensure platforms are on same row
   - Check if combined length would exceed 8 tiles

3. **Level code not saving**
   - Verify variable IDs in code_gen.c match your project
   - Check that save event is being called

## Best Practices

1. **Initialize Properly**:

   - Always call Setup Paint Actors at scene start
   - Call Enable Editor when toggling editor mode

2. **User Feedback**:

   - Use Get Brush Tile to show valid/invalid positions
   - Display level code for sharing/backup

3. **Save Frequently**:

   - Implement auto-save or regular save prompts
   - Consider creating backup codes for complex levels

4. **Exit Cleanly**:
   - Save level code before exiting editor mode
   - Disable editor when leaving the scene

## See Also

For more detailed information, consult:

- [Tilemap Editor Overview](tilemap-editor-plugin.md)
- [Integration Guide](integration-guide.md)
- [Platform Paint System](platform-paint-system.md)
- [Level Code System](level-code-system.md)
