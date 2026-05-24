/**
 * Dummy test file for testing the test framework.
 * This file should be compiled and run when the RUN_TESTS flag is set.
 * It contains placeholder tests that can be expanded with actual test logic.
 *
 * To run tests, set the RUN_TESTS flag in the Makefile and compile the kernel.
 * The tests will be executed during kernel initialization, and their results
 * will be printed to the console.
 *
 * Note: This file is intended for demonstration purposes and should be replaced
 * with actual test cases as needed.
 */

#include "../src/include/utils/kprintf.h"

#include "test.h"

#include <stdbool.h>
#include <stddef.h>


/**
 * @brief Run a dummy test.
 * @return true if the test passes, false otherwise.
 */
bool test(void)
{
	return true; // Indicate test passed
}

/**
 * @brief Run a dummy failing test.
 * @return true if the test passes, false otherwise.
 */
bool test_fail(void)
{
	return false; // Indicate test failed
}

/**
 * @brief Get the dummy test suite.
 * @return The dummy test suite.
 */
test_suite_t get_dummy_test_suite(void)
{
	test_suite_t suite = {
		.suite_name = "dummy",
		.tests = {
            { .test_name = "test", .test_fn = test },
            { .test_name = "test_fail", .test_fn = test_fail }
        },
		.num_tests = 2
	};
    return suite;
}
