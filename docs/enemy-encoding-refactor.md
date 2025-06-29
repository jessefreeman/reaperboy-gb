# Enemy Encoding Refactor - Level Code System

## Overview

This document describes the finalized enemy encoding system for the level code, using a pattern-based approach. There are 12 unique enemy pattern variations (out of 9 possible base patterns), each with a unique ID. Each pattern variation is either a prime pattern or its horizontal flip. Enemies can only be placed on the top row (5 tiles) of each block.

## Character Set

- **Available characters**: 36 total (0-9, A-Z)
- **Encoding range**: 0-35 values per character

## Level Code Structure (24 characters total)

- **Characters 0-15**: Platform patterns (16 blocks, 5x2 each)
- **Character 16**: Player column position (0-19)
- **Characters 17-21**: Enemy pattern data (5 characters, one per block row)
- **Character 22**: Enemy direction bitmask (1 character, 5 bits used)
- **Character 23**: Reserved for future use (validation, expansion, or padding)

## Enemy Pattern Encoding

### 12 Unique Enemy Pattern Variations

Each pattern is a 5-bit string ("1" = enemy present, "0" = empty) for the top row of a block. Patterns with two variations are horizontally flipped versions of each other and get unique IDs.

| Pattern ID | Pattern (top row) | Description                    |
| ---------- | ----------------- | ------------------------------ |
| 0          | 00000             | Empty                          |
| 1          | 00001             | Rightmost                      |
| 2          | 10000             | Leftmost (flip of 1)           |
| 3          | 00010             | 2nd from right                 |
| 4          | 01000             | 2nd from left (flip of 3)      |
| 5          | 00100             | Center                         |
| 6          | 00101             | Center + rightmost             |
| 7          | 10100             | Center + leftmost (flip 6)     |
| 8          | 01010             | 2nd from left + 2nd right      |
| 9          | 10101             | Left, center, right            |
| 10         | 10001             | Ends only                      |
| 11         | 01001             | 2nd left + rightmost           |
| 12         | 10010             | Leftmost + 2nd right (flip 11) |

- Patterns with only one variation (e.g., 00000, 00100, 01010, 10101, 10001) are their own flip.
- Each pattern is mapped to a unique character (0–9, A–D) for encoding.

### Pattern Table Example

```c
const char* ENEMY_PATTERNS[13] = {
  "00000", // 0
  "00001", // 1
  "10000", // 2
  "00010", // 3
  "01000", // 4
  "00100", // 5
  "00101", // 6
  "10100", // 7
  "01010", // 8
  "10101", // 9
  "10001", // A
  "01001", // B
  "10010"  // C
};
```

### Encoding/Decoding Workflow

- **To encode**: For each enemy pattern slot, match the top row of the block to a pattern, assign its unique ID (0–C), and encode as a character.
- **To decode**: Map char (0–C) to pattern ID, retrieve the 5-bit pattern.
- **Directions**: Use a single character as a 5-bit bitmask for enemy directions (bit 0 = first enemy, bit 1 = second, etc.; 0 = right, 1 = left).
- **Reserved code char**: Character 23 can be used for future features or validation.

### Example

- Level code chars 17–21: Enemy patterns (e.g., "01347")
- Level code char 22: Direction bitmask (e.g., "A" for 01010b)
- Level code char 23: Reserved (set to "0" or used for future features)

## Implementation Notes

- **Encoding and decoding functions** should use the pattern table and bitmask logic as described above.
- **Validation**: The unused code char can be used for error detection or future expansion.
- **Maintainability**: The system is designed for easy extension and robust error checking.

---
