#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "tile_utils.h"
#include "paint.h"
#include "paint_entity.h"
#include "code_persistence.h"

// External data declarations for cross-bank access
extern const UBYTE PATTERN_TILE_MAP[];
extern const UBYTE EXTENDED_PATTERN_TILE_MAP[];

// Forward declarations
void move_player_to_column(UBYTE x, UBYTE y) BANKED;

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================

// Main level code data structure instance
level_code_t current_level_code;

// Track external changes to level code display values
UBYTE level_code_display_values[LEVEL_CODE_CHARS_TOTAL];
UBYTE level_code_display_changed[LEVEL_CODE_CHARS_TOTAL];
UBYTE level_code_display_initialized = 0;

// ============================================================================
// SELECTIVE UPDATE SYSTEM - Prevents flicker by only updating changed chars
// ============================================================================

level_code_t previous_level_code;
UBYTE level_code_initialized = 0;

// Cache for encoded values to avoid recalculation (now 7 values for enemy data)
UBYTE previous_encoded_enemy_data[7];
UBYTE current_encoded_enemy_data[7];

// Bitmask to track which display positions need updating (24 bits max)
UWORD display_update_mask_low = 0;  // Bits 0-15
UBYTE display_update_mask_high = 0; // Bits 16-23

// Mark a display position for update
void mark_display_position_for_update(UBYTE position) BANKED
{
    if (position < 16)
    {
        display_update_mask_low |= (1 << position);
    }
    else if (position < 24)
    {
        display_update_mask_high |= (1 << (position - 16));
    }
}

// Check if a display position needs updating
UBYTE display_position_needs_update(UBYTE position) BANKED
{
    if (position < 16)
    {
        return (display_update_mask_low & (1 << position)) != 0;
    }
    else if (position < 24)
    {
        return (display_update_mask_high & (1 << (position - 16))) != 0;
    }
    return 0;
}

// Clear all update flags
void clear_display_update_flags(void) BANKED
{
    display_update_mask_low = 0;
    display_update_mask_high = 0;
}

// Calculate display position for a given character index
void get_display_position(UBYTE char_index, UBYTE *x, UBYTE *y) BANKED
{
    UBYTE display_x = LEVEL_CODE_START_X;
    UBYTE display_y = LEVEL_CODE_START_Y;

    // Layout: 3 blocks of 4 characters per row, with spaces between blocks
    // Row 0: 0000 0000 0000 (characters 0-11)
    // Row 1: 0000 0000 0000 (characters 12-23)
    // Display format: "0000 0000 0000" per row

    UBYTE row = char_index / 12; // 12 characters per row
    UBYTE col = char_index % 12; // Position within row

    // Calculate column position with spaces between blocks
    UBYTE block = col / 4;        // Which block (0, 1, or 2)
    UBYTE pos_in_block = col % 4; // Position within block (0-3)

    display_x += block * 5 + pos_in_block; // 5 = 4 chars + 1 space
    display_y += row;

    *x = display_x;
    *y = display_y;
}

// Compare current level code with previous and mark changes
void detect_level_code_changes(void) BANKED
{
    // STEP 1: Process any external changes to level code display first
    process_level_code_external_changes();

    if (!level_code_initialized)
    {
        // First time - mark everything for update
        for (UBYTE i = 0; i < LEVEL_CODE_CHARS_TOTAL; i++)
        {
            mark_display_position_for_update(i);
        }

        // Cache initial encoded enemy values (now includes all 7 enemy characters)
        current_encoded_enemy_data[0] = encode_enemy_positions();  // Character 17
        current_encoded_enemy_data[1] = encode_enemy_details_1();  // Character 18
        current_encoded_enemy_data[2] = encode_enemy_details_2();  // Character 19
        current_encoded_enemy_data[3] = encode_enemy_position_4(); // Character 20
        current_encoded_enemy_data[4] = encode_enemy_position_5(); // Character 21
        current_encoded_enemy_data[5] = encode_odd_mask_value();   // Character 22
        current_encoded_enemy_data[6] = encode_enemy_directions(); // Character 23

        // Copy to previous cache
        for (UBYTE i = 0; i < 7; i++)
        {
            previous_encoded_enemy_data[i] = current_encoded_enemy_data[i];
        }

        level_code_initialized = 1;
    }
    else
    {
        // Check platform patterns (positions 0-15)
        for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
        {
            if (current_level_code.platform_patterns[i] != previous_level_code.platform_patterns[i])
            {
                mark_display_position_for_update(i);
            }
        }

        // Update current encoded enemy data (all 7 characters)
        current_encoded_enemy_data[0] = encode_enemy_positions();  // Character 17
        current_encoded_enemy_data[1] = encode_enemy_details_1();  // Character 18
        current_encoded_enemy_data[2] = encode_enemy_details_2();  // Character 19
        current_encoded_enemy_data[3] = encode_enemy_position_4(); // Character 20
        current_encoded_enemy_data[4] = encode_enemy_position_5(); // Character 21
        current_encoded_enemy_data[5] = encode_odd_mask_value();   // Character 22
        current_encoded_enemy_data[6] = encode_enemy_directions(); // Character 23

        // Compare with previous encoded values (positions 17-23)
        for (UBYTE i = 0; i < 7; i++)
        {
            if (current_encoded_enemy_data[i] != previous_encoded_enemy_data[i])
            {
                mark_display_position_for_update(17 + i); // Positions 17-23
            }
        }

        // Update previous cache
        for (UBYTE i = 0; i < 7; i++)
        {
            previous_encoded_enemy_data[i] = current_encoded_enemy_data[i];
        }
    }

    // Update the cache
    previous_level_code = current_level_code;
}

// ============================================================================
// CORE LEVEL CODE FUNCTIONS
// ============================================================================

// Main level code initialization
void init_level_code(void) BANKED
{
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        current_level_code.platform_patterns[i] = 0;
    }
    for (UBYTE i = 0; i < MAX_ENEMIES; i++)
    {
        current_level_code.enemy_positions[i] = 255;
    }
    current_level_code.enemy_directions = 0;
    current_level_code.enemy_types = 0;
    current_level_code.player_column = 0;

    // Initialize valid player positions
    update_valid_player_positions();

    // Ensure player starts at a valid position
    if (valid_player_count > 0 && !is_valid_player_position(current_level_code.player_column))
    {
        current_level_code.player_column = valid_player_columns[0];
    }
}

// Unified update function that coordinates all subsystems
void update_complete_level_code(void) BANKED
{
    extract_platform_data();
    extract_enemy_data();
    extract_player_data();
}

// Update player actor position based on current level code
void update_player_actor_position(void) BANKED
{
    // Convert column position to tile coordinates (add 2 for offset)
    UBYTE player_x = current_level_code.player_column + 2;
    
    // For edit mode: Place the player marker tile on row 11 at the correct column
    // This ensures the editor shows the player position correctly
    replace_meta_tile(player_x, 11, TILE_PLAYER, 1);
    
    // For gameplay: Move the player actor to the top (row 0) at the correct column
    // This ensures the player starts at the right position when playing
    move_player_to_column(player_x, 0);
    
    // Position the exit sprite relative to the player marker
    position_exit_for_player(player_x, 11);
}

// ============================================================================
// DISPLAY SYSTEM FUNCTIONS
// ============================================================================

// Selective level code display - prevents flicker by only updating changed chars
void display_selective_level_code(void) BANKED
{
    update_complete_level_code();
    detect_level_code_changes();

    // Only update characters that have changed
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (display_position_needs_update(i))
        {
            get_display_position(i, &display_x, &display_y);
            display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        }
    }

    // Display player column (position 16)
    if (display_position_needs_update(16))
    {
        get_display_position(16, &display_x, &display_y);
        display_char_at_position(current_level_code.player_column, display_x, display_y);
    }

    // Display enemy data (positions 17-23) - NEW SIMPLIFIED ENCODING
    UBYTE enemy_data[] = {
        encode_enemy_positions(),  // Position 17: Enemy 0 position (POS41)
        encode_enemy_details_1(),  // Position 18: Enemy 1 position (POS41)
        encode_enemy_details_2(),  // Position 19: Enemy 2 position (POS41)
        encode_enemy_position_4(), // Position 20: Enemy 3 position (POS41)
        encode_enemy_position_5(), // Position 21: Enemy 4 position (POS41)
        encode_odd_mask_value(),   // Position 22: Odd column parity mask (BASE32)
        encode_enemy_directions()  // Position 23: Direction mask (BASE32)
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            UBYTE enemy_char = get_enemy_display_char(enemy_data[i], pos);
            display_char_at_position(enemy_char, display_x, display_y);
        }
    }

    // Clear update flags after updating
    clear_display_update_flags();
}

// Fast selective update that doesn't re-extract all data
void display_selective_level_code_fast(void) BANKED
{
    // DON'T call update_complete_level_code() - assume data is already correct
    detect_level_code_changes();

    // Only update characters that have changed
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        if (display_position_needs_update(i))
        {
            get_display_position(i, &display_x, &display_y);
            display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
        }
    }

    // Display player column (position 16)
    if (display_position_needs_update(16))
    {
        get_display_position(16, &display_x, &display_y);
        // Pass the numeric value directly to display_char_at_position
        display_char_at_position(current_level_code.player_column, display_x, display_y);
    }

    // Display enemy data (positions 17-23) - NEW SIMPLIFIED ENCODING
    UBYTE enemy_data[] = {
        encode_enemy_positions(),  // Position 17: Enemy 0 position (POS41)
        encode_enemy_details_1(),  // Position 18: Enemy 1 position (POS41)
        encode_enemy_details_2(),  // Position 19: Enemy 2 position (POS41)
        encode_enemy_position_4(), // Position 20: Enemy 3 position (POS41)
        encode_enemy_position_5(), // Position 21: Enemy 4 position (POS41)
        encode_odd_mask_value(),   // Position 22: Odd column parity mask (BASE32)
        encode_enemy_directions()  // Position 23: Direction mask (BASE32)
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        if (display_position_needs_update(pos))
        {
            get_display_position(pos, &display_x, &display_y);
            UBYTE enemy_char = get_enemy_display_char(enemy_data[i], pos);
            display_char_at_position(enemy_char, display_x, display_y);
        }
    }

    // Clear update flags after updating
    clear_display_update_flags();
}

// Force a complete redraw (useful for initialization)
void force_complete_level_code_display(void) BANKED
{
    update_complete_level_code();
    clear_level_code_display();

    // Declare display_x and display_y for use in this function
    UBYTE display_x, display_y;

    // Display 16 platform patterns (positions 0-15)
    for (UBYTE i = 0; i < TOTAL_BLOCKS; i++)
    {
        get_display_position(i, &display_x, &display_y);
        display_pattern_char(current_level_code.platform_patterns[i], display_x, display_y);
    }

    // Display player column (position 16)
    get_display_position(16, &display_x, &display_y);
    // Pass the numeric value directly to display_char_at_position
    display_char_at_position(current_level_code.player_column, display_x, display_y);

    // Display enemy data (positions 17-23) - NEW SIMPLIFIED ENCODING
    UBYTE enemy_data[] = {
        encode_enemy_positions(),  // Position 17: Enemy 0 position (POS41)
        encode_enemy_details_1(),  // Position 18: Enemy 1 position (POS41)
        encode_enemy_details_2(),  // Position 19: Enemy 2 position (POS41)
        encode_enemy_position_4(), // Position 20: Enemy 3 position (POS41)
        encode_enemy_position_5(), // Position 21: Enemy 4 position (POS41)
        encode_odd_mask_value(),   // Position 22: Odd column parity mask (BASE32)
        encode_enemy_directions()  // Position 23: Direction mask (BASE32)
    };

    for (UBYTE i = 0; i < 7; i++)
    {
        UBYTE pos = 17 + i; // Positions 17-23
        get_display_position(pos, &display_x, &display_y);
        UBYTE enemy_char = get_enemy_display_char(enemy_data[i], pos);
        display_char_at_position(enemy_char, display_x, display_y);
    }

    // Initialize the cache after complete redraw
    previous_level_code = current_level_code;
    level_code_initialized = 1;
    clear_display_update_flags();
}

// Main level code display function
void display_complete_level_code(void) BANKED
{
    // Sync display values with current game state
    sync_level_code_display_values();

    // Use selective update to prevent flicker
    display_selective_level_code();
}

// ============================================================================
// CHARACTER DISPLAY FUNCTIONS
// ============================================================================

// This function is kept for backward compatibility but now simply returns the value directly
// since display_char_at_position handles the mapping to metatile ID
UBYTE get_extended_display_char(UBYTE value) BANKED
{
    // Simply validate the range and return the value
    if (value <= 48) // Allow extended range
    {
        return value;
    }
    return 0; // Default for invalid values
}

// Helper function to validate and prepare enemy values for display
UBYTE get_enemy_display_char(UBYTE value, UBYTE char_position) BANKED
{
    // For enemy position characters (17-21), use POS41 system (0-40)
    if (char_position >= 17 && char_position <= 21)
    {
        // Validate POS41 range
        if (value > 40)
            value = 0; // Safety check
    }
    // For mask characters (22-23), use BASE32 system (0-31)
    else if (char_position == 22 || char_position == 23)
    {
        // Validate BASE32 range
        if (value > 31)
            value = 0; // Safety check
    }

    return value; // Return validated value directly
}

// Helper function to convert POS41 value directly to tile ID
UBYTE pos41_value_to_tile_id(UBYTE value) BANKED
{
    // Use the same mapping logic as display_char_at_position
    if (value > 40)
        return 48; // Default to '0' tile

    // Values 0-9 map to tiles 48-57
    if (value < 10)
    {
        return 48 + value;
    }
    // Values 10-35 map to tiles 58-83 (A-Z)
    else if (value < 36)
    {
        return 58 + (value - 10);
    }
    // Special values 36-40 map to tiles 84-88 (!, @, #, $, %)
    else if (value == 36)
    {
        return 84;
    }
    else if (value == 37)
    {
        return 85;
    }
    else if (value == 38)
    {
        return 86;
    }
    else if (value == 39)
    {
        return 87;
    }
    else if (value == 40)
    {
        return 88;
    }

    return 48; // Default to '0' tile
}

// Helper function to convert BASE32 value directly to tile ID
UBYTE base32_value_to_tile_id(UBYTE value) BANKED
{
    // BASE32 uses the same mapping for 0-31 as POS41 does for 0-31
    if (value > 31)
        return 48; // Default to '0' tile

    // Values 0-9 map to tiles 48-57
    if (value < 10)
    {
        return 48 + value;
    }
    // Values 10-31 map to tiles 58-79 (first 22 characters of A-Z)
    else if (value < 32)
    {
        return 58 + (value - 10);
    }

    return 48; // Default to '0' tile
}

void display_char_at_position(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    // Direct mapping from value to metatile ID
    UBYTE tile_index;

    // Values 0-9 map to tiles 48-57
    if (value < 10)
    {
        tile_index = 48 + value;
    }
    // Values 10-35 map to tiles 58-83 (A-Z)
    else if (value < 36)
    {
        tile_index = 58 + (value - 10);
    }
    // Special values 36-40 map to tiles 84-88 (!, @, #, $, %)
    else if (value == 36)
    {
        tile_index = 84;
    }
    else if (value == 37)
    {
        tile_index = 85;
    }
    else if (value == 38)
    {
        tile_index = 86;
    }
    else if (value == 39)
    {
        tile_index = 87;
    }
    else if (value == 40)
    {
        tile_index = 88;
    }
    else if (value <= 48) // Extended range support
    {
        // For extended values, we'll still map them to visible tiles
        // This keeps debug functionality
        tile_index = 89 + (value - 41); // Use additional tiles if available
    }
    else
    {
        // Default fallback
        tile_index = 48; // Default to '0' tile
    }

    replace_meta_tile(x, y, tile_index, 1);
}

void display_pattern_char(UBYTE value, UBYTE x, UBYTE y) BANKED
{
    // Pass the value directly to display_char_at_position
    // which now handles the direct mapping to metatile ID
    display_char_at_position(value, x, y);
}

void clear_level_code_display(void) BANKED
{
    // Only clear the actual level code character positions, not the spaces/background
    for (UBYTE i = 0; i < LEVEL_CODE_CHARS_TOTAL; i++)
    {
        UBYTE display_x, display_y;
        get_display_position(i, &display_x, &display_y);
        replace_meta_tile(display_x, display_y, 0, 1);
    }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// Convert display position (x,y) to character index (0-23)
UBYTE get_char_index_from_display_position(UBYTE x, UBYTE y) BANKED
{
    // Check if position is within the level code display area
    if (x < LEVEL_CODE_START_X || y < LEVEL_CODE_START_Y || y >= LEVEL_CODE_START_Y + 2)
    {
        return 255; // Invalid position
    }

    UBYTE rel_x = x - LEVEL_CODE_START_X;
    UBYTE rel_y = y - LEVEL_CODE_START_Y;

    // Layout: 3 blocks of 4 characters per row, with spaces between blocks
    // Row format: "0000 0000 0000" (positions 0,1,2,3, 5,6,7,8, 10,11,12,13)

    // Check if we're on a space (positions 4, 9, 14)
    if (rel_x == 4 || rel_x == 9 || rel_x == 14)
    {
        return 255; // Space position, not a character
    }

    // Calculate which block (0, 1, or 2)
    UBYTE block;
    UBYTE pos_in_block;

    if (rel_x < 4)
    {
        block = 0;
        pos_in_block = rel_x;
    }
    else if (rel_x < 9)
    {
        block = 1;
        pos_in_block = rel_x - 5; // Skip the space at position 4
    }
    else if (rel_x < 14)
    {
        block = 2;
        pos_in_block = rel_x - 10; // Skip spaces at positions 4 and 9
    }
    else
    {
        return 255; // Beyond valid range
    }

    // Calculate final character index
    UBYTE char_index = rel_y * 12 + block * 4 + pos_in_block;

    return (char_index < LEVEL_CODE_CHARS_TOTAL) ? char_index : 255;
}

// ============================================================================
// LEVEL CODE EDITING FUNCTIONS
// ============================================================================

// Handle when a level code character is edited by the user
void handle_level_code_character_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    if (char_index >= 17 && char_index <= 23)
    {
        // Enemy data characters - route to enemy system handler
        handle_enemy_data_edit(char_index, new_value);

        // Force update of the display to show changes
        mark_display_position_for_update(char_index);
        display_selective_level_code_fast();
    }
    else if (char_index < 16)
    {
        // Platform pattern characters (0-15)
        if (new_value <= 34) // Valid platform pattern range
        {
            current_level_code.platform_patterns[char_index] = new_value;

            // Update the tilemap to reflect the new platform pattern
            reconstruct_tilemap_from_level_code();

            // Update display
            mark_display_position_for_update(char_index);
            display_selective_level_code_fast();
        }
    }
    else if (char_index == 16)
    {
        // Player position character
        if (new_value <= 40) // Valid player position range
        {
            current_level_code.player_column = new_value;

            // Move player actor to new position
            update_player_actor_position();

            // Update display
            mark_display_position_for_update(char_index);
            display_selective_level_code_fast();
        }
    }
}

// ============================================================================
// DIRECT LEVEL CODE EDITING VM FUNCTIONS
// ============================================================================

// ============================================================================
// BIDIRECTIONAL LEVEL CODE SYSTEM
// ============================================================================

// External changes to level code display values are tracked above in globals

// Mark a level code position as externally changed
void mark_level_code_position_changed(UBYTE position, UBYTE new_value) BANKED
{
    if (position >= LEVEL_CODE_CHARS_TOTAL)
        return;

    level_code_display_values[position] = new_value;
    level_code_display_changed[position] = 1;
}

// Check for external changes and apply them to game state
void process_level_code_external_changes(void) BANKED
{
    for (UBYTE i = 0; i < LEVEL_CODE_CHARS_TOTAL; i++)
    {
        if (level_code_display_changed[i])
        {
            // Apply the change to the appropriate system
            if (i >= 17 && i <= 23)
            {
                // Enemy data characters
                handle_enemy_data_edit(i, level_code_display_values[i]);
            }
            else if (i < 16)
            {
                // Platform pattern characters
                if (level_code_display_values[i] <= 34)
                {
                    current_level_code.platform_patterns[i] = level_code_display_values[i];
                    reconstruct_tilemap_from_level_code();
                }
            }
            else if (i == 16)
            {
                // Player position character
                if (level_code_display_values[i] <= 40)
                {
                    current_level_code.player_column = level_code_display_values[i];
                    update_player_actor_position();
                }
            }

            // Clear the change flag
            level_code_display_changed[i] = 0;

            // Force display update for this position
            mark_display_position_for_update(i);
        }
    }
}

// Synchronize level_code_display_values with current game state
void sync_level_code_display_values(void) BANKED
{
    // Sync enemy position characters (17-21)
    level_code_display_values[17] = encode_enemy_positions();
    level_code_display_values[18] = encode_enemy_details_1();
    level_code_display_values[19] = encode_enemy_details_2();
    level_code_display_values[20] = encode_enemy_position_4();
    level_code_display_values[21] = encode_enemy_position_5();

    // Sync enemy mask characters (22-23)
    level_code_display_values[22] = encode_odd_mask_value();
    level_code_display_values[23] = encode_enemy_directions();

    // Sync platform patterns (0-15)
    for (UBYTE i = 0; i < 16; i++)
    {
        level_code_display_values[i] = current_level_code.platform_patterns[i];
    }

    // Sync player position (16)
    level_code_display_values[16] = current_level_code.player_column;
}

// ============================================================================
// DIRECT ENEMY DATA EDITING FUNCTIONS
// ============================================================================

// Directly set enemy position (0-40) for a specific enemy (0-4)
// Enemy 0 position is at character 17, enemy 1 at 18, etc.
void set_enemy_position_direct(UBYTE enemy_index, UBYTE position) BANKED
{
    if (enemy_index > 4)
        return; // Invalid enemy index

    // Calculate the character index for this enemy's position
    UBYTE char_index = 17 + enemy_index;

    // Set the enemy position directly
    handle_enemy_data_edit(char_index, position);
}

// Directly set enemy direction (0-31) for all enemies at once
// 0 = all facing right, 1 = all facing left
void set_all_enemies_direction(UBYTE direction) BANKED
{
    if (direction > 1)
        return; // Invalid direction

    // Set the direction mask directly
    handle_enemy_data_edit(23, direction);
}

// ============================================================================
// EXAMPLES OF DIRECT ENEMY DATA EDITING
// ============================================================================

// Example: Place enemy 0 at position 11 (row 1, column 0)
void example_place_enemy_0_at_11(void) BANKED
{
    set_enemy_position_direct(0, 11);
}

// Example: Increment enemy 0 position (wraps around 0-40)
void example_increment_enemy_0_position(void) BANKED
{
    UBYTE current = encode_enemy_positions();
    UBYTE new_value = (current + 1) % 41;
    set_enemy_position_direct(0, new_value);
}

// Example: Set all enemies to face left
void example_set_all_enemies_face_left(void) BANKED
{
    set_all_enemies_direction(1);
}

// Example: Set enemy 1 position to 22
void example_set_enemy_1_to_22(void) BANKED
{
    set_enemy_position_direct(1, 22);
}

// Example: Decrement enemy 2 position (wraps around 0-40)
void example_decrement_enemy_2_position(void) BANKED
{
    UBYTE current = encode_enemy_positions();
    UBYTE new_value = (current == 0) ? 40 : (current - 1);
    set_enemy_position_direct(2, new_value);
}

// Example: Set all enemies to face right
void example_set_all_enemies_face_right(void) BANKED
{
    set_all_enemies_direction(0);
}

// Optimized display function for enemy characters
void display_enemy_char_at_position(UBYTE value, UBYTE char_position, UBYTE x, UBYTE y) BANKED
{
    UBYTE tile_id;

    if (char_position >= 17 && char_position <= 21) // POS41 characters
    {
        tile_id = pos41_value_to_tile_id(value);
    }
    else if (char_position == 22 || char_position == 23) // BASE32 characters
    {
        tile_id = base32_value_to_tile_id(value);
    }
    else
    {
        tile_id = 48; // Default to '0'
    }

    replace_meta_tile(x, y, tile_id, 1);
}

// Initialize tilemap editor with level data from memory
void init_tilemap_editor_from_memory(void) BANKED
{
    // This function should be called when the tilemap editor loads
    // It restores the level state from memory and ensures proper positioning
    
    // First restore the complete level from memory
    restore_level_from_memory();
    
    // Extract current player data to ensure it's in sync
    extract_player_data();
    
    // Update player position to ensure both marker and actor are positioned correctly
    update_player_actor_position();
    
    // Force display update to show current state
    force_complete_level_code_display();
}

// VM function to initialize tilemap editor from memory
void vm_init_tilemap_editor_from_memory(SCRIPT_CTX *THIS) BANKED
{
    init_tilemap_editor_from_memory();
}

// ============================================================================
