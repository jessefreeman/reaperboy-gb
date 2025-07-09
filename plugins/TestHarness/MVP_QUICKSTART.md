# Test Harness MVP - Quick Start Guide

## What You Get

A minimal test harness that lets you:
1. **Start a test** - Initialize the test system
2. **Compare variables** - Check if a variable has the expected value
3. **Get results** - See PASS/FAIL messages and final counts
4. **End the test** - Stop testing and show final results

## Basic Usage in GB Studio

### Step 1: Start Your Test
```
Event: Test: Start Test Suite
- test_name: "My First Test"
- debug_enabled: ✓
```

### Step 2: Test Some Variables
```
Event: Test: Verify Variable
- variable: V_PLAYER_HEALTH
- expected_value: 3
- test_name: "Player starts with 3 health"
- delay: 30

Event: Test: Verify Variable  
- variable: V_EDIT_MODE
- expected_value: 0
- test_name: "Not in edit mode"
- delay: 30
```

### Step 3: End Your Test
```
Event: Test: End Test Suite
- show_results: ✓
```

## What You'll See

```
Starting tests...                    ← From start event
PASS: Player starts with 3 health   ← If variable matches
FAIL: Not in edit mode E:0 A:1      ← If variable doesn't match (Expected:0, Actual:1)
P:01 F:01                           ← Final results (1 passed, 1 failed)
```

## Integration Required

You need to add these 2 functions to your game code:

```c
// Display a message on screen
void test_display_message(const char* message) BANKED {
    // Use your game's text display system
    // Examples:
    // display_text(0, 0, message);
    // printf("%s\\n", message);
    // set_bkg_tiles(0, 0, strlen(message), 1, message);
}

// Clear the message display area
void test_clear_display(void) BANKED {
    // Clear your message area
    // Examples:
    // clear_text_area(0, 0, 20, 2);
    // fill_bkg_rect(0, 0, 20, 2, 0);
}
```

## Add to Your Game Loop

In your main game loop, add this one line:

```c
void main_loop(void) BANKED {
    // Your existing game logic...
    
    // Add this line to update the test harness
    if (test_harness_is_active()) {
        test_harness_update();
    }
    
    // Your existing rendering code...
}
```

## Files Included

- `plugin.json` - Plugin configuration
- `engine/include/test_harness.h` - Header file  
- `engine/src/test_harness_simple.c` - Core implementation
- `events/eventTestStart.js` - Start test event
- `events/eventTestVerifyVariable.js` - Variable verification event
- `events/eventTestEnd.js` - End test event

## That's It!

You now have a working test harness MVP. Start with a simple variable test and expand from there.

## Next Steps

1. Test one variable first to make sure it works
2. Add more variable tests as needed
3. Later: Add more complex test events (enemy positioning, etc.)
4. Later: Add automated test sequences
