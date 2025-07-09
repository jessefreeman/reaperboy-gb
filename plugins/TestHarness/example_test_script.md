# GB Studio Test Script Example

## Tilemap Editor Test Script

This is an example of how to create a test script in GB Studio using the test harness events to test your tilemap editor mode.

### Test: Enter Tilemap Edit Mode

**Test Steps:**
1. **Test: Start Test Suite**
   - test_name: "Tilemap Editor Tests"
   - debug_enabled: true

2. **Test: Log Message**
   - message: "Testing tilemap editor entry"
   - delay: 30

3. **Test: Press Button**
   - button: "Start"
   - hold_frames: 3
   - delay: 6 (0.1 seconds at 60fps)

4. **Test: Wait Frames**
   - frames: 6
   - message: "Waiting for menu"

5. **Test: Press Button**
   - button: "A"
   - hold_frames: 3
   - delay: 30 (0.5 seconds at 60fps)

6. **Test: Wait Frames**
   - frames: 30
   - message: "Waiting for mode change"

7. **Test: Verify Variable**
   - variable: V_EDIT_MODE
   - expected_value: 1
   - test_name: "Edit mode active"
   - delay: 30

8. **Test: Verify Actor Active**
   - actor: cursor
   - should_be_active: true
   - test_name: "Cursor visible"
   - delay: 30

9. **Test: Log Message**
   - message: "Testing cursor movement"
   - delay: 30

10. **Test: Press Button**
    - button: "Right"
    - hold_frames: 3
    - delay: 15

11. **Test: Press Button**
    - button: "Right"
    - hold_frames: 3
    - delay: 15

12. **Test: Verify Actor Position**
    - actor: cursor
    - expected_x: 2
    - expected_y: 0
    - test_name: "Cursor moved right"
    - delay: 30

### Test: Platform Placement

13. **Test: Log Message**
    - message: "Testing platform placement"
    - delay: 30

14. **Test: Press Button**
    - button: "A"
    - hold_frames: 3
    - delay: 30

15. **Test: Verify Variable**
    - variable: V_PLATFORM_COUNT
    - expected_value: 1
    - test_name: "Platform placed"
    - delay: 30

### Test: Exit Edit Mode

16. **Test: Log Message**
    - message: "Testing exit edit mode"
    - delay: 30

17. **Test: Press Button**
    - button: "Start"
    - hold_frames: 3
    - delay: 30

18. **Test: Verify Variable**
    - variable: V_EDIT_MODE
    - expected_value: 0
    - test_name: "Edit mode inactive"
    - delay: 30

19. **Test: Verify Actor Active**
    - actor: cursor
    - should_be_active: false
    - test_name: "Cursor hidden"
    - delay: 30

20. **Test: Log Message**
    - message: "Tests complete"
    - delay: 60

## Key Benefits

1. **Non-blocking waits**: Tests don't freeze the game - they let it run
2. **Real button simulation**: Tests actual input handling
3. **State validation**: Checks variables, actors, positions
4. **Visual feedback**: Shows progress and results
5. **Reproducible**: Same sequence every time
6. **Automated**: No manual intervention needed

## Usage Notes

- Place this sequence in a GB Studio scene
- Call `test_harness_update()` from your main game loop
- Tests will run automatically when scene is triggered
- Results are displayed on screen
- Failed tests show expected vs actual values

## Example Integration Code

```c
// In your main game loop
void main_game_loop(void) BANKED
{
    // ... your game logic ...
    
    // Update test harness if active
    if (test_harness_is_active())
    {
        test_harness_update();
    }
    
    // ... rest of game logic ...
}

// Implement required functions
void test_simulate_button_press(UBYTE button) BANKED
{
    // Set joypad state for next frame
    joypad_pressed |= button;
}

UBYTE test_get_actor_active(UBYTE actor_id) BANKED
{
    // Return 1 if actor is active, 0 if not
    return actors[actor_id].active;
}

UBYTE test_get_actor_x(UBYTE actor_id) BANKED
{
    return actors[actor_id].x;
}

UBYTE test_get_actor_y(UBYTE actor_id) BANKED
{
    return actors[actor_id].y;
}
```
