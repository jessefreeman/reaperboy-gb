# Test Harness POC - Engine Integration

## What This POC Does

This POC integrates the test harness directly with GB Studio's engine text system. The test harness now:

1. **Uses engine's text display** - No external functions needed
2. **Displays messages directly** - Test results appear immediately
3. **Works with GB Studio events** - Pure event-driven workflow

## POC Test Script

### Example 1: Basic Variable Test
```
1. Test: Start Test Suite
   - test_name: "Health Check"
   - debug_enabled: ✓

2. Store Variable: V_PLAYER_HEALTH = 3

3. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Player health is 3"
   - delay: 30

4. Test: End Test Suite
   - show_results: ✓
```

### Example 2: Multiple Variable Tests
```
1. Test: Start Test Suite
   - test_name: "Game State Test"

2. Store Variable: V_EDIT_MODE = 1
3. Store Variable: V_PLAYER_HEALTH = 2

4. Test: Verify Variable
   - variable: V_EDIT_MODE
   - expected_value: 1
   - test_name: "Edit mode active"
   - delay: 60

5. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Player health full"
   - delay: 60

6. Test: End Test Suite
   - show_results: ✓
```

## Expected Output

### For Example 1:
```
Starting tests...
PASS: Player health is 3
Tests Complete
Passed: 1
Failed: 0
```

### For Example 2:
```
Starting tests...
PASS: Edit mode active
FAIL: Player health full
Expected: 3
Actual: 2
Tests Complete
Passed: 1
Failed: 1
```

## Key Features

### ✅ **No External Functions Required**
- Uses GB Studio's built-in text system
- No need to implement `test_display_message()` or `test_clear_display()`

### ✅ **Automatic Display**
- Test results appear immediately when tests run
- Start/end messages display automatically
- Multi-line failure messages show expected vs actual

### ✅ **Pure Event Control**
- Everything controlled by GB Studio events
- No hardcoded timers or delays
- Events can add delays as needed

### ✅ **Engine Integration**
- Uses `ui_text_data` buffer directly
- Calls `vm_display_text()` for rendering
- Respects GB Studio's text system settings

## Files Updated

1. **`test_harness.h`** - Added engine includes and new display function
2. **`test_harness.c`** - Uses engine's text system directly
3. **`eventTestStart.js`** - Simplified start event
4. **`eventTestVerifyVariable.js`** - Direct display of results
5. **`eventTestEnd.js`** - Simplified end event

## How to Test

1. Create a new scene in GB Studio
2. Add the test events in sequence
3. Set some variables to test values
4. Run the events
5. Watch the test results appear on screen

This POC proves that the test harness can integrate seamlessly with GB Studio's engine!
