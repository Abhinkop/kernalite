/**
 * @file fdt.c
 * @brief Implementation of FDT memory discovery.
 */

#include "utils/printf.h"

#include "string.h"
#include <stddef.h>
#include <stdarg.h>

serial_t serial_console;

void set_serial_console(serial_t con)
{
	serial_console.getc = con.getc;
	serial_console.putc = con.putc;
}

static void itoa(unsigned long num, int base, char *buf)
{
	char *ptr = buf;
	char *ptr1 = buf;
	char *ptr2 = NULL;
	unsigned long digit;

	// Build string in reverse
	do {
		digit = num % base;
		// NOLINTNEXTLINE(bugprone-narrowing-conversions)
		*ptr++ = (digit < 10) ? (digit + '0') : (digit - 10 + 'a');
	} while ((num /= base) > 0);

	*ptr = '\0';

	// Reverse the string in place
	ptr1 = buf;
	ptr2 = ptr - 1;
	while (ptr1 < ptr2) {
		char tmp = *ptr1;
		*ptr1++ = *ptr2;
		*ptr2-- = tmp;
	}
}

/**
 * @brief Implementation of vprintf.
 * @return Total number of characters output to UART.
 */
// NOLINTNEXTLINE(*-cognitive-complexity)
int vprintf(const char *format, va_list args)
{
	if (serial_console.putc == NULL) {
		return 0; // No console set, can't print
	}

	char buf[64];
	const char *ptr = NULL;
	int printed = 0;

	for (ptr = format; *ptr != '\0'; ptr++) {
		if (*ptr != '%') {
			if (*ptr == '\n') {
				serial_console.putc('\r');
				printed++;
			}
			serial_console.putc(*ptr);
			printed++;
			continue;
		}

		ptr++; // Skip '%'

		switch (*ptr) {
		case 'l':
			ptr++;
		case 'x':
		case 'p': {
			if (*ptr == 'p') {
				serial_console.putc('0');
				serial_console.putc('x');
				printed += 2;
			}
			unsigned long num = va_arg(args, unsigned long);
			itoa(num, 16, buf);
			for (char *str = buf; *str; str++) {
				serial_console.putc(*str);
				printed++;
			}
			break;
		}
		case 'c': {
			char chr = (char)va_arg(args, int);
			serial_console.putc(chr);
			printed++;
			break;
		}
		case 's': {
			char *str = va_arg(args, char *);
			if (!str)
				str = "(null)";
			while (*str) {
				serial_console.putc(*str++);
				printed++;
			}
			break;
		}
		case 'd': {
			long num = va_arg(args, long);
			if (num < 0) {
				serial_console.putc('-');
				printed++;
				num = -num;
			}
			itoa((unsigned long)num, 10, buf);
			for (char *str = buf; *str; str++) {
				serial_console.putc(*str);
				printed++;
			}
			break;
		}
		case 'u': {
			unsigned long num = va_arg(args, unsigned long);
			itoa(num, 10, buf);
			for (char *str = buf; *str; str++) {
				serial_console.putc(*str);
				printed++;
			}
			break;
		}
		case '%': {
			serial_console.putc('%');
			printed++;
			break;
		}
		default: {
			serial_console.putc('%');
			serial_console.putc(*ptr);
			printed += 2;
			break;
		}
		}
	}
	return printed;
}

int printf(const char *format, ...)
{
	int printed = 0;

	va_list args;
	va_start(args, format);
	// NOLOINTNEXTLINE(clang-analyzer-valist*)
	printed = vprintf(format, args);
	va_end(args);

	return printed;
}
