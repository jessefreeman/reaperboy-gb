# Test Harness - Fixed Implementation

## Issues Fixed

1. **Engine version warning** - Moved `engine.json` to correct location (`engine/engine.json`)
2. **Memory conflicts** - Removed direct engine includes to avoid symbol conflicts
3. **Simplified implementation** - Back to basic approach with external display functions

## Implementation Required (Optional)

The test harness provides default stub implementations of the display functions, so it will compile and run without any additional code. However, to see the test results, you should override these functions in your game code:

```c
#include "test_harness.h"

// Override this function to display test messages
void test_display_message(const char* message) BANKED {
    // Option 1: Use a simple text event
    // (This is the easiest - just show the message however you want)
    
    // Option 2: Use printf if available
    // printf("%s\n", message);
    
    // Option 3: Use your game's text system
    // show_text(message);
}

// Override this function to clear the display (optional)
void test_clear_display(void) BANKED {
    // Clear previous messages if needed
    // This can be empty if you don't need to clear
}
```

**Note:** If you don't override these functions, the test harness will still track test results internally, but you won't see any messages on screen.

## Test Script Example

```
1. Test: Start Test Suite
   - test_name: "Basic Test"
   - debug_enabled: ✓

2. Store Variable: V_PLAYER_HEALTH = 3

3. Test: Verify Variable
   - variable: V_PLAYER_HEALTH
   - expected_value: 3
   - test_name: "Health check"
   - delay: 60

4. Test: End Test Suite
   - show_results: ✓
```

## What You'll See

The test harness will call your `test_display_message()` function with:
- "Starting tests..." (from start event)
- "PASS: Health check" or "FAIL: Health check" (from verify event)
- "Tests Complete\nPassed: 1\nFailed: 0" (from end event)

## Files Structure

```
plugins/TestHarness/
├── engine/
│   ├── engine.json ← Fixed location
│   ├── include/test_harness.h ← No engine includes
│   └── src/test_harness.c ← Simple implementation
├── events/
│   ├── eventTestStart.js ← Uses textDialogue
│   ├── eventTestVerifyVariable.js ← Shows PASS/FAIL
│   └── eventTestEnd.js ← Shows final results
└── plugin.json
```

The implementation is now much simpler and should compile without memory conflicts!
