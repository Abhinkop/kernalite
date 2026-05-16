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
#include "drivers/uart.h"
#include "utils/kprintf.h"
#include <stdio.h>

#define FDT_MAGIC 0xedfe0dd0 // 0xd00dfeed in Little Endian

static uart_device_t uart0;
/**
 * @brief UART0 Character Output Function.
 * * This function is designed to be used as the 'putc' function in the
 * serial_t structure for kprintf. It abstracts the hardware-specific details
 * of writing a character to the UART0 data register.
 * * @param chr The character to be transmitted over UART0.
 */
void uart0_putchar(char chr)
{
	uart0.putc(&uart0, chr);
}

int is_valid_fdt(uintptr_t phys_addr)
{
	// 1. Cast the physical address to a pointer
	// (Note: This only works if the address is already mapped or MMU is off)
	// NOLINTNEXTLINE(*-int-to-ptr)
	uint32_t *ptr = (uint32_t *)phys_addr;

	// 2. Check the first 4 bytes for the magic number
	if (*ptr == FDT_MAGIC)
		return 1; // Valid FDT

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
	pl011_init(&uart0, 0x09000000);
	set_kprintf_console((serial_t){ .putc = uart0_putchar, .getc = NULL });
	if (is_valid_fdt(boot_args_ptr[0])) {
		// FDT is valid, you can parse it here or pass it to other functions
		kprintf("Valid FDT\n");
	} else {
		// Handle invalid FDT case (e.g., print an error message)
		kprintf("Invalid FDT\n");
	}
	kprintf("Hello World!\n");

	asm volatile(
		"brk #0"); // Trigger a breakpoint to test exception handling
	/* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
