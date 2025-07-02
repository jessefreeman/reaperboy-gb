# Enemy Position Value Mapping Reference

## POS41 System (Enemy Positions - Characters 17-21)

| Value | ASCII Tile | Description   | Position Formula     |
| ----- | ---------- | ------------- | -------------------- |
| 0     | '0' (48)   | No enemy      | n/a                  |
| 1     | '1' (49)   | Row 0, Col 0  | 1 + row\*10 + anchor |
| 2     | '2' (50)   | Row 0, Col 2  | 1 + row\*10 + anchor |
| ...   | ...        | ...           | ...                  |
| 9     | '9' (57)   | Row 0, Col 16 | 1 + row\*10 + anchor |
| 10    | 'A' (58)   | Row 0, Col 18 | 1 + row\*10 + anchor |
| 11    | 'B' (59)   | Row 1, Col 0  | 1 + row\*10 + anchor |
| ...   | ...        | ...           | ...                  |
| 35    | 'Z' (83)   | Row 3, Col 8  | 1 + row\*10 + anchor |
| 36    | '!' (84)   | Row 3, Col 10 | 1 + row\*10 + anchor |
| 37    | '@' (85)   | Row 3, Col 12 | 1 + row\*10 + anchor |
| 38    | '#' (86)   | Row 3, Col 14 | 1 + row\*10 + anchor |
| 39    | '$' (87)   | Row 3, Col 16 | 1 + row\*10 + anchor |
| 40    | '%' (88)   | Row 3, Col 18 | 1 + row\*10 + anchor |

## BASE32 System (Mask Values - Characters 22-23)

| Value | ASCII Tile | Description    |
| ----- | ---------- | -------------- |
| 0     | '0' (48)   | 00000 (binary) |
| 1     | '1' (49)   | 00001 (binary) |
| ...   | ...        | ...            |
| 9     | '9' (57)   | 01001 (binary) |
| 10    | 'A' (58)   | 01010 (binary) |
| ...   | ...        | ...            |
| 31    | 'V' (79)   | 11111 (binary) |

## Debugging Notes

- Values 0-40 should cycle correctly for enemy position characters (17-21)
- Each enemy's position is fully independent and determined by:
  - The numeric value (0-40) stored in characters 17-21 (one character per enemy)
  - The odd bit for that enemy from character 22
  - The direction bit for that enemy from character 23
