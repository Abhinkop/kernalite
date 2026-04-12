/**
 * @file panic.c
 * @brief Diagnostic and emergency error handling routines.
 * @author Abhin Parekadan Jose
 * @date 2024-06-01
 * * This file provides low-level printing utilities for hexadecimal values
 * and the primary panic handler used when the kernel encounters an
 * unrecoverable state.
 */

#include "error/error_strings.h"
#include <stdint.h>

extern void print_uart0(const char *str);

/**
 * @brief Converts a 4-bit nibble to a hex character and prints it.
 * * @param digit The digit to print (0-15).
 * @note This function uses a compound literal to pass a temporary string
 * to the UART driver.
 */
void print_hex_digit(uint8_t digit)
{
	const uint8_t ten = 10;
	const uint8_t ascii_0 = (uint8_t)'0';
	const uint8_t ascii_upper_a = (uint8_t)'A';
	// NOLINTBEGIN(bugprone-narrowing-conversions)
	if (digit < 10)
		print_uart0((const char[]){ digit + ascii_0, 0 });
	else
		print_uart0((const char[]){ digit - ten + ascii_upper_a, 0 });
	// NOLINTEND(bugprone-narrowing-conversions)
}

/**
 * @brief Prints a 64-bit unsigned integer in hexadecimal format.
 * * Iterates through the 64-bit value 4 bits at a time from most
 * significant bit to least significant bit.
 * * @param val The 64-bit value to be printed.
 */
void print_hex(uint64_t val)
{
	print_uart0("0x");
	for (int i = 60; i >= 0; i -= 4) {
		print_hex_digit((val >> i) & 0xF);
	}
}

/**
 * @brief Primary C-level panic entry point.
 * * This function is typically called by the assembly panic_print macro.
 * It outputs the numeric error code, resolves the human-readable
 * error string, and indicates system halt.
 * * @param code The numeric error code representing the failure type.
 * @see error_to_string
 */
void panic_print_c(long code)
{
	print_uart0("\r\n!!! KERNEL PANIC !!!\r\n");
	print_uart0("Error Code:    ");
	print_hex(code);

	print_uart0("\r\nError Message: ");
	print_uart0(error_to_string(code));

	print_uart0("\r\nHalting system.\r\n");
}
