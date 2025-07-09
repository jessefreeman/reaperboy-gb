# Integration Example

## Complete Game Integration

Here's how to integrate the test harness into your GB Studio game:

### 1. Add These Functions to Your Game Code

```c
#include "test_harness.h"

// Example implementation for a simple text display
void test_display_message(const char* message) BANKED {
    // Clear previous message area
    fill_bkg_rect(0, 0, 20, 2, 0);
    
    // Display the message at the top of the screen
    set_bkg_tiles(0, 0, strlen(message), 1, message);
    
    // Alternative: If you have a printf-like function
    // printf("%s\\n", message);
}

void test_clear_display(void) BANKED {
    // Clear the message area (top 2 rows)
    fill_bkg_rect(0, 0, 20, 2, 0);
}
```

### 2. Update Your Main Game Loop

```c
void main_loop(void) BANKED {
    // Handle input
    handle_input();
    
    // Update game logic
    update_game_state();
    
    // Update test harness (ADD THIS LINE)
    if (test_harness_is_active()) {
        test_harness_update();
    }
    
    // Render everything
    render_game();
    
    // Wait for next frame
    wait_vbl_done();
}
```

### 3. Example Test Script in GB Studio

Create a new script or add to an existing one:

```
1. Test: Start Test Suite
   - test_name: "Health System Test"
   - debug_enabled: ✓

2. Store Variable: V_PLAYER_HEALTH = 3

3. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Initial health"
   - delay: 60

4. Store Variable: V_PLAYER_HEALTH = 2

5. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 2
   - test_name: "Health after damage"
   - delay: 60

6. Test: End Test Suite
   - show_results: ✓
```

### 4. Expected Output

```
Starting tests...
PASS: Initial health
PASS: Health after damage
P:02 F:00
```

## Advanced Integration

### Custom Message Display

If you have a more sophisticated UI system:

```c
void test_display_message(const char* message) BANKED {
    // Use your UI system
    ui_show_notification(message, UI_NOTIFICATION_TEST);
    
    // Or use a dedicated test window
    test_window_show(message);
}

void test_clear_display(void) BANKED {
    ui_hide_notification(UI_NOTIFICATION_TEST);
    // or
    test_window_hide();
}
```

### Conditional Testing

You can wrap test calls in conditions:

```c
// In your game script
if (V_DEBUG_MODE == 1) {
    // Run tests only in debug mode
    Test: Start Test Suite
    // ... your tests ...
    Test: End Test Suite
}
```

## Troubleshooting

- **No messages appear**: Check your `test_display_message` implementation
- **Messages don't clear**: Check your `test_clear_display` implementation  
- **Tests don't run**: Make sure `test_harness_update()` is called in your main loop
- **Compiler errors**: Ensure the test harness files are in your engine folder
