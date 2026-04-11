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

/**
 * @brief UART0 Data Register (MMIO).
 * * This pointer accesses the hardware's transmit/receive buffer.
 * It is marked 'volatile' to prevent the compiler from optimizing out
 * repeated writes to the same memory location, which are necessary
 * for hardware communication.
 */
volatile unsigned int *const UART0DR = (unsigned int *)0x09000000;

/**
 * @brief Transmits a null-terminated string over UART0.
 * * This function performs a simple polling-based write. It does not
 * check for FIFO status (TX Full), assuming the baud rate/buffer is
 * sufficient for early boot strings.
 * * @param s A pointer to the null-terminated ASCII string to be printed.
 */
void print_uart0(const char *s) {
  while (*s != '\0') {             /* Loop until end of string */
    *UART0DR = (unsigned int)(*s); /* Transmit char */
    s++;                           /* Next char */
  }
}

/**
 * @brief Kernel Main Entry Point.
 * * Called from primary_entry (boot.s) after the stack has been initialized
 * and the BSS section has been cleared.
 * * @note This function should never return.
 */
void main() {
  print_uart0("Hello World!\n");

  /* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
