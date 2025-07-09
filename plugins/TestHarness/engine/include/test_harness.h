#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <gbdk/platform.h>

// Test harness core functions
extern void test_harness_init(void) BANKED;
extern void test_harness_update(void) BANKED;
extern UBYTE test_harness_is_active(void) BANKED;

// Test execution functions
extern void test_start_execution(void) BANKED;
extern void test_stop_execution(void) BANKED;

// Test result functions
extern UBYTE test_get_passed_count(void) BANKED;
extern UBYTE test_get_failed_count(void) BANKED;
extern UBYTE test_get_total_count(void) BANKED;

// Test functions - returns 1 for PASS, 0 for FAIL
extern UBYTE test_verify_variable(UBYTE actual_value, UBYTE expected_value, const char* description) BANKED;

// Display functions - default implementations provided, override in your game
extern void test_display_message(const char* message) BANKED;
extern void test_clear_display(void) BANKED;

#endif
