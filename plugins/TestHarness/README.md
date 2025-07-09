# Test Harness Framework - MVP

## Overview
A minimal test harness framework for GB Studio that provides basic automated testing capabilities. This MVP version focuses on variable testing and validation.

## Features
- **Simple variable testing**: Compare variable values against expected results
- **Visual feedback**: See PASS/FAIL messages with actual vs expected values
- **Test counting**: Track passed and failed tests
- **Easy integration**: Just 2 functions to implement in your game

## Quick Start

### 1. Available Events
- **Test: Start Test Suite** - Initialize and start testing
- **Test: Verify Variable** - Check if a variable matches expected value
- **Test: End Test Suite** - Stop testing and show final results

### 2. Basic Test Flow
```
1. Test: Start Test Suite
2. Test: Verify Variable (repeat as needed)
3. Test: End Test Suite
```

### 3. Integration Required
Add these functions to your game:
```c
void test_display_message(const char* message) BANKED;
void test_clear_display(void) BANKED;
```

Add to your main loop:
```c
if (test_harness_is_active()) test_harness_update();
```

## Example Output
```
Starting tests...
PASS: Player health check
FAIL: Edit mode E:0 A:1
P:01 F:01
```

## Files
- `MVP_QUICKSTART.md` - Quick start guide
- `INTEGRATION_EXAMPLE.md` - Complete integration example
- `engine/` - C code implementation
- `events/` - GB Studio events

## Next Steps
This MVP provides the foundation for more advanced testing features. Once you have this working, you can expand with:
- Enemy positioning tests
- Platform placement tests
- Complex game state validation
- Automated test sequences
