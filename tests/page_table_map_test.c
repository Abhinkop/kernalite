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

#include "../src/include/page_table/page_table.h"
#include "../src/include/allocator/page_allocator.h"

#include "test.h"

#include <stdbool.h>
#include <stddef.h>

static inline phy_addr
get_page_table_entry_address(const page_table_entry_t *entry)
{
	// Shift to align address to bits [47:12]
	return ((phy_addr)entry->nlta_47_12 << 12);
}

static inline page_table_t *get_next_level_table(page_table_entry_t *entry)
{
	if (!entry->valid) {
		return NULL;
	}

	phy_addr next_table_addr = get_page_table_entry_address(entry);

	// NOLINTNEXTLINE(*-int-to-ptr)
	return (page_table_t *)next_table_addr;
}

bool verify_map(page_table_t *root, virt_addr v_addr, phy_addr expected_pa,
		page_permissions_t expected_perms)
{
	// Get Indices
	const uint64_t l0_idx_start_bit = 39;
	const uint64_t l1_idx_start_bit = 30;
	const uint64_t l2_idx_start_bit = 21;
	const uint64_t l3_idx_start_bit = 12;
	const uint64_t idx_mask = 0x1FFUL; // 9 bits for each level index

	const uint64_t l0_idx = (v_addr >> l0_idx_start_bit) & idx_mask;
	const uint64_t l1_idx = (v_addr >> l1_idx_start_bit) & idx_mask;
	const uint64_t l2_idx = (v_addr >> l2_idx_start_bit) & idx_mask;
	const uint64_t l3_idx = (v_addr >> l3_idx_start_bit) & idx_mask;

	EXPECT(root->entries[l0_idx].value);
	page_table_t *level1 = get_next_level_table(&root->entries[l0_idx]);
	EXPECT(level1 != NULL);
	EXPECT(level1->entries[l1_idx].value);
	page_table_t *level2 = get_next_level_table(&level1->entries[l1_idx]);
	EXPECT(level2 != NULL);
	EXPECT(level2->entries[l2_idx].value);
	page_table_t *level3 = get_next_level_table(&level2->entries[l2_idx]);
	EXPECT(level3 != NULL);
	phy_addr actual_pa =
		get_page_table_entry_address(&level3->entries[l3_idx]);
	EXPECT(actual_pa == expected_pa);
	(void)expected_perms; // TODO: Verify permissions once we set them in
			      // map_page
	return true;
}

bool test_page_table_mapping(page_table_t *root, virt_addr v_addr,
			     phy_addr phy_addr, page_permissions_t test_perms)
{
	EXPECT(root != NULL);
	EXPECT(map_page(root, v_addr, phy_addr, test_perms));
	EXPECT(verify_map(root, v_addr, phy_addr, test_perms));

	return true; // Indicate test passed
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool page_table_map(void)
{
	page_table_t *root = (page_table_t *)page_alloc(1);

	// Group 1: Share L0, L1, L2 — only L3 entries differ
	// VA bits [47:39]=0, [38:30]=0, [29:21]=0, [20:12]=1,2,3,4
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00000001000, 0x1000,
		(page_permissions_t){ .read = 1, .write = 0, .execute = 0 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00000002000, 0x2000,
		(page_permissions_t){ .read = 1, .write = 1, .execute = 0 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00000003000, 0x3000,
		(page_permissions_t){ .read = 1, .write = 0, .execute = 1 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00000004000, 0x4000,
		(page_permissions_t){ .read = 1, .write = 1, .execute = 1 }));

	// Group 2: Different L1 region — forces new L2 table allocation
	// VA bits [47:39]=0, [38:30]=1,2,3,4, [29:21]=0, [20:12]=0
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00040000000, 0x5000,
		(page_permissions_t){ .read = 0, .write = 0, .execute = 1 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00080000000, 0x6000,
		(page_permissions_t){ .read = 1, .write = 0, .execute = 0 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF000C0000000, 0x7000,
		(page_permissions_t){ .read = 0, .write = 1, .execute = 0 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF00100000000, 0x8000,
		(page_permissions_t){ .read = 1, .write = 1, .execute = 0 }));

	// Group 3: Different L0 region — forces new L1 table allocation
	// VA bits [47:39]=1,2,3,4, [38:30]=0, [29:21]=0, [20:12]=0
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF08000000000, 0x9000,
		(page_permissions_t){ .read = 0, .write = 0, .execute = 1 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF10000000000, 0xA000,
		(page_permissions_t){ .read = 1, .write = 0, .execute = 1 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF18000000000, 0xB000,
		(page_permissions_t){ .read = 0, .write = 1, .execute = 1 }));
	EXPECT(test_page_table_mapping(
		root, 0xFFFFF20000000000, 0xC000,
		(page_permissions_t){ .read = 1, .write = 1, .execute = 1 }));
	return true;
}

test_suite_t get_page_table_test_suite(void)
{
	test_suite_t suite = { .suite_name = "page_table",
			       .tests = { { .test_name = "page_table_map",
					    .test_fn = page_table_map } },
			       .num_tests = 1 };
	return suite;
}
