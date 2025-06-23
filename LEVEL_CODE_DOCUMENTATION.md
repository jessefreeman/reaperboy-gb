# Level Code Encoding System Documentation

## Overview

The level code system provides a compact, optimized way to serialize and deserialize player and enemy positions for GB Studio platformer levels. The system uses bit-packing to minimize storage requirements while maintaining full fidelity of game state.

## Encoding Specification

### Total Storage: 8 bytes (64 bits)

- **Player data**: 5 bits
- **Enemy data**: 6 enemies × 9 bits each = 54 bits
- **Total used**: 59 bits (5 bits unused)

### Player Encoding (5 bits)

- **X Position**: 5 bits (0-19, relative to PLATFORM_X_MIN)
- **Y Position**: Fixed at row 11 (not encoded)
- **Range**: Columns 2-21 on the map

### Enemy Encoding (9 bits per enemy)

- **X Position**: 5 bits (0-19, relative to PLATFORM_X_MIN)
- **Y Position**: 3 bits (encoded as 0=row13, 1=row15, 2=row17, 3=row19, 7=no enemy)
- **Direction**: 1 bit (0=right facing, 1=left facing)

### Empty Slots

- Unused enemy slots are marked with Y=7 (binary 111)
- X and direction are set to 0 for empty slots

## API Functions

### Core Functions

```c
void generate_level_code(UBYTE *code_buffer);
void parse_level_code(UBYTE *code_buffer);
UBYTE encode_enemy_y(UBYTE y);
UBYTE decode_enemy_y(UBYTE encoded_y);
```

### VM Wrapper Functions

```c
void vm_generate_level_code(SCRIPT_CTX *THIS);    // ARG0: variable ID to store 8 bytes
void vm_parse_level_code(SCRIPT_CTX *THIS);       // ARG0: variable ID containing 8 bytes
void vm_validate_level_setup(SCRIPT_CTX *THIS);   // ARG0: variable ID for validation result
void vm_get_level_stats(SCRIPT_CTX *THIS);        // ARG0: player_x var, ARG1: enemy_count var
```

### Utility Functions

```c
UBYTE validate_level_setup(void);                 // Returns 1 if player exists, 0 otherwise
void get_level_stats(UBYTE *player_x, UBYTE *enemy_count);
```

## Usage Examples

### Generating a Level Code

```c
UBYTE level_code[8];
generate_level_code(level_code);
// level_code now contains the 8-byte encoded level
```

### Loading a Level Code

```c
UBYTE level_code[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
parse_level_code(level_code);
// Map is now updated with player and enemies from the code
```

### In GB Studio Scripts

1. **Generate Code**: Use "Generate Level Code" event with a variable ID
2. **Parse Code**: Use "Parse Level Code" event with a variable ID containing the code
3. **Validate**: Use "Validate Level Setup" to check if level has a player

## Constraints and Validation

### Player Constraints

- Must be placed on row 11
- Must be above a platform tile
- Only one player allowed per level

### Enemy Constraints

- Must be placed above platform tiles
- Only on rows 13, 15, 17, or 19
- Must be at least 1 tile apart from other enemies
- Maximum of 6 enemies per level

### Platform Constraints

- Platforms can be placed on rows 13, 15, 17, or 19
- Deleting a platform removes any enemies above it

## Bit Layout

```
Bytes:  0        1        2        3        4        5        6        7
Bits:   76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210

Player X (5 bits):                 XXXXX
Enemy 1 (9 bits):              YYY      XXXXX D
Enemy 2 (9 bits):          YYY      XXXXX D
Enemy 3 (9 bits):      YYY      XXXXX D
Enemy 4 (9 bits):  YYY      XXXXX D
Enemy 5 (9 bits):       XXXXX D YYY
Enemy 6 (9 bits): XXXXX D YYY
Unused (5 bits):              UUUUU

Legend:
X = X position (5 bits)
Y = Y position (3 bits, encoded)
D = Direction (1 bit)
U = Unused (always 0)
```

## Error Handling

- Invalid positions are skipped during parsing
- Missing platforms cause enemies to not be placed
- Validation functions help ensure level integrity
- Y=7 encoding safely handles empty enemy slots

## Performance Notes

- Bit-packing minimizes memory usage (8 bytes vs 14+ bytes uncompressed)
- Single-pass generation and parsing algorithms
- Efficient validation checks
- Compatible with GB Studio variable system (8 consecutive variables)

## Future Enhancements

Potential additions could include:

- Platform encoding for full level serialization
- Checksum validation for data integrity
- Compression for even smaller codes
- Support for more entity types

## Platform Pattern Display System

### Extended Character Set Support

The platform pattern display system has been extended to support more than 16 unique patterns by using an expanded character set beyond hexadecimal.

#### Character Mapping

- **0-9**: Numeric characters (patterns 0-9)
  - Tile positions: (0,3) to (9,3) = tile indices 48-57
- **A-F**: Hexadecimal letters (patterns 10-15)
  - Tile positions: (10,3) to (15,3) = tile indices 58-63
- **G-T**: Extended alphabet (patterns 16-20+)
  - G (pattern 16): Tile position (4,0) = tile index 4
  - H (pattern 17): Tile position (5,0) = tile index 5
  - And so on...

#### Pattern Display Functions

```c
void display_pattern_char(UBYTE value, UBYTE x, UBYTE y);    // Supports 0-31 (0-9, A-F, G-V)
void display_hex_digit(UBYTE value, UBYTE x, UBYTE y);       // Legacy hex-only (0-F)
void display_debug_patterns_formatted(void);                 // Shows all patterns in 4-char blocks
void generate_and_display_level_code(void);                  // Level code with extended chars
```

#### Test Functions

```c
void test_all_pattern_chars(void);                          // Display all supported characters
void vm_test_all_pattern_chars(SCRIPT_CTX *THIS);          // VM wrapper
void test_level_code_display(void);                         // Sample pattern display
void vm_test_level_code_display(SCRIPT_CTX *THIS);         // VM wrapper
```

#### GB Studio Events

The following events are available for testing and using the extended pattern display system:

- **"Test All Pattern Characters"**: Displays all supported characters (0-9, A-F, G-T)
- **"Test Pattern Mapping"**: Verifies specific character mappings (0, 9, A, F, G, H, T)
- **"Display Level Code (Extended)"**: Shows current level patterns using extended character set
- **"Display Debug Formatted"**: Shows debug patterns in 4-character blocks
- **"Test Hex Tiles"**: Legacy test for hexadecimal tiles only (0-F)

These events can be used in GB Studio scripts to test and display platform patterns during development.

#### Display Format

Both debug patterns and level codes are displayed in 4-character blocks with spaces:

- Format: `XXXX XXXX XXXX`
- Row 1: 12 characters (3 blocks of 4)
- Row 2: 4 characters (1 block)
- Total: 16 pattern characters

#### Supported Patterns

The system supports 21 unique platform patterns (0-20), which maps to characters:

- 0-9 (patterns 0-9)
- A-F (patterns 10-15)
- G-T (patterns 16-20)

This ensures that all platform patterns can be uniquely represented and displayed without ambiguity.

#### Single Source of Truth

Both the debug display and level code systems use the same pattern extraction logic to ensure consistency:

1. **`extract_chunk_pattern()`**: Extracts 5x2 platform patterns from the map
2. **`match_platform_pattern()`**: Maps extracted patterns to unique pattern IDs (0-20)
3. **`display_pattern_char()`**: Converts pattern IDs to the correct tile characters

This unified approach ensures that debug displays always match the level code representation.
