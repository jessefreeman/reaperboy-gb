#ifndef ENEMY_POSITION_TESTS_H
#define ENEMY_POSITION_TESTS_H

#include <gbdk/platform.h>

// ============================================================================
// ENEMY POSITION MANAGER TESTS
// ============================================================================

// Test function to verify enemy position validation
void test_enemy_position_validation(void) BANKED;

// Test function to verify adjacent enemy detection
void test_adjacent_enemy_detection(void) BANKED;

// Test function to verify position cycling
void test_position_cycling(void) BANKED;

// Main test runner
void run_enemy_position_tests(void) BANKED;

#endif // ENEMY_POSITION_TESTS_H
