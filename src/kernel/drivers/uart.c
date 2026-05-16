#include "uart.h"
#include <stdint.h>

/* PL011 Register Offsets */
#define UART_DR 0x00
#define UART_FR 0x18

/* Flag Register Bits */
#define UART_FR_TXFF (1 << 5) // Transmit FIFO full
#define UART_FR_RXFE (1 << 4) // Receive FIFO empty

/**
 * @brief PL011 Specific putc implementation.
 */
static void pl011_putc(uart_device_t *dev, char chr)
{
	// NOLINTNEXTLINE(*-int-to-ptr)
	volatile uint32_t *data_reg = (uint32_t *)(dev->mmio_base + UART_DR);

	*data_reg = (uint32_t)chr;
}

/**
 * @brief PL011 Specific getc implementation.
 */
static char pl011_getc(uart_device_t *dev)
{
	// NOLINTNEXTLINE(*-int-to-ptr)
	volatile uint32_t *data_reg = (uint32_t *)(dev->mmio_base + UART_DR);
	// NOLINTNEXTLINE(*-int-to-ptr)
	volatile uint32_t *f_reg = (uint32_t *)(dev->mmio_base + UART_FR);

	// Wait for data to arrive
	while (*f_reg & UART_FR_RXFE) {
		__asm__ volatile("nop");
	}

	return (char)(*data_reg & 0xFF);
}

/**
 * @brief Bind the PL011 functions to the device struct.
 */
void pl011_init(uart_device_t *dev, uintptr_t base)
{
	dev->mmio_base = base;
	dev->putc = pl011_putc;
	dev->getc = pl011_getc;
}
