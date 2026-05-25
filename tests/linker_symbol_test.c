/**
* @file linker_symbol_test.c
* @brief Test suite for verifying linker symbols and their values.
*/

#include "../src/include/linker/symbols.h"
// #include "../src/include/linker/linker_defines.h"

#include "test.h"

bool allocator_bit_map_check(void)
{
    uintptr_t bitmap_start_addr = (uintptr_t)&page_allocator_bit_map_start;
	uintptr_t bitmap_end_addr = (uintptr_t)&page_allocator_bit_map_end;
    EXPECT(bitmap_start_addr + 0x10000 == bitmap_end_addr);
    return true;
}

test_suite_t get_linker_symbol_test_suite(void)
{
	test_suite_t suite = { .suite_name = "linker_symbols",
			       .tests = { { .test_name = "allocator_bit_map_check",
					    .test_fn = allocator_bit_map_check } },
			       .num_tests = 1 };
	return suite;
}
