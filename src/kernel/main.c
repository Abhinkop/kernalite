/**
 * @file main.c
 * @brief Main kernel entry point and primitive UART driver.
 * @author Abhin Parekadan Jose
 * @date 2024-06-01
 * * This file contains the initialization sequence for the C environment
 * and a basic polling-based UART driver for QEMU 'virt' board or similar
 * ARM-based systems.
 */

#include "linker/symblos.h"
#include "drivers/uart.h"
#include "utils/kprintf.h"
#include "fdt/check.h"

#include <stdio.h>
#include <stdint.h>

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

	if (!check_fdt(boot_args_ptr[0])) {
		kprintf("FDT validation failed. Halting.\n");
		return;
	}

	kprintf("Hello World!\n");

	asm volatile(
		"brk #0"); // Trigger a breakpoint to test exception handling
	/* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
