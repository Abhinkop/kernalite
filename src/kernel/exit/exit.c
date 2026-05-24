
/**
 * @file exit.c
 * @brief Provides a mechanism to exit QEMU with a specific code.
 */

#include "utils/kprintf.h"

#include <stdint.h>

[[noreturn]] void exit(uint32_t code)
{
	kprintf("Exiting QEMU with code: %u\n", code);

	volatile uint64_t block[2];
	block[0] = 0x20026;
	block[1] = code;

	// NOLINTBEGIN(hicpp-no-assembler)
	__asm__ volatile("mov x0, #0x18\n" /* SYS_EXIT */
			 "mov x1, %0\n" /* pointer to block */
			 "hlt #0xF000\n" ::"r"((uint64_t)block)
			 : "x0", "x1", "memory");
	// NOLINTEND(hicpp-no-assembler)
	__builtin_unreachable();
}
