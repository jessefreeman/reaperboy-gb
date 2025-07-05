#pragma bank 254

/*
 * REFACTORING COMPLETE
 * ===================
 *
 * This file has been refactored into multiple modules:
 *
 * 1. paint_core.c/.h
 *    - Main paint() function
 *    - Core utility functions
 *    - Constants and shared definitions
 *
 * 2. paint_platform.c/.h
 *    - Platform validation and placement logic
 *    - Platform reconstruction algorithm
 *    - Platform length calculation
 *
 * 3. paint_entity.c/.h
 *    - Player and exit positioning
 *    - Enemy management (FIFO pool implementation)
 *    - Actor movement functions
 *
 * 4. paint_ui.c/.h
 *    - Brush state functions
 *    - Selector state management
 *    - UI feedback
 *
 * 5. paint_vm.c/.h
 *    - VM wrapper functions
 *    - Script interface
 *
 * This file is now a compatibility stub that includes all the modules.
 * New code should be added to the appropriate module based on its responsibility.
 */

#include <gbdk/platform.h>
#include "paint.h"

// This file is now an empty stub that serves as a compatibility layer.
// All functionality has been moved to dedicated modules.
