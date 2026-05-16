/**
 * @file handler.c
 * @brief Exception and interrupt handler utilities used during early boot.
 *
 * This file contains a simple generic exception handler used during
 * early bring-up to report the current exception level and stack pointer
 * selection, then halt to avoid returning to the faulting context.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-05-16
 */

#include "utils/kprintf.h"

#include <stdint.h>

void generic_handler(void)
{
	kprintf("Generic Handler invoked\n");
	uint64_t current_el = 0;
	uint64_t sp_sel = 0;

	asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
	asm volatile("mrs %0, SPSel" : "=r"(sp_sel));

	uint64_t el_num = (current_el >> 2) & 0x3;
	kprintf("Current EL = %lx\n", el_num);
	kprintf("Used stack is = EL%lx\n", sp_sel == 0 ? 0 : el_num);

	while (1) {
		// Loop indefinitely to prevent returning to the faulting code
	}
}
