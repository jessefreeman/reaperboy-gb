## Overview

This document describes the updated enemy-encoding system for the 24-character level code, using a 7-character scheme to pack each enemy’s position (row & column parity) and direction.

## Character Set

* **Available symbols**: 36 total (0–9, A–Z)
* **Position alphabet (41 symbols)**: reserve `0` for “no enemy”, then `1–9`, `A–Z`, `!@#$%` → 41 values
* **Base-32 alphabet**: `0–9, A–V` (32 symbols)

## Level Code Structure (24 characters)

| Chars | Purpose                                  |
| :---: | :--------------------------------------- |
|  0–15 | Platform patterns (16 blocks)            |
|   16  | Player column (0–19)                     |
| 17–21 | Enemy positions (5 chars, POS41)         |
|   22  | Odd-column parity mask (1 char, base-32) |
|   23  | Direction mask (1 char, base-32)         |

## Enemy Position Encoding

We map each enemy onto 4 rows × 20 columns by anchoring to even-column slots and tracking odd parity:

1. **Anchor columns**: even columns `0,2,…,18` → 10 anchors.
2. **Index calculation**:

   ```c
   // for enemy k at (row: 0–3, col: 0–19)
   uint8_t anchor = col/2;              // 0–9
   uint8_t idx    = 1 + row*10 + anchor; // 1–40
   code[17 + k]   = POS41[idx];         // '0'=none, '1'–'9','A'–'Z','!','#','$','%'
   ```
3. **Empty slot**: if no enemy, use `code[17+k] = '0'`.

## Odd-Column Parity Mask (Char 22)

One bit per enemy k (bit 0→enemy 0 ... bit 4→enemy 4):

```c
uint8_t oddMask = 0;
for(int k=0; k<5; ++k) {
  if (enemies[k].col & 1) oddMask |= (1<<k);
}
code[22] = BASE32[oddMask];  // '0'–'V'
```

## Direction Mask (Char 23)

One bit per enemy k: 0=right, 1=left.

```c
uint8_t dirMask = 0;
for(int k=0; k<5; ++k) {
  if (enemies[k].dir == LEFT) dirMask |= (1<<k);
}
code[23] = BASE32[dirMask];  // '0'–'V'
```

## Decoding Workflow

1. **Read positions** (chars 17–21):

   ```c
   int idx = findIndex(POS41, code[17+k]); // 0–40
   if (idx == 0) continue;                 // no enemy
   uint8_t v      = idx - 1;
   row    = v / 10;
   anchor = v % 10;
   ```
2. **Read oddMask**: `oddMask = invBASE32(code[22]);`
3. **Read dirMask**: `dirMask = invBASE32(code[23]);`
4. **Recover column and direction**:

   ```c
   col = anchor*2 + ((oddMask>>k)&1);
   dir = (dirMask>>k)&1;  // 0=right,1=left
   ```

## Implementation Notes

* Define:

  ```c
  const char *POS41 = "0"                    // no enemy
                       "123456789"            // 1–9
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // 10–35
                       "!@#$%";               // 36–40
  const char *BASE32 = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
  ```
* Implement `findIndex()` and `invBASE32()` to map chars back to values.
* Validate `idx≤40`, `oddMask<32`, `dirMask<32`.
* Char 23 can double as a version byte or checksum if desired.
