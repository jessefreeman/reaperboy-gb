# String-Based Level Code System

## Overview

The string-based level code system stores level codes as 24 individual character values in GB Studio variables. This allows you to:

- **Save/Load Levels**: Store and restore level designs that persist across scene reloads
- **Package Pre-made Levels**: Include level codes directly in your game
- **Easy Import/Export**: Level codes are simple arrays of 24 numbers (0-40 range)

## Character Encoding

Each of the 24 characters uses this encoding:

- **0-9**: Numbers (tile IDs 48-57)
- **10-35**: Letters A-Z (tile IDs 58-83) 
- **36-40**: Special characters !, @, #, $, % (tile IDs 84-88)

## Level Code Layout

The 24 characters represent:

- **Characters 0-15**: Platform patterns (values 0-34)
- **Character 16**: Player starting column (values 0-40)
- **Characters 17-21**: Enemy positions (POS41 system: 0-40)
- **Characters 22-23**: Enemy direction/parity masks (BASE32 system: 0-31)

## Setup Instructions

### 1. Create GB Studio Variables

In your GB Studio project, create **24 consecutive variables** for level code storage:

```
level_code_char_0  (Variable ID: 50)
level_code_char_1  (Variable ID: 51)
level_code_char_2  (Variable ID: 52)
...
level_code_char_23 (Variable ID: 73)
```

### 2. Update Configuration

In `code_persistence.h`, update the base variable ID:

```c
#define VAR_LEVEL_CODE_CHAR_BASE 50  // Change to your first variable ID
```

### 3. GB Studio Event Usage

The plugin provides 8 easy-to-use events that appear in your GB Studio event menu:

#### **Save Level Code**
- **Event Name**: "Save Level Code"
- **Description**: Saves the current level design as a 24-character level code to memory
- **Usage**: Simply add this event when exiting the level editor
- **Persistence**: The code survives scene reloads and game restarts

#### **Load Level Code**
- **Event Name**: "Load Level Code" 
- **Description**: Loads a previously saved level code and rebuilds the level
- **Usage**: Add this event when entering the level editor to restore saved levels

#### **Check for Saved Level Code**
- **Event Name**: "Check for Saved Level Code"
- **Variable**: Choose a variable to store the result (1 = has data, 0 = no data)
- **Usage**: Use this to create "Continue Level" vs "New Level" menu options

#### **Clear Level Code**
- **Event Name**: "Clear Level Code"
- **Description**: Clears all saved level code data from memory
- **Usage**: Use for "Reset Level" or "New Level" functionality

#### **Load Predefined Level**
- **Event Name**: "Load Predefined Level"
- **Level Index**: Choose level number (0, 1, 2, etc.) or use a variable
- **Description**: Loads built-in levels that are packaged with your game
- **Usage**: Perfect for level selection menus and tutorials

#### **Get Predefined Level Count**
- **Event Name**: "Get Predefined Level Count"
- **Variable**: Choose a variable to store the total number of levels
- **Usage**: Use this to create dynamic level selection menus

#### **Set Level Code Character** (Advanced)
- **Event Name**: "Set Level Code Character"
- **Character Index**: Position (0-23) to modify
- **Character Value**: New value (0-40 range)
- **Usage**: For manual level code construction or level code import systems

#### **Get Level Code Character** (Advanced)
- **Event Name**: "Get Level Code Character"
- **Character Index**: Position (0-23) to read
- **Result Variable**: Variable to store the character value
- **Usage**: For level code export, validation, or sharing systems

## Adding Predefined Levels

Edit the `PREDEFINED_LEVELS` array in `code_persistence.c`:

```c
const predefined_level_t PREDEFINED_LEVELS[] = {
    // Level 0: Tutorial level
    {{ 1, 1, 1, 1,  2, 2, 2, 2,  3, 3, 3, 3,  4, 4, 4, 4,  // Platforms
       10,                                                    // Player at column 10
       0, 0, 0, 0, 0,                                        // No enemies
       0, 0 }},                                              // No enemy masks
       
    // Level 1: Challenge level  
    {{ 5, 3, 1, 7,  2, 4, 6, 8,  1, 3, 5, 7,  2, 4, 6, 8,  // Complex platforms
       5,                                                     // Player at column 5
       15, 25, 35, 0, 0,                                     // 3 enemies
       7, 3 }},                                              // Enemy direction data
       
    // Add more levels here...
};
```

## Level Code Examples

### Simple Starting Level
```
Platform patterns: 1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4
Player position: 10
Enemy data: 0,0,0,0,0,0,0 (no enemies)
```

### Complex Level with Enemies
```
Platform patterns: 5,3,1,7,2,4,6,8,1,3,5,7,2,4,6,8
Player position: 5
Enemy positions: 15,25,35,0,0 (3 enemies at columns 15,25,35)
Enemy masks: 7,3 (direction and parity data)
```

## Advanced Usage

### Individual Character Access

```c
// Set character 0 to value 5
vm_set_level_code_character(0, 5);

// Get character 16 (player position)
UBYTE player_pos = vm_get_level_code_character(16);
```

### Level Code Generation Workflow

1. **Design Level**: Use your level editor to create the level
2. **Save Level Code**: Call `vm_save_level_code_string`
3. **Export Data**: Read the 24 variables to get the level code array
4. **Add to Game**: Include the level code in `PREDEFINED_LEVELS`

## Benefits

- **Persistent Storage**: Level codes survive scene reloads and game restarts
- **Compact Format**: 24 bytes per level (very memory efficient)
- **Easy Packaging**: Include dozens of levels directly in your ROM
- **Debug Friendly**: Level codes are human-readable sequences
- **Flexible**: Can store and restore any level configuration

## Integration with Level Editor

The string-based system works seamlessly with your existing level editor:

- **Auto-save**: Save level codes when exiting the editor
- **Quick Load**: Restore levels when entering the editor
- **Level Browser**: Create a menu to browse predefined levels
- **Share Codes**: Players can share level codes as simple number sequences
