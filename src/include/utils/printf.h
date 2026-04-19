/**
 * @file printf.h
 * @brief Utility functions for formatted output and serial console management.
 * * Provides a freestanding implementation of printf and vprintf logic
 * tailored for kernel-level debugging and serial communication.
 */

#ifndef UTILS_PRINTF_H
#define UTILS_PRINTF_H

#include <stdarg.h>
#include <stddef.h>

/**
 * @struct serial_t
 * @brief Hardware abstraction layer for serial I/O operations.
 * * This structure maps generic I/O requests to hardware-specific
 * driver functions (e.g., PL011 UART).
 */
typedef struct {
    /** @brief Function pointer to transmit a single character over the serial port. */
    void (*putc)(char chr);

    /** @brief Function pointer to receive a single character from the serial port. */
    char (*getc)();
} serial_t;

/**
 * @brief Configures the global serial console used by printf.
 * * Sets the function pointers that printf will use to output characters.
 * * @param console A serial_t structure containing initialized function pointers.
 */
void set_serial_console(serial_t console);

/**
 * @brief Variadic print function.
 * * Processes the format string and arguments, outputting characters via the 
 * configured serial console. 
 * * Supports: %%c, %%s, %%d, %%u, %%x, %%p.
 * * @param format Formatting string containing plain text and specifiers.
 * @param args   An initialized va_list containing the arguments to be formatted.
 * @return       The total number of characters successfully printed.
 */
int vprintf(const char *format, va_list args);

/**
 * @brief Standard formatted print to the serial console.
 * * A wrapper around vprintf that handles variadic argument initialization 
 * and cleanup.
 * * @param format Formatting string containing plain text and specifiers.
 * @param ...    Variable arguments matching the specifiers in the format string.
 * @return       The total number of characters successfully printed.
 */
int printf(const char *format, ...);

#endif // UTILS_PRINTF_H
