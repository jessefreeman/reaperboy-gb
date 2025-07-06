# Level Code System

## Overview

The level code system provides a compact, optimized way to serialize and deserialize player and enemy positions for GB Studio platformer levels. The system uses bit-packing to minimize storage requirements while maintaining full fidelity of game state.

## Encoding Specification

### Platform Pattern Storage

- **Pattern Count**: 21 platform patterns (UIDs 0-20)
- **Pattern Format**: 10-bit patterns where top row = bits 9-5, bottom row = bits 4-0
- **Segment Size**: 5 tiles wide × 2 tiles high
- **Position Mapping**: Position 0 = leftmost (bit 4), Position 4 = rightmost (bit 0)
- **Total Blocks**: 16 blocks in a 4×4 grid

### Variable-Based Persistence

The system stores level data across 6 global variables for persistence between sessions:

```c
#define VAR_LEVEL_CODE_PART_1 0 // Platform patterns 0-2   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_2 1 // Platform patterns 3-5   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_3 2 // Platform patterns 6-8   (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_4 3 // Platform patterns 9-11  (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_5 4 // Platform patterns 12-14 (3×5 bits = 15 bits)
#define VAR_LEVEL_CODE_PART_6 5 // Platform pattern 15 (1×5 bits = 5 bits)
```

### Display System

The level code is displayed as a 24-character string:

- **Characters 0-15**: Platform patterns (hex encoded)
- **Characters 16-19**: Enemy summary data
- **Characters 20-23**: Player and detailed enemy data

## Core Functions

### Pattern Management

```c
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id);
void extract_platform_data(void);
UBYTE extract_chunk_pattern(UBYTE segment_x, UBYTE segment_y);
```

### Level Code Display

```c
void display_complete_level_code(void);
void update_complete_level_code(void);
void force_complete_level_code_display(void);
```

### Persistence Functions

```c
void save_level_code_to_variables(void);
void load_level_code_from_variables(void);
UBYTE has_saved_level_code(void);
void clear_saved_level_code(void);
```

### VM Wrapper Functions

```c
void vm_save_level_code(SCRIPT_CTX *THIS);
void vm_load_level_code(SCRIPT_CTX *THIS);
void vm_has_saved_level_code(SCRIPT_CTX *THIS);
```

## Platform Rules

1. **Minimum Length**: 2 tiles (no single-tile platforms allowed)
2. **Maximum Length**: 8 tiles (platform cannot exceed 8 connected tiles)
3. **Connectivity**: Platforms connect horizontally across segment boundaries
4. **Auto-completion**: Single platforms at edges get completed to 2-tile minimum

## Usage Examples

### Saving a Level Code

```c
save_level_code_to_variables();
// Level data is now stored in global variables for persistence
```

### Loading a Level Code

```c
if (has_saved_level_code()) {
    load_level_code_from_variables();
    force_complete_level_code_display();
}
```

### In GB Studio Scripts

1. **Save Level Code** event - Saves current level design to variables
2. **Load Level Code** event - Loads previously saved level design
3. **Check Saved Level Code** event - Returns 1 if saved data exists

## Technical Implementation

### Flicker Prevention System

The level code display uses a selective update system to prevent flicker:

- **Bitmask tracking**: `display_update_mask` tracks which characters need updating
- **Cached encoding**: Avoids redundant calculations
- **Smart change detection**: Only updates characters that actually changed
- **Zone-specific updates**: Platform changes only update the affected zone

### Pattern Application Process

1. **Manual Paint Simulation**: Uses actual `paint()` function calls
2. **Auto-completion**: Single platforms automatically extend to valid lengths
3. **Platform Merging**: Adjacent platforms merge using identical rules
4. **Level Code Sync**: Immediate updates for all affected blocks

## Player Position System - Final Implementation

The player position system now handles both editor and gameplay modes correctly:

### Editor Mode (Visual Marker)
- Player marker tile is placed at row 11 (the bottom row) at the correct column
- This provides visual feedback in the editor showing where the player will start
- The marker is restored when loading a level code for editing

### Gameplay Mode (Actor Movement)
- Player actor is moved to row 0 (the top row) at the correct column
- This positions the player at the starting position for gameplay
- The actor movement is handled by the `move_player_to_column` function

### Code Structure
- `update_player_actor_position()` handles both modes:
  - Places marker tile at row 11 for editor visualization
  - Moves player actor to row 0 for gameplay
- `move_player_to_column()` uses the paint system's `move_player_actor_to_tile()` function
- Both functions work together to ensure seamless transitions between modes

### Level Code Format
The player position is stored as a single column value (0-19) in the level code:
- Column 0 = leftmost playable position
- Column 19 = rightmost playable position  
- The system automatically converts this to the appropriate tile coordinates (adds 2 for offset)

This ensures that both the editor shows the correct player position and gameplay starts the player at the right location.

## Setup Requirements

### GB Studio Variables

Create these global variables in your GB Studio project and update the variable IDs in `code_gen.c`:

1. `level_code_part_1` through `level_code_part_6`
2. Update `#define VAR_LEVEL_CODE_PART_X` values to match your variable indices

### Event Usage Flow

```
Level Editor Entry:
1. Check Saved Level Code → $has_saved_data
2. If data exists: Load Level Code
3. Display Complete Level Code

Level Editor Exit:
1. Save Level Code
2. Show confirmation message
```

## Tilemap Editor Initialization

### Function: `init_tilemap_editor_from_memory()`

This function handles the complete initialization of the tilemap editor from memory, ensuring all elements are properly positioned:

**Purpose**: 
- Restores the complete level state when the tilemap editor loads
- Ensures player marker, player actor, and exit sprite are correctly positioned
- Synchronizes the level code display with the current state

**Actions Performed**:
1. Calls `restore_level_from_memory()` to rebuild the level
2. Extracts current player data to ensure synchronization
3. Updates player actor position (marker tile + actor movement)
4. Forces a complete display update

**GB Studio Usage**:
```
Event: "Initialize Tilemap Editor"
└── Call Native: vm_init_tilemap_editor_from_memory
```

**When to Use**:
- Scene "On Init" script for tilemap editor scenes
- After loading a level code from external source
- When switching between editor and gameplay modes
- To refresh the editor state after any major changes

This ensures that both the visual editor (with marker tiles) and the gameplay system (with positioned actors) are properly initialized when the tilemap editor loads.
