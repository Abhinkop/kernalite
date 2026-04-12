/**
 * @file main.c
 * @brief Main kernel entry point and primitive UART driver.
 * @author Abhin Parekadan Jose
 * @date 2024-06-01
 * * This file contains the initialization sequence for the C environment
 * and a basic polling-based UART driver for QEMU 'virt' board or similar
 * ARM-based systems.
 */

#include <stdint.h>

#include "linker/symblos.h"

#define FDT_MAGIC 0xedfe0dd0 // 0xd00dfeed in Little Endian

/**
 * @brief UART0 Data Register (MMIO).
 * * This pointer accesses the hardware's transmit/receive buffer.
 * It is marked 'volatile' to prevent the compiler from optimizing out
 * repeated writes to the same memory location, which are necessary
 * for hardware communication.
 */
volatile unsigned int *const uart0_dr = (unsigned int *)0x09000000;

/**
 * @brief Transmits a null-terminated string over UART0.
 * * This function performs a simple polling-based write. It does not
 * check for FIFO status (TX Full), assuming the baud rate/buffer is
 * sufficient for early boot strings.
 * * @param str A pointer to the null-terminated ASCII string to be printed.
 */
void print_uart0(const char *str)
{
	while (*str != '\0') { /* Loop until end of string */
		*uart0_dr = (unsigned int)(*str); /* Transmit char */
		str++; /* Next char */
	}
}

int is_valid_fdt(uint64_t phys_addr)
{
	// 1. Cast the physical address to a pointer
	// (Note: This only works if the address is already mapped or MMU is off)
	uint32_t *ptr = (uint32_t *)phys_addr;

	// 2. Check the first 4 bytes for the magic number
	if (*ptr == FDT_MAGIC) {
		return 1; // Valid FDT
	}

	return 0; // Not an FDT
}

/**
 * @brief Kernel Main Entry Point.
 * * Called from primary_entry (boot.s) after the stack has been initialized
 * and the BSS section has been cleared.
 * * @note This function should never return.
 */
void main(const uint64_t *boot_args_ptr)
{
	if (is_valid_fdt(boot_args_ptr[0])) {
		// FDT is valid, you can parse it here or pass it to other functions
		print_uart0("Valid FDT\n");
	} else {
		// Handle invalid FDT case (e.g., print an error message)
		print_uart0("Invalid FDT\n");
	}
	print_uart0("Hello World!\n");

	/* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
