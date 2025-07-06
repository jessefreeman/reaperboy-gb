# Level Code System - Memory-Based Persistence

## Overview

The level code system stores your level in **C memory** automatically as you edit. When you restart the scene, just call "Restore Level from Memory" to reload whatever you were working on.

## How It Works

1. **Edit your level** - Changes are automatically stored in C memory
2. **Scene restarts** - Your level data persists in memory
3. **Call restore event** - Rebuilds the level from memory
4. **Continue editing** - Pick up where you left off

No variables needed! Pure C memory persistence.

## Available Events

### 1. **Restore Level from Memory**
- Rebuilds level tilemap from current C memory using paint system
- Use on scene init to restore after editing
- No parameters needed
- **Features**: Platforms restored with correct end caps and visual styling
- **Note**: Only restores platforms/level layout, not level code display

### 2. **Load Level Code Into Memory**  
- Loads a specific level code into C memory
- Takes level code as comma-separated values
- Format: 24 numbers (0-40 range)
- Example: `1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,10,0,0,0,0,0,0,0`

### 3. **Load Predefined Level**
- Loads pre-built levels by index
- Stores in C memory automatically
- Use for tutorial/challenge levels

## Basic Workflows

### Level Editor Scene Init
```
→ On Scene Init:
   → Restore Level from Memory
   → Text: "Level restored from memory!"
```

### Loading a Custom Level
```
→ Load Level Code Into Memory
   Level Code: "5,3,1,7,2,4,6,8,1,3,5,7,2,4,6,8,15,25,35,0,0,7,3,0"
→ Restore Level from Memory
→ Text: "Custom level loaded!"
```

### Loading Tutorial Levels
```
→ Load Predefined Level: 0
→ Text: "Tutorial level loaded!"
→ Change Scene: "level_editor"
```

## Predefined Level Menu System

### Level Selection Scene
```
→ Get Predefined Level Count
   Variable: $max_levels

→ Text: "Select Level (0 to " + $max_levels + ")"

→ Choice Menu:
   Option 0: "Tutorial Level"
      → Load Predefined Level: 0
      → Change Scene: "level_editor"
   
   Option 1: "Challenge Level"  
      → Load Predefined Level: 1
      → Change Scene: "level_editor"
      
   Option 2: "Expert Level"
      → Load Predefined Level: 2  
      → Change Scene: "level_editor"
```

## Advanced Level Code Manipulation

### Check Current Level Code in Variables
```
// View current level code in debugger
// Variables 50-73 automatically reflect current level state
// No events needed - just check the variables directly
```

### Load a Specific Level Code
```
→ Load Level Code Into Memory
   Level Code: "5,3,1,7,2,4,6,8,1,3,5,7,2,4,6,8,15,25,35,0,0,7,3,0"
→ Restore Level from Memory
→ Text: "Custom level loaded!"
```

### Copy Level Between Scenes
```
// Variables 50-73 automatically carry level data between scenes
// No events needed - level code persists automatically
```

### Level Code Validation
```
// Check if variables contain valid level code
→ Get Level Code Character: 16 → $player_pos
→ If Variable Greater Than: $player_pos, 40
   → Text: "Invalid level code detected!"
```

### Custom Level Editor Menu
```
→ Text: "Level Editor Menu"
→ Choice A: "Load Tutorial Level"
   → Load Predefined Level: 0
   → Text: "Tutorial loaded!"
   
→ Choice B: "Load Challenge Level"  
   → Load Predefined Level: 1
   → Text: "Challenge loaded!"
   
→ Choice C: "Load Custom Level"
   → Load Level Code Into Memory
      Level Code: "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,20,0,0,0,0,0,0,0"
   → Restore Level from Memory
   → Text: "Custom level loaded!"
```

## Simple Usage Examples

### Basic Level Editor Flow
```
Scene: Level Editor
→ On Init:
   → Restore Level from Memory
   → Text: "Level restored from memory!"
   
→ On Exit:
   // Level automatically preserved in C memory
   // No saving needed
```

### Level Sharing System
```
→ Text: "Share your level!"
→ Loop i from 0 to 23:
   → Get Level Code Character: i → $temp_char
   → Text: "Character " + i + ": " + $temp_char
   
→ Text: "Copy these values to share your level!"
```

### Level Progression
```
→ Text: "Level completed!"
→ Variable Add: $current_level, 1

→ Get Predefined Level Count → $max_levels
→ If Variable Less Than: $current_level, $current_level, $max_levels
   → Load Predefined Level: $current_level
   → Text: "Loading next level..."
→ Else:
   → Text: "All levels completed!"
```

## Tips for Implementation

1. **Setup**: No variables needed - pure C memory system

2. **Scene Init**: Always call "Restore Level from Memory" to reload after editing

3. **No Manual Saving**: Level data persists automatically in C memory

4. **Performance**: Level codes load/save instantly with no lag

5. **Debugging**: Level state preserved across scene reloads

6. **Simplicity**: Only 3 events needed for full level code system
