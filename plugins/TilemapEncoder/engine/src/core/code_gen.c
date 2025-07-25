#pragma bank 254

#include <gbdk/platform.h>
#include "vm.h"
#include "meta_tiles.h"
#include "code_gen.h"
#include "tile_utils.h"
#include "paint.h"

// Include the new modular headers
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"
#include "code_persistence.h"

// ############################################################################
// MAIN COORDINATOR - Reduced from original monolithic file
// ############################################################################
//
// This file now serves as a coordination layer between the modular systems
// and provides VM wrapper functions for GB Studio integration.
// ############################################################################

// ============================================================================
// COORDINATION FUNCTIONS - High-level operations that use multiple systems
// ============================================================================

// Complete level regeneration - coordinates all systems
void regenerate_complete_level(void) BANKED
{
    // Clear existing display
    clear_level_code_display();

    // Update valid player positions
    update_valid_player_positions();

    // Display the complete updated level code
    update_complete_level_code();
}

// Handle platform changes - coordinates platform and player systems
void handle_platform_change(UBYTE block_index, UBYTE pattern_id) BANKED
{
    // Apply the platform pattern
    apply_pattern_with_brush_logic(block_index, pattern_id);

    // Update related platform codes
    update_all_affected_block_codes(block_index);

    // Update player position if needed
    update_exit_position_after_platform_change();
}

// Handle level code character edits - coordinates all affected systems
void handle_level_code_edit(UBYTE char_index, UBYTE new_value) BANKED
{
    // Use the general update function from persistence module
    // This will call apply_pattern_with_brush_logic() which uses paint() calls
    // The paint() calls automatically handle level code and display updates
    update_level_code_from_character_edit(char_index, new_value);

    // No additional display updates needed - paint() calls handle everything
}

// Zone management functions (placeholder implementations)
void draw_segment_ids(void) BANKED
{
    // Placeholder: draw zone IDs for debugging
    // This can be implemented later if needed
}

void update_zone_code(UBYTE zone_index) BANKED
{
    // Placeholder: update code for specific zone
    zone_index; // Suppress warning
}

UBYTE get_zone_index_from_tile(UBYTE x, UBYTE y) BANKED
{
    // Calculate zone index from tile coordinates
    // Using simple block-based calculation
    if (x >= PLATFORM_X_MIN && x <= PLATFORM_X_MAX &&
        y >= PLATFORM_Y_MIN && y <= PLATFORM_Y_MAX)
    {
        UBYTE block_x = (x - PLATFORM_X_MIN) / SEGMENT_WIDTH;
        UBYTE block_y = (y - PLATFORM_Y_MIN) / SEGMENT_HEIGHT;
        return block_y * SEGMENTS_PER_ROW + block_x;
    }
    return 255; // Invalid position
}

// ============================================================================
// VM WRAPPER FUNCTIONS - Entry points for GB Studio scripts
// ============================================================================

// ============================================================================
// LEGACY COMPATIBILITY FUNCTIONS - Maintain backward compatibility
// ============================================================================

// // Legacy function names that may be called from existing scripts
// void display_level_code(void) BANKED
// {
//     update_complete_level_code();
// }

// void save_current_level_code(void) BANKED
// {
//     save_level_code_to_variables();
// }

// void load_saved_level_code(void) BANKED
// {
//     load_level_code_from_variables();
//     regenerate_complete_level();
// }

// // Missing VM wrapper function
// void vm_draw_segment_ids(SCRIPT_CTX *THIS) BANKED
// {
//     THIS; // Suppress unreferenced parameter warning
//     draw_segment_ids();
// }
