# Platform Paint System

## Overview

The platform paint system handles the creation, modification, and validation of platforms within the tilemap. It ensures platforms follow game rules while providing a consistent painting experience. The system is deeply integrated with the level code system and enemy placement logic to create a cohesive level editing experience.

## Platform Rules

### Length Constraints

- **Minimum Length**: 2 tiles (single-tile platforms are auto-completed or removed)
- **Maximum Length**: 8 tiles (prevents creation of overly long platforms)
- **Auto-completion**: Single platforms at edges automatically extend to 2-tile minimum
- **Segment-Aware Logic**: Platforms can span across segment boundaries while maintaining rules

### Visual-Logical Consistency

- What users see on screen exactly matches the internal platform structure
- No visual "phantom" connections that don't represent actual platform data
- Platform merging follows predictable rules
- Auto-detection of isolated platforms for cleanup

## Core Functions

### Paint Operations (paint_core.h)

```c
void paint(UBYTE x, UBYTE y);                           // Main paint function
void delete_tile(UBYTE x, UBYTE y);                    // Remove tile/entity
UBYTE get_current_tile_type(UBYTE x, UBYTE y);         // Get tile type at position
UBYTE is_within_platform_bounds(UBYTE x, UBYTE y);     // Check coordinate bounds
```

### Platform-Specific Functions (paint_platform.h)

```c
UBYTE count_connected_platform_length(UBYTE x, UBYTE y);    // Calculate total platform length
UBYTE would_2tile_platform_exceed_limit(UBYTE x, UBYTE y);  // Check merge limits
UBYTE check_platform_vertical_conflict(UBYTE x, UBYTE y);   // Check for conflicts
void rebuild_platform_row(UBYTE y);                    // Enforce platform rules
void cleanup_invalid_platforms(void);                  // Remove single-tile platforms
void remove_enemies_above_platform(UBYTE x, UBYTE y);  // Safety cleanup
```

### Entity Management (paint_entity.h)

```c
void paint_player(UBYTE x, UBYTE y);                   // Place player
void paint_enemy_right(UBYTE x, UBYTE y);              // Place right-facing enemy
void paint_enemy_left(UBYTE x, UBYTE y);               // Place left-facing enemy
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y);         // Validate enemy placement
UBYTE can_paint_enemy_left(UBYTE x, UBYTE y);          // Validate enemy placement
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y); // Find valid enemy position
```

## Paint Logic Details

### Platform Creation Process

1. **Empty Tile Click**: Creates a 2-tile platform automatically
2. **Adjacent to Platform**: Extends existing platform (if within 8-tile limit)
3. **Gap Bridging**: Creates new 2-tile platform that may auto-merge with nearby platforms
4. **Validation**: All operations check the 8-tile maximum length rule

### Auto-Completion System

When a single platform tile is created at segment edges:

```c
// Isolated platform at rightmost position gets auto-completed
if (is_isolated && i == SEGMENT_WIDTH - 1 && tile_x + 1 <= PLATFORM_X_MAX) {
    paint(tile_x + 1, current_y);  // Auto-complete to 2-tile platform
}
```

### Platform Merging

Adjacent platforms automatically merge when:

- Total length ≤ 8 tiles
- No vertical conflicts exist
- Platforms are on the same row

## Brush Preview System

### Preview States

- **BRUSH_TILE_EMPTY**: No tile at position
- **BRUSH_TILE_PLATFORM**: Platform tile present
- **BRUSH_TILE_PLAYER**: Player position
- **BRUSH_TILE_ENEMY_RIGHT**: Right-facing enemy
- **BRUSH_TILE_ENEMY_LEFT**: Left-facing enemy

### Preview Validation (paint_ui.h)

```c
UBYTE get_platform_placement_type(UBYTE x, UBYTE y);
UBYTE get_brush_tile(UBYTE x, UBYTE y);
```

Shows users what will happen before they click:

- Green: Valid platform placement
- Red: Invalid (would exceed limits)
- Yellow: Will extend existing platform

## Pattern Application Integration

### True Manual Paint Simulation (paint_vm.h)

Pattern application uses the actual `paint()` function to ensure consistency:

```c
void vm_paint_tile(void);
void vm_get_brush_tile(void);
void vm_cycle_character(void);
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id);
```

The VM wrapper functions connect to GB Studio events while pattern application uses real paint calls:

```c
void apply_pattern_with_brush_logic(UBYTE block_index, UBYTE pattern_id) {
    // Apply pattern tile by tile using real paint() calls
    for (UBYTE i = 0; i < SEGMENT_WIDTH; i++) {
        if ((row_pattern >> (4 - i)) & 1) {
            UBYTE tile_x = PLATFORM_X_MIN + (segment_x * SEGMENT_WIDTH) + i;

            // Check if already has correct platform
            if (get_meta_tile(tile_x, current_y) != PLATFORM_TILE_2) {
                paint(tile_x, current_y);  // Use actual paint function
            }
        }
    }
}
```

### Benefits of True Simulation

- **Perfect Consistency**: Code entry performs identical actions to manual painting
- **Automatic Updates**: Level codes sync immediately
- **Auto-completion**: Single platforms get completed automatically
- **Platform Merging**: Uses same logic as manual editing

## Error Prevention

### Length Validation

- **Direct Extension**: Prevents extending 8-tile platforms
- **Indirect Merging**: Prevents creating platforms that would merge beyond limits
- **Gap Analysis**: Calculates total post-merge length before allowing placement

### Safety Measures

- **Enemy Cleanup**: Removes enemies when platforms are deleted
- **Boundary Checks**: Ensures operations stay within tilemap bounds
- **Conflict Detection**: Prevents invalid vertical platform arrangements

## Cleanup System

### Cleanup System (paint_platform.h)

```c
void fix_platform_segment_rules(UBYTE segment_x, UBYTE segment_y);
void cleanup_invalid_platforms(void);
```

Automatically called to:

1. Remove single-tile platforms
2. Enforce minimum 2-tile length
3. Split platforms exceeding 8-tile maximum
4. Update level codes after changes

### Usage in Code Entry Mode

- During code entry: All patterns allowed (including single tiles)
- On exit: Cleanup automatically fixes invalid patterns
- Result: Valid tilemap that follows all platform rules

## Integration with Level Code System

### Immediate Updates

Every paint operation triggers:

```c
update_level_code_for_paint(x, y);  // Smart zone-specific update
```

### Complete Refresh

For pattern application and major changes:

```c
update_complete_level_code();  // Full re-extraction and display
```

This ensures the visual level code display always reflects the current tilemap state immediately after any paint operation.

## Enemy System Integration

### Enemy Placement (paint_entity.h)

```c
void paint_enemy_right(UBYTE x, UBYTE y);
void paint_enemy_left(UBYTE x, UBYTE y);
```

- Validates position using `can_paint_enemy_right/left()`
- Automatically selects next available enemy slot using FIFO replacement when needed
- Updates enemy paint order tracking for sequential replacement
- Handles actor movement and sprite orientation

### Position Validation (paint_entity.h)

```c
UBYTE can_paint_enemy_right(UBYTE x, UBYTE y);
UBYTE can_paint_enemy_left(UBYTE x, UBYTE y);
void find_next_valid_enemy_position(UBYTE *x, UBYTE *y);
UBYTE has_enemy_below_player(UBYTE x, UBYTE y);
```

- Requires platform tile directly below (no floating enemies)
- Checks for space above platform (no vertical overlapping)
- Prevents enemy stacking or overlapping on the same tile
- Handles edge cases near player and scene boundaries

### Player Placement Integration (paint_entity.h)

```c
void paint_player(UBYTE x, UBYTE y);
UBYTE can_paint_player(UBYTE x, UBYTE y);
```

- Ensures player can only be placed once
- Automatically repositions exit based on player location
- Validates player placement against enemies and level boundaries
- Updates level code display for player position

## Plugin Architecture Integration

The paint system is designed with a modular architecture that enables:

### Core Module Structure

- **paint_core.h/c**: Main painting logic and coordinate bounds checking
- **paint_platform.h/c**: Platform-specific validation and manipulation
- **paint_entity.h/c**: Entity placement (player, enemies) and validation
- **paint_ui.h/c**: UI feedback and brush preview system
- **paint_vm.h/c**: VM wrapper functions connecting to GB Studio events
- **paint.h**: Umbrella header including all modules

### Module Benefits

- **Code Separation**: Each module has clear responsibilities and focused functionality
- **Maintainability**: Easier to locate and modify specific features
- **Extensibility**: New features can be added to appropriate modules
- **Cross-Module Communication**: Smart integration with enemy, platform and level code systems

## Performance Optimizations

- **Cached Tile Access**: `get_current_tile_type()` in paint_core.h reduces repeated lookups
- **Boundary Check Inlining**: `is_within_platform_bounds()` in paint_core.h optimizes frequent checks
- **Smart Updates**: Zone-specific level code updates instead of full recalculation
- **State Tracking**: Enemy slot management in paint_entity.h prevents unnecessary iteration
- **Modular Design**: Each module is optimized for its specific responsibilities
