# Simple Test Harness - Basic Variable Testing

## What's Left Active:

### Events:
- **eventTestStart.js** - Start test suite
- **eventTestVerifyVariable.js** - Check variable values

### Everything Else Moved to _trash/:
All the complex events are moved to `_trash/` folder for later use.

## Your First Test Script:

### In GB Studio:
```
1. Test: Start Test Suite
   - test_name: "Basic Variable Test"
   - debug_enabled: ✓

2. Test: Verify Variable  
   - variable: V_EDIT_MODE
   - expected_value: 1
   - test_name: "Edit mode active"
   - delay: 30

3. Test: Verify Variable
   - variable: V_PLAYER_HEALTH  
   - expected_value: 3
   - test_name: "Player health"
   - delay: 30
```

## What You'll See:

```
Starting tests...          ← From start event
PASS: Edit mode active     ← If V_EDIT_MODE = 1
FAIL: Player health E:3 A:2 ← If V_PLAYER_HEALTH = 2 (Expected:3, Actual:2)
P:01 F:01                  ← Final results (1 passed, 1 failed)
```

## Integration (You Need These 2 Functions):

```c
// 1. Display function - show messages on screen
void test_display_message(const char* message) BANKED
{
    // Use your text display system here
    // Examples:
    // set_bkg_tiles(0, 0, strlen(message), 1, message);
    // or display_text(0, 0, message);
    // or printf("%s\n", message);
}

// 2. Clear function - clear message area  
void test_clear_display(void) BANKED
{
    // Clear the message area
    // Examples:
    // fill_bkg_rect(0, 0, 20, 1, 0);
    // or clear_text_area(0, 0, 20, 1);
}

// 3. In your main game loop:
void main_loop(void) BANKED
{
    // Your game logic...
    
    // Add this one line:
    if (test_harness_is_active()) test_harness_update();
    
    // Your rendering...
}
```

## Files You Need:

1. `engine/include/test_harness.h` ✅
2. `engine/src/test_harness_simple.c` ✅ (Use this instead of test_harness.c)
3. `events/eventTestStart.js` ✅
4. `events/eventTestVerifyVariable.js` ✅
5. `engine.json` ✅

## Next Steps:

1. **Try the basic variable test first**
2. **Once it works, move one event at a time from _trash/ back**
3. **Test each addition individually**

This gives you a **minimal, working test system** to start with!
