# Enemy Encoding System Refactor - IMPLEMENTED

## Overview

This document describes the **completed** enemy encoding system for the 24-character level code, using a 7-character scheme (positions 17-23) to encode enemy positions, odd-column parity, and directions. The system has been fully implemented and integrated with the tilemap editor and actor management systems.

## Implementation Status: ✅ COMPLETE

The enemy encoding system has been successfully refactored and implemented with the following key features:

- **NEW POS41/BASE32 encoding system** replacing the old pattern-based approach
- **Bidirectional editing**: Level code changes instantly update tilemap and actors
- **Full cycling support**: All 41 POS41 positions accessible via character cycling
- **Extended character set**: Uses metatile sheet tiles 84-88 for characters beyond 'Z'
- **Actor synchronization**: Enemy actors move/clear automatically when level code changes
- **Backward compatibility**: Legacy functions maintained for existing code

## Character Sets (IMPLEMENTED)

### POS41 Alphabet (41 symbols)

```c
const char POS41[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%";
```

- **'0'**: No enemy (position 0)
- **'1'-'9'**: Positions 1-9 (tiles 49-57)
- **'A'-'Z'**: Positions 10-35 (tiles 58-83)
- **'!@#$%'**: Positions 36-40 (tiles 84-88, extended metatile characters)

### BASE32 Alphabet (32 symbols)

```c
const char BASE32[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
```

Used for bitmasks (odd-column parity and directions).

## Level Code Structure (24 characters) - IMPLEMENTED

| Chars | Purpose                                 | Status |
| :---: | :-------------------------------------- | :----: |
| 0–15  | Platform patterns (16 blocks)           |   ✅   |
|  16   | Player column (0–19)                    |   ✅   |
| 17–21 | Enemy positions (5 chars, POS41)        |   ✅   |
|  22   | Odd-column parity mask (1 char, BASE32) |   ✅   |
|  23   | Direction mask (1 char, BASE32)         |   ✅   |

## Enemy Position Encoding Algorithm - IMPLEMENTED

The system maps 5 enemies across 4 rows × 20 columns using anchor-based encoding:

### Encoding Process

```c
// For enemy k at (row: 0–3, col: 0–19)
UBYTE anchor = col / 2;                    // 0–9 (even-column anchors)
UBYTE idx = 1 + row * 10 + anchor;        // 1–40
char pos_char = POS41[idx];                // Position character

// Special case: no enemy
if (enemy_positions[k] == 255) {
    pos_char = POS41[0];  // '0' = no enemy
}
```

### Row Assignment (IMPLEMENTED)

```c
// Maps enemy index to row (0-3)
const UBYTE ENEMY_ROW_MAP[MAX_ENEMIES] = {0, 1, 2, 3, 0, 1};
// 6 enemies: 0→row0, 1→row1, 2→row2, 3→row3, 4→row0, 5→row1
```

## Mask Encoding - IMPLEMENTED

### Odd-Column Parity Mask (Character 22)

```c
char encode_odd_mask(void) {
    UBYTE odd_mask = 0;
    for (UBYTE k = 0; k < LEVEL_CODE_MAX_ENEMIES; k++) {
        if (current_level_code.enemy_positions[k] != 255) {
            UBYTE col = current_level_code.enemy_positions[k];
            if (col & 1) { // If column is odd
                odd_mask |= (1 << k);
            }
        }
    }
    return BASE32[odd_mask & 0x1F];
}
```

### Direction Mask (Character 23)

```c
char encode_direction_mask(void) {
    UBYTE dir_mask = current_level_code.enemy_directions & 0x1F;
    return BASE32[dir_mask];
}
```

## Decoding Workflow - IMPLEMENTED

### Position Decoding

```c
void decode_enemy_position(UBYTE enemy_index, char pos_char, UBYTE odd_bit, UBYTE dir_bit) {
    UBYTE idx = find_pos41_index(pos_char);

    if (idx == 255 || idx == 0) {
        // Clear enemy (invalid char or '0')
        current_level_code.enemy_positions[enemy_index] = 255;
        clear_enemy_actor(enemy_index);
        return;
    }

    // Decode: idx = 1 + row*10 + anchor
    UBYTE v = idx - 1;           // 0-39
    UBYTE row = v / 10;          // 0-3
    UBYTE anchor = v % 10;       // 0-9
    UBYTE col = anchor * 2 + odd_bit;  // Recover actual column

    // Update game state
    current_level_code.enemy_positions[enemy_index] = col;

    // Update tilemap
    UBYTE tilemap_x = PLATFORM_X_MIN + col;
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;
    UBYTE enemy_tile = dir_bit ? TILE_LEFT_ENEMY : TILE_RIGHT_ENEMY;
    replace_meta_tile(tilemap_x, actual_y, enemy_tile, 1);

    // Update actor
    place_enemy_actor(enemy_index, tilemap_x, actual_y, dir_bit);
}
```

## Key Implementation Features

### 1. Bidirectional Level Code Editing ✅

- **Direct character editing**: `handle_enemy_data_edit(char_index, new_value)`
- **Increment/decrement**: `vm_increment_enemy_level_code_char()`, `vm_decrement_enemy_level_code_char()`
- **Real-time updates**: Changes immediately reflect in tilemap and actors

### 2. Extended Character Support ✅

```c
const UBYTE POS41_TILE_MAP[] = {
    48,                                     // '0' (tile 48)
    49, 50, 51, 52, 53, 54, 55, 56, 57,     // '1'-'9' (tiles 49-57)
    58, 59, 60, 61, 62, 63, 64, 65, 66, 67, // 'A'-'J' (tiles 58-67)
    68, 69, 70, 71, 72, 73, 74, 75, 76, 77, // 'K'-'T' (tiles 68-77)
    78, 79, 80, 81, 82, 83,                 // 'U'-'Z' (tiles 78-83)
    84, 85, 86, 87, 88                      // Extended chars (tiles 84-88)
};
```

### 3. Actor Management Integration ✅

```c
// Clear specific enemy actor
void clear_enemy_actor(UBYTE enemy_index);

// Place enemy actor at position
void place_enemy_actor(UBYTE enemy_index, UBYTE tilemap_x, UBYTE tilemap_y, UBYTE direction);

// Clear all enemy actors
void clear_all_enemy_actors(void);
```

### 4. VM Script Integration ✅

- `vm_edit_enemy_level_code_char()`: Direct character editing from scripts
- `vm_increment_enemy_level_code_char()`: Increment with wraparound
- `vm_decrement_enemy_level_code_char()`: Decrement with wraparound
- `vm_get_enemy_encoding_info()`: Get current encoding values

## Code Structure

### Core Files Implemented

- **`code_enemy_system.c/.h`**: New POS41/BASE32 encoding system
- **`code_level_core.c/.h`**: Integration with level code display and editing
- **Updated `code_level_core.h`**: Fixed struct definition and MAX_ENEMIES constant

### Key Functions Implemented

#### Encoding Functions

- `encode_enemy_position(enemy_index)` → POS41 character
- `encode_odd_mask()` → BASE32 character for odd columns
- `encode_direction_mask()` → BASE32 character for directions

#### Decoding Functions

- `decode_enemy_position(enemy_index, pos_char, odd_bit, dir_bit)`
- `decode_enemy_data_from_level_code(enemy_chars[7])`

#### Interface Functions (Backward Compatible)

- `encode_enemy_positions()` → Character 17 value
- `encode_enemy_details_1()` → Character 18 value
- `encode_enemy_details_2()` → Character 19 value
- `encode_enemy_position_4()` → Character 20 value
- `encode_enemy_position_5()` → Character 21 value
- `encode_odd_mask_value()` → Character 22 value
- `encode_enemy_directions()` → Character 23 value

### Constants Defined

```c
#define MAX_ENEMIES 6                // System maximum (actors)
#define LEVEL_CODE_MAX_ENEMIES 5     // Level code encoding limit
#define LEVEL_CODE_CHARS_TOTAL 24    // Total level code characters
```

## Testing and Validation ✅

### Test Functions Implemented

- `test_new_enemy_encoding()`: Test with sample data
- `validate_enemy_encoding_roundtrip()`: Verify encode/decode consistency
- `vm_test_enemy_level_code_editing()`: Test direct level code editing

### Edge Cases Handled

- Invalid position characters default to '0' (no enemy)
- Out-of-bounds positions are clamped/cleared
- Array bounds checking for both LEVEL_CODE_MAX_ENEMIES and MAX_ENEMIES
- Proper wraparound for character cycling (0-40 for POS41, 0-31 for BASE32)

## Integration Status

### ✅ Completed Integrations

- **Tilemap Editor**: Real-time tilemap updates when level code changes
- **Actor System**: Enemies move/clear when positions change
- **Display System**: All 41 POS41 characters display correctly
- **Level Code Cycling**: Full support for cycling through all positions
- **VM Script Access**: Complete script interface for enemy editing
- **Backward Compatibility**: Legacy functions maintained

### ✅ Fixed Issues

- Removed `states_defines.h` dependency causing build failures
- Fixed struct field ordering (flexible array member at end)
- Defined `MAX_ENEMIES` constant in header
- Resolved all macro redefinition issues
- Fixed array bounds consistency between encoding (5) and actor (6) systems

## Usage Examples

### Direct Level Code Editing

```c
// Set enemy 0 to position index 15 (will be decoded to appropriate row/col)
handle_enemy_data_edit(17, 15);

// Set direction mask to 0b01010 (enemies 1,3 face left)
handle_enemy_data_edit(23, 10);
```

### Character Cycling

```c
// Increment character 17 (first enemy position) with wraparound
vm_increment_enemy_level_code_char(17);  // 0→1→2→...→40→0

// Increment character 23 (direction mask) with wraparound
vm_increment_enemy_level_code_char(23);  // 0→1→2→...→31→0
```

## Performance Notes

- **Encoding**: O(1) per enemy, O(5) total
- **Decoding**: O(1) per position lookup, O(5) total enemy processing
- **Character lookup**: Linear search O(41) for POS41, O(32) for BASE32
- **Memory**: Minimal overhead, uses existing level code structure

## Future Enhancements

- **Optimized lookup tables**: Replace linear search with direct indexing
- **Enemy type encoding**: Use reserved bits in masks for walker/jumper types
- **Validation checksums**: Use unused character space for data integrity
- **Pattern templates**: Predefined enemy arrangements for quick level design

---

**Status**: ✅ **IMPLEMENTATION COMPLETE AND FULLY FUNCTIONAL**

All enemy encoding functionality has been implemented, tested, and integrated with the existing tilemap editor system. The system supports bidirectional editing, full character cycling, extended character sets, and maintains backward compatibility.
