# Enemy System Documentation

## Overview

The enemy placement system in the Tilemap Editor plugin controls where enemies can be placed in the level editor. This system ensures that enemies are only placed in valid locations that make sense from a gameplay perspective and ensures the level code system works correctly.

## Rules for Enemy Placement

### 1. Platform Requirement

- Enemies can only be placed directly above platform tiles (1:1 relationship)
- An enemy must have a platform tile directly beneath it (in the row immediately below)
- No platform elsewhere in the column will satisfy this requirement
- When a platform is deleted, any enemy directly above it is automatically deleted

### 2. Player and Exit Avoidance

- Enemies cannot be placed directly below the player
- This prevents unfair gameplay where the player would immediately collide with an enemy when dropping down
- Enemies cannot be placed above the exit to avoid blocking the level completion

### 3. No Stacking

- Multiple enemies cannot occupy the same position
- When placing an enemy at a position where an enemy already exists, the existing one is replaced

## Implementation Details

### Core Functions

```c
// Paint functions
void paint_enemy_right(UBYTE x, UBYTE y);
void paint_enemy_left(UBYTE x, UBYTE y);

// Validation
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y);
UBYTE can_paint_enemy_left(UBYTE x, UBYTE y);
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y);
UBYTE has_enemy_below_player(UBYTE x, UBYTE y);

// Cleanup
void remove_enemies_above_platform(UBYTE x, UBYTE y);
void delete_enemy(UBYTE x, UBYTE y);
```

### Enemy Management

```c
// Actor IDs for enemies
UBYTE paint_enemy_ids[5] = {2, 3, 4, 5, 6};
UBYTE paint_enemy_slots_used[5] = {0, 0, 0, 0, 0};

// FIFO replacement tracking
UBYTE enemy_paint_order[5];
UBYTE enemy_paint_count = 0;
UBYTE next_paint_slot = 0;
```

## Enemy Position Encoding

### Level Code Format

The enemy positions are encoded in the level code using a compact system:

- **Characters 17-20**: Enemy summary data
- **Characters 21-23**: Detailed enemy positions and types

### Position Value Mapping

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
| 40    | '%' (88)   | Row 3, Col 18 | 1 + row\*10 + anchor |

### Direction Encoding

The last character of the level code encodes the direction (left/right) for each enemy:

```c
// Example: encoding enemy directions
UBYTE direction_bits = 0;
for (UBYTE i = 0; i < enemy_count; i++) {
    if (enemy_directions[i] == DIR_RIGHT) {
        direction_bits |= (1 << i);  // Set bit for right-facing enemy
    }
}
```

## Paint System Integration

### Enemy Position Finding

The `find_next_valid_enemy_position()` function helps users find a valid enemy position:

1. Starts at the current cursor position
2. Checks if the position is valid for an enemy
3. If not, searches in a spiral pattern outward
4. Returns the first valid position found

### FIFO Replacement System

When the maximum of 5 enemies is reached, the system replaces the oldest enemy:

1. Tracks the order in which enemies were painted using `enemy_paint_order[]`
2. When a 6th enemy is added, finds and replaces the oldest enemy
3. Updates tracking arrays to maintain the correct FIFO order

```c
// Example: Finding the oldest enemy slot to replace
UBYTE find_oldest_enemy_slot() {
    return enemy_paint_order[0]; // First position holds oldest enemy
}

// After replacement, shift the array
void update_paint_order_after_replacement(UBYTE slot) {
    for (UBYTE i = 0; i < 4; i++) {
        enemy_paint_order[i] = enemy_paint_order[i+1];
    }
    enemy_paint_order[4] = slot; // New enemy becomes newest
}
```

## Level Code Integration

Enemy data is automatically extracted and encoded during level code generation:

```c
void update_enemy_data_in_level_code() {
    // Count enemies
    level_code.enemy_count = count_enemies_on_map();

    // Store positions and directions
    UBYTE enemy_index = 0;
    for (UBYTE y = PLATFORM_Y_MIN; y <= PLATFORM_Y_MAX; y++) {
        for (UBYTE x = PLATFORM_X_MIN; x <= PLATFORM_X_MAX; x++) {
            if (is_enemy_at_position(x, y)) {
                level_code.enemy_positions[enemy_index*2] = x;
                level_code.enemy_positions[enemy_index*2+1] = y;
                level_code.enemy_types[enemy_index] = get_enemy_type(x, y);
                enemy_index++;
            }
        }
    }
}
```
