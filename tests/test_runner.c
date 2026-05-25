/**
 * Test runner for executing kernel tests.
 */

#include "../src/include/utils/kprintf.h"
#include "../src/include/utils/utils.h"

#include "test.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_TEST_SUITES 10

/**
 * @brief Exit QEMU with a specific code.
 * @param code The exit code.
 */
static inline void qemu_exit(uint32_t code)
{
	volatile uint64_t block[2];
	block[0] = 0x20026;
	block[1] = code;

	// NOLINTBEGIN(hicpp-no-assembler)
	__asm__ volatile("mov x0, #0x18\n" /* SYS_EXIT */
			 "mov x1, %0\n" /* pointer to block */
			 "hlt #0xF000\n" ::"r"((uint64_t)block)
			 : "x0", "x1", "memory");
	// NOLINTEND(hicpp-no-assembler)
}

/**
 * @breif list of all test suites to run. Each suite contains multiple tests.
 * The test runner will execute all tests in all suites and report results.
 */
extern test_suite_t get_page_table_test_suite(void);
extern test_suite_t get_linker_symbol_test_suite(void);

/**
 * @brief Run internal kernel tests.
 * * This function is called when the RUN_TESTS flag is set during compilation.
 * * It executes a suite of internal tests to validate kernel functionality
 * before proceeding with normal operation. The results of the tests are
 * printed to the console, and the system exits with an appropriate code based
 * on the test outcomes.
 * @param fdt_addr Pointer to the Device Tree Blob (FDT) address passed by the bootloader.
 */
void run_internal_tests(const void *fdt_addr)
{
	kprintf("Running internal tests...\n");

	test_suite_t test_suite[MAX_TEST_SUITES];
	size_t num_suites = 0;
	bool failed = false;

	test_suite[num_suites++] = get_page_table_test_suite();
	test_suite[num_suites++] = get_linker_symbol_test_suite();

	for (size_t i = 0; i < num_suites; i++) {
		kprintf("Running test suite: %s\n", test_suite[i].suite_name);
		for (size_t j = 0; j < test_suite[i].num_tests; j++) {
			if (!setup_global_allocator(fdt_addr)) {
				kprintf("Failed to set up global allocator. Halting tests.\n");
				qemu_exit(1);
			}

			kprintf("  Running test: %s\n",
				test_suite[i].tests[j].test_name);

			if (test_suite[i].tests[j].test_fn()) {
				kprintf("    Result: %s PASSED\n",
					test_suite[i].tests[j].test_name);
			} else {
				kprintf("    Result: %s FAILED\n",
					test_suite[i].tests[j].test_name);
				failed = true;
			}
		}
	}

	qemu_exit(failed);
}
