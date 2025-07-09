#include "test_harness.h"

// Test harness state - just track counters and active status
static UBYTE test_active = 0;
static UBYTE test_results_passed = 0;
static UBYTE test_results_failed = 0;

// Initialize the test harness
void test_harness_init(void) BANKED {
    test_active = 0;
    test_results_passed = 0;
    test_results_failed = 0;
}

// Update the test harness (call this each frame)
void test_harness_update(void) BANKED {
    // No automatic behavior - everything controlled by events
}

// Check if test harness is active
UBYTE test_harness_is_active(void) BANKED {
    return test_active;
}

// Start test execution
void test_start_execution(void) BANKED {
    test_active = 1;
    test_results_passed = 0;
    test_results_failed = 0;
    
    // Display start message
    test_display_message("Starting tests...");
}

// Stop test execution
void test_stop_execution(void) BANKED {
    test_active = 0;
    
    // Display final results
    if (test_results_failed == 0) {
        test_display_message("All tests passed!");
    } else {
        test_display_message("Some tests failed!");
    }
}

// Get test results
UBYTE test_get_passed_count(void) BANKED {
    return test_results_passed;
}

UBYTE test_get_failed_count(void) BANKED {
    return test_results_failed;
}

UBYTE test_get_total_count(void) BANKED {
    return test_results_passed + test_results_failed;
}

// Verify a variable value and return result
UBYTE test_verify_variable(UBYTE actual_value, UBYTE expected_value, const char* description) BANKED {
    description; // Suppress unused parameter warning
    
    if (actual_value == expected_value) {
        test_results_passed++;
        return 1; // PASS
    } else {
        test_results_failed++;
        return 0; // FAIL
    }
}

// Stub implementations of display functions
// These will be overridden by your game's implementations
void test_display_message(const char* message) BANKED {
    message; // Suppress unused parameter warning
    // Default implementation does nothing
    // Override this function in your game code
}

void test_clear_display(void) BANKED {
    // Default implementation does nothing  
    // Override this function in your game code
}
