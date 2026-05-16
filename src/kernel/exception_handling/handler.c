#include <stdint.h>

#include "utils/printf.h"

void curr_el_curr_sp_sync_handler(void)
{
	uint64_t current_el = 0;

	asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
	current_el = (current_el >> 2) & 0x3;

	printf("Exception from the current EL while using SP_EL%lu\n",
	       current_el);
	while (1) {
		// Loop indefinitely to prevent returning to the faulting code
	}
}

void generic_handler(void)
{
	printf("Generic Handler invoked\n");
	uint64_t current_el = 0;
	uint64_t sp_sel = 0;

	asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
	asm volatile("mrs %0, SPSel" : "=r"(sp_sel));

	uint64_t el_num = (current_el >> 2) & 0x3;
	printf("Current EL = %lu\n", el_num);
	printf("Used stack is = EL%lu\n", sp_sel == 0 ? 0 : el_num);

	while (1) {
		// Loop indefinitely to prevent returning to the faulting code
	}
}