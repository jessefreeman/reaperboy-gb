#pragma bank 254

#include <gbdk/platform.h>
#include "paint.h"
#include "paint_vm.h"
#include "paint_core.h"
#include "paint_ui.h"
#include "paint_entity.h"
#include "vm.h"
#include "meta_tiles.h"
#include "code_level_core.h"
#include "code_platform_system.h"
#include "code_player_system.h"
#include "code_enemy_system.h"

// Forward declarations
extern void deactivate_actor(actor_t *actor) BANKED;
extern actor_t actors[];
extern UWORD script_memory[];
extern void reset_enemy_pool(void) BANKED;
extern void init_column_platform_tracking(void) BANKED;
extern UBYTE is_map_empty(void) BANKED;
extern void init_default_level_code(void) BANKED;
extern void update_complete_level_code(void) BANKED;
extern void force_complete_level_code_display(void) BANKED;
extern void update_player_actor_position(void) BANKED;
extern void restore_enemy_actors_from_level_code(void) BANKED;
extern void init_tilemap_editor_from_memory(void) BANKED;
extern void save_level_code_to_variables(void) BANKED;

// ============================================================================
// VM WRAPPER FUNCTIONS
// ============================================================================

/**
 * @brief Set up actor IDs for paint operations from VM context
 *
 * This function initializes the actor IDs used for painting entities
 * by reading them from VM arguments.
 *
 * @param THIS Script context
 */
void vm_setup_paint_actors(SCRIPT_CTX *THIS) BANKED
{
    paint_player_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    paint_exit_id = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint_enemy_ids[0] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG2);
    paint_enemy_ids[1] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG3);
    paint_enemy_ids[2] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG4);
    paint_enemy_ids[3] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG5);
    paint_enemy_ids[4] = *(UBYTE *)VM_REF_TO_PTR(FN_ARG6);

    // Reset the FIFO enemy pool
    reset_enemy_pool();

    // Disable all assigned actors to prepare for level loading
    deactivate_actor(&actors[paint_player_id]);
    deactivate_actor(&actors[paint_exit_id]);

    // Deactivate enemy actors
    for (UBYTE i = 0; i < 5; i++)
    {
        deactivate_actor(&actors[paint_enemy_ids[i]]);
    }
}

/**
 * @brief Paint at a position from VM context
 *
 * This function triggers a paint operation at the given coordinates,
 * delegating to the main paint function.
 *
 * @param THIS Script context
 */
void vm_paint(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    paint(x, y);
}

/**
 * @brief Get brush tile position state from VM context
 *
 * This function retrieves the brush state for a position and
 * stores it in script memory.
 *
 * @param THIS Script context
 */
void vm_get_brush_tile_pos(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_state(x, y);
}

/**
 * @brief Get brush preview tile state from VM context
 *
 * This function is similar to vm_get_brush_tile_pos but intended
 * for preview purposes.
 *
 * @param THIS Script context
 */
void vm_get_brush_preview_tile(SCRIPT_CTX *THIS) BANKED
{
    UBYTE x = *(UBYTE *)VM_REF_TO_PTR(FN_ARG0);
    UBYTE y = *(UBYTE *)VM_REF_TO_PTR(FN_ARG1);
    script_memory[*(int16_t *)VM_REF_TO_PTR(FN_ARG2)] = get_brush_tile_state(x, y);
}

/**
 * @brief Enable the editor system from VM context
 *
 * This function initializes the paint editor and prepares
 * for level editing operations.
 *
 * @param THIS Script context
 */
void vm_enable_editor(SCRIPT_CTX *THIS) BANKED
{
    (void)THIS;

    // Only deactivate the specific actors that the tilemap editor will be using
    // This prevents disrupting other actors in the scene

    // Deactivate player actor
    deactivate_actor(&actors[paint_player_id]);

    // Deactivate exit actor
    deactivate_actor(&actors[paint_exit_id]);

    // Deactivate enemy actors
    for (UBYTE i = 0; i < 5; i++)
    {
        deactivate_actor(&actors[paint_enemy_ids[i]]);
    }

    // Initialize enemy validation system
    init_enemy_system();

    // Initialize player position tracking
    update_valid_player_positions();

    // Reset the FIFO enemy pool
    reset_enemy_pool();

    // Initialize the player position tracking system
    init_column_platform_tracking();

    // Check if map is empty and initialize with default level if needed
    if (is_map_empty())
    {
        init_default_level_code();
    }
    else
    {
        // Always extract the current level state from the tilemap first
        // This ensures the level code is in sync with what's actually on screen
        update_complete_level_code();
        
        // Always restore player position and actors based on current level code
        update_player_actor_position();
    }

    // Always ensure the level code is displayed after initialization
    force_complete_level_code_display();
}
