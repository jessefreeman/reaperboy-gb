#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_enemy_system.h"
#include "code_level_core.h"
#include "tile_utils.h"

// ============================================================================
// ENEMY PATTERN DATA (from refactor document)
// ============================================================================

// Enemy patterns for new pattern system
const enemy_pattern_t ENEMY_PATTERNS[] = {
    {0, {255, 255, 255, 255, 255, 255}, 0, "No enemies"},
    {1, {5, 255, 255, 255, 255, 255}, 1, "Single enemy center"},
    {2, {2, 8, 255, 255, 255, 255}, 2, "Two enemies spread"},
    {3, {1, 5, 9, 255, 255, 255}, 3, "Three enemies spaced"},
    // Add more patterns as needed
};

// ============================================================================
// ENEMY TYPE DETECTION
// ============================================================================

UBYTE detect_enemy_type(UBYTE tile_type) BANKED
{
    // Future expansion: detect different enemy types
    switch (tile_type)
    {
    case BRUSH_TILE_ENEMY_L:
    case BRUSH_TILE_ENEMY_R:
        return 0; // Walker type
    // Future: Add new brush tile types for jumper enemies
    // case BRUSH_TILE_ENEMY_JUMPER_L:
    // case BRUSH_TILE_ENEMY_JUMPER_R:
    //     return 1; // Jumper type
    default:
        return 0; // Default to walker
    }
}

// ============================================================================
// ENEMY DATA EXTRACTION
// ============================================================================

void extract_enemy_data(void) BANKED
{
    UBYTE enemy_count = 0;
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }

    // Scan all rows for enemies (12, 14, 16, 18) - ensure we cover the full map width
    for (UBYTE row = 12; row <= 18; row += 2)
    {
        for (UBYTE col = 2; col < 22; col++) // Covers columns 0-19 in game coordinates
        {
            UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, row)];
            UBYTE tile_type = get_tile_type(tile);
            if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < MAX_ENEMIES)
            {
                current_level_code.enemy_positions[enemy_count] = col - 2; // 0-based column

                // Set direction bit (left = 1, right = 0)
                if (tile_type == BRUSH_TILE_ENEMY_L)
                {
                    current_level_code.enemy_directions |= (1 << enemy_count);
                }

                // Detect and store enemy type (walker=0, jumper=1, etc.)
                UBYTE enemy_type = detect_enemy_type(tile_type);
                if (enemy_type == 1) // If jumper
                {
                    current_level_code.enemy_types |= (1 << enemy_count);
                }

                enemy_count++;
            }
        }
    }
}

// ============================================================================
// ENEMY ENCODING FUNCTIONS
// ============================================================================

// Encode enemy positions using a checksum-based approach
UBYTE encode_enemy_positions(void) BANKED
{
    UBYTE checksum = 0;
    UBYTE enemy_count = 0;

    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
            // Use position value directly in checksum, avoid XOR cancellation
            checksum += (current_level_code.enemy_positions[i] * (i + 1)) & 0x1F;
        }
    }

    // Combine enemy count (3 bits) with position checksum (5 bits)
    UBYTE result = ((enemy_count & 0x07) << 5) | (checksum & 0x1F);

    // Ensure result fits in 0-34 range for display
    if (result > 34)
        result = result % 35;
    return result;
}

// Pack individual enemy positions into a compact format
UBYTE encode_enemy_details_1(void) BANKED
{
    UBYTE result = 0;
    UBYTE shift = 0;

    // Pack first 3 enemy positions (each needs ~5 bits for 0-19)
    for (UBYTE i = 0; i < 3; i++)
    {
        if (i >= MAX_ENEMIES)
            break;
        if (shift > 30)
            break;
        if (current_level_code.enemy_positions[i] != 255)
        {
            // Reduce precision: map 0-19 to 0-15 for better packing
            UBYTE compressed_pos = (current_level_code.enemy_positions[i] * 15) / 19;
            result += (compressed_pos & 0x0F) << (shift % 8);
            shift += 4;
        }
    }

    if (result > 34)
        result = result % 35;
    return result;
}

// Pack remaining enemy data
UBYTE encode_enemy_details_2(void) BANKED
{
    UBYTE result = 0;

    // Combine directions (6 bits) and types (6 bits) - but we only have 8 bits to work with
    // So we'll prioritize directions and use remaining space for types
    result = (current_level_code.enemy_directions & 0x3F); // 6 bits for directions

    // Add first 2 bits of enemy types if space allows
    if ((current_level_code.enemy_types & 0x03) > 0)
    {
        result |= ((current_level_code.enemy_types & 0x03) << 6);
    }

    if (result > 34)
        result = result % 35;
    return result;
}

// Original function name for compatibility
UBYTE encode_enemy_bitmask(void) BANKED
{
    return encode_enemy_positions();
}

UBYTE encode_enemy_directions(void) BANKED
{
    return encode_enemy_details_2();
}

// ============================================================================
// ENEMY PATTERN SYSTEM FUNCTIONS
// ============================================================================

// Extract enemy pattern for a specific block row
UBYTE extract_enemy_pattern_for_block_row(UBYTE row) BANKED
{
    // This function would analyze enemies in a specific row and return a pattern ID
    // Implementation depends on the specific pattern system requirements

    if (row >= 4)
        return 0; // Invalid row

    // Count enemies in this row and determine pattern
    UBYTE enemy_count = 0;
    UBYTE positions[MAX_ENEMIES];

    // Calculate the actual Y coordinate for this block row
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

    // Scan for enemies in this row
    for (UBYTE col = 2; col < 22; col++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, actual_y)];
        UBYTE tile_type = get_tile_type(tile);

        if ((tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R) && enemy_count < MAX_ENEMIES)
        {
            positions[enemy_count] = col - 2; // 0-based column
            enemy_count++;
        }
    }

    // Return a simple pattern based on enemy count
    return enemy_count;
}

// Apply enemy pattern to a specific block row
void apply_enemy_pattern_to_block_row(UBYTE row, UBYTE pattern_id, UBYTE direction_mask) BANKED
{
    if (row >= 4 || pattern_id >= sizeof(ENEMY_PATTERNS) / sizeof(ENEMY_PATTERNS[0]))
        return;

    const enemy_pattern_t *pattern = &ENEMY_PATTERNS[pattern_id];
    UBYTE actual_y = PLATFORM_Y_MIN + row * SEGMENT_HEIGHT;

    // Clear existing enemies in this row first
    for (UBYTE col = 2; col < 22; col++)
    {
        UBYTE tile = sram_map_data[METATILE_MAP_OFFSET(col, actual_y)];
        UBYTE tile_type = get_tile_type(tile);

        if (tile_type == BRUSH_TILE_ENEMY_L || tile_type == BRUSH_TILE_ENEMY_R)
        {
            replace_meta_tile(col, actual_y, 0, 1); // Clear enemy
        }
    }

    // Place new enemies according to pattern
    for (UBYTE i = 0; i < pattern->count && i < MAX_ENEMIES; i++)
    {
        if (pattern->positions[i] != 255)
        {
            UBYTE col = pattern->positions[i] + 2; // Convert to tile coordinates
            UBYTE direction_bit = (direction_mask >> i) & 1;
            UBYTE enemy_tile = direction_bit ? BRUSH_TILE_ENEMY_L : BRUSH_TILE_ENEMY_R;

            replace_meta_tile(col, actual_y, enemy_tile, 1);
        }
    }
}

// ============================================================================
// VALIDATION AND TESTING
// ============================================================================

// Diagnostic function to check enemy encoding integrity
void validate_enemy_encoding(void) BANKED
{
    UBYTE active_enemies = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            active_enemies++;
        }
    }

    // Could add validation logic here to ensure encoding/decoding works
    // For now, just count active enemies for debugging
}

// Initialize test enemy patterns
void init_test_enemy_patterns(void) BANKED
{
    // Set up some test enemy patterns for development/testing
    current_level_code.enemy_positions[0] = 5;
    current_level_code.enemy_positions[1] = 10;
    current_level_code.enemy_positions[2] = 15;
    current_level_code.enemy_directions = 0b101; // First and third face left
    current_level_code.enemy_types = 0b010;      // Second is a jumper
}

// Test cycling through enemy patterns
void test_cycle_enemy_patterns(void) BANKED
{
    // Cycle through different enemy patterns for testing
    static UBYTE current_pattern = 0;

    current_pattern = (current_pattern + 1) % (sizeof(ENEMY_PATTERNS) / sizeof(ENEMY_PATTERNS[0]));

    // Apply the pattern to row 0
    apply_enemy_pattern_to_block_row(0, current_pattern, 0b101010);
}

// Test cycling through enemy directions
void test_cycle_enemy_directions(void) BANKED
{
    // Flip all enemy directions for testing
    current_level_code.enemy_directions = ~current_level_code.enemy_directions;
}

// ============================================================================
// DEBUG FUNCTIONS
// ============================================================================

void test_enemy_encoding(void) BANKED
{
    init_level_code();

    // Test data - including bottom-right corner enemies
    current_level_code.enemy_positions[0] = 5;   // Middle column
    current_level_code.enemy_positions[1] = 12;  // Right side
    current_level_code.enemy_positions[2] = 18;  // Near bottom-right corner
    current_level_code.enemy_positions[3] = 19;  // Bottom-right corner
    current_level_code.enemy_positions[4] = 0;   // Left edge
    current_level_code.enemy_positions[5] = 255; // Empty slot

    // Test directions (left=1, right=0)
    current_level_code.enemy_directions = 0b010110; // Enemies 1,2,4 face left

    // Test enemy types (walker=0, jumper=1)
    current_level_code.enemy_types = 0b001100; // Enemies 2,3 are jumpers
    current_level_code.player_column = 3;

    force_complete_level_code_display();
}

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

void vm_test_enemy_encoding(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    test_enemy_encoding();
}

void vm_get_enemy_info(SCRIPT_CTX *THIS) BANKED
{
    // Useful for debugging - stores enemy info in script variables
    update_complete_level_code();

    // Count active enemies
    UBYTE enemy_count = 0;
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        if (current_level_code.enemy_positions[i] != 255)
        {
            enemy_count++;
        }
    }

    // Store results for script access
    *(UWORD *)VM_REF_TO_PTR(FN_ARG0) = enemy_count;
    *(UWORD *)VM_REF_TO_PTR(FN_ARG1) = encode_enemy_positions();
    *(UWORD *)VM_REF_TO_PTR(FN_ARG2) = current_level_code.enemy_directions;
    *(UWORD *)VM_REF_TO_PTR(FN_ARG3) = current_level_code.enemy_types;
}

void vm_init_test_enemy_patterns(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;
    init_test_enemy_patterns();
}
