# Tilemap Editor Integration Guide

## Quick Start Guide

This guide provides step-by-step instructions for integrating the Tilemap Editor plugin into your GB Studio project.

## Installation

1. Copy the `TilemapEditor` folder from the `plugins` directory to your project's `plugins` directory
2. Restart GB Studio to load the plugin
3. Create necessary GB Studio variables (see Variable Setup below)

## Variable Setup

The Tilemap Editor requires 6 global variables for level code persistence:

1. Create these variables in your GB Studio project:

   - `level_code_part_1` through `level_code_part_6`

2. Update the variable IDs in `code_gen.c` to match your project:

```c
// Update these values to match your variable IDs
#define VAR_LEVEL_CODE_PART_1 0
#define VAR_LEVEL_CODE_PART_2 1
#define VAR_LEVEL_CODE_PART_3 2
#define VAR_LEVEL_CODE_PART_4 3
#define VAR_LEVEL_CODE_PART_5 4
#define VAR_LEVEL_CODE_PART_6 5
```

## Scene Setup

### Required Actors

Prepare your scene with these actors:

1. **Player Character**: Will be positioned based on painting
2. **Exit Point**: Automatically positioned based on player
3. **Enemies (1-5)**: Used for enemy placement

### Event Script Setup

#### Initialization

Add these events to your scene start script:

```
Setup Paint Actors
├── Player Actor: [select your player actor]
├── Exit Actor: [select your exit actor]
└── Enemy Actors: [select your enemy actors]

If Variable(has_saved_level) == 1
└── Load Level Code

Setup UI Elements
```

#### Editor Toggle

Create a script that activates when the player presses a button:

```
If Editor Active == FALSE
├── Enable Editor
└── Set Editor Active = TRUE
Else
├── Save Level Code
└── Set Editor Active = FALSE
```

#### Paint Controls

Create scripts for the different painting controls:

```
When A Button Pressed:
└── Paint Tile
    ├── Position X: $CURSOR_X
    └── Position Y: $CURSOR_Y

When B Button Pressed:
└── Delete Tile
    ├── Position X: $CURSOR_X
    └── Position Y: $CURSOR_Y

When SELECT Button Pressed:
└── Cycle Character
```

## Cursor Management

### Basic Cursor Setup

1. Create a cursor actor with a visible sprite
2. Add movement controls for d-pad navigation
3. Track cursor position in variables ($CURSOR_X, $CURSOR_Y)

### Adding Preview Feedback

For visual feedback on valid/invalid positions:

```
On Cursor Move:
└── Get Brush Tile
    ├── Position X: $CURSOR_X
    └── Position Y: $CURSOR_Y

If Result == VALID:
└── Set Cursor Sprite: green_cursor
Else If Result == INVALID:
└── Set Cursor Sprite: red_cursor
Else If Result == EXTEND:
└── Set Cursor Sprite: yellow_cursor
```

## Level Code Display

### Setting Up Level Code Display

1. Create text objects for displaying the level code (24 characters)
2. Position them in a visible but non-intrusive location
3. They will automatically update when painting

### Copying Level Codes

Add an option for players to copy the code:

```
When START Button Pressed:
└── Display Message: "Level Code: [insert level code variable values]"
```

## Advanced Integration

### Custom Tile Types

To add custom tile types:

1. Update `tile_utils.c` with your new tile definitions
2. Add appropriate validation in `paint.c`
3. Create corresponding visual assets

### Multiple Level Support

For games with multiple levels:

1. Create separate variable sets for each level
2. Add level selection UI
3. Use the appropriate variables based on selected level

## Troubleshooting

### Common Issues

1. **Actors not appearing**: Verify actor IDs match those in "Setup Paint Actors"
2. **Invalid platform behavior**: Check for conflicts with other metatile modifications
3. **Level code not saving**: Verify variable IDs match your GB Studio project

### Debug Mode

Add a debug mode for development:

```
When START+SELECT pressed:
└── Toggle Debug Mode
    ├── If ON: Show grid, display coordinates, show platform lengths
    └── If OFF: Hide debug information
```

## Performance Tips

1. Limit total enemies to 5 for optimal performance
2. Consider disabling level code display updates during rapid painting
3. Use zone-based updates for larger levels

## Next Steps

- Review [Tilemap Editor Overview](tilemap-editor-plugin.md) for detailed architecture
- See [Platform Paint System](platform-paint-system.md) for implementation details
- Check [Level Code System](level-code-system.md) for serialization information
