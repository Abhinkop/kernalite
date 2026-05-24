/**
 * @file main.c
 * @brief Main kernel entry point and primitive UART driver.
 *
 * Handles early kernel initialization, UART setup, FDT parsing, page
 * allocator bootstrapping, and the transition into the kernel runtime loop.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-04-11
 */

#include "linker/symblos.h"
#include "drivers/uart.h"
#include "utils/kprintf.h"
#include "fdt/fdt.h"
#include "allocator/page_allocator.h"
#include "page_table/page_table.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#ifdef RUN_TESTS

/**
 * @brief Run internal kernel tests.
 * * This function is called when the RUN_TESTS flag is set during compilation.
 * * It executes a suite of internal tests to validate kernel functionality
 * before proceeding with normal operation. The results of the tests are
 * printed to the console, and the system exits with an appropriate code based
 * on the test outcomes.
 */
extern void run_internal_tests(void);

#endif /* RUN_TESTS */

static uart_device_t uart0;

/**
 * @brief Captured bootloader arguments.
 * * Stores the raw values of registers x0 through x3 as passed by the
 * bootloader (e.g., U-Boot) at the moment of kernel entry.
 * * - boot_args[0]: Physical address of the Device Tree Blob (FDT).
 * - boot_args[1]: Reserved (0).
 * - boot_args[2]: Reserved (0).
 * - boot_args[3]: Reserved (0).
 */
uint64_t boot_args[4];

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
 * @brief Print the memory map.
 * @param mmap Pointer to the memory map structure.
 */
void print_memory_map(const Memory_map_t *mmap)
{
	kprintf("Memory Map (Total Regions: %d):\n", mmap->count);
	for (int i = 0; i < mmap->count; i++) {
		kprintf("  Region %d: Base = 0x%lx, Size = 0x%lx bytes\n", i,
			mmap->regions[i].base, mmap->regions[i].size);
	}
}

/**
 * @brief Reserve pages occupied by the kernel image.
 * * This function calculates the number of pages occupied by the kernel
 * image based on the linker-provided symbols and reserves those pages in
 * the page allocator to prevent them from being allocated for other purposes.
 * * @return bool True if reservation was successful, false otherwise.
 */
bool reserve_kerenel_img_pages(void)
{
	size_t img_size = get_image_size();
	size_t num_pages = (img_size + PAGE_SIZE - 1) / PAGE_SIZE;
	void *img_start = (void *)&image_start;

	kprintf("Reserving kernel image pages: start=%p, size=0x%lx bytes, pages=%u\n",
		img_start, img_size, num_pages);

	if (!reserve_page(img_start, num_pages)) {
		kprintf("Failed to reserve kernel image pages. Halting.\n");
		return false;
	}
	return true;
}

/**
 * @brief Kernel Main Entry Point.
 * * Called from primary_entry (boot.s) after the stack has been initialized
 * and the BSS section has been cleared.
 * @note This function should never return.
 * @param boot_args_ptr Pointer to an array containing the bootloader arguments
 * passed in registers x0-x3.
 * @return exit code
 */
int main(const uint64_t *boot_args_ptr)
{
	pl011_init(&uart0, 0x09000000);
	set_kprintf_console((serial_t){ .putc = uart0_putchar, .getc = NULL });

	// NOLINTNEXTLINE(*-int-to-ptr)
	const void *fdt_addr = (const void *)boot_args_ptr[0];
	if (!check_fdt(fdt_addr)) {
		kprintf("FDT validation failed. Halting.\n");
		return 1;
	}

	Memory_map_t mmap;
	// NOLINTNEXTLINE(*-int-to-ptr)
	if (get_mem(fdt_addr, &mmap) < 0) {
		kprintf("Failed to parse memory map from FDT. Halting.\n");
		return 1;
	}

	if (mmap.count != 1) {
		kprintf("Current implementation only supports a single memory region. Halting.\n");
		return 1;
	}

	print_memory_map(&mmap);

	// NOLINTBEGIN(*-int-to-ptr)
	bool page_init_result =
		page_init((void *)mmap.regions[0].base, mmap.regions[0].size);
	// NOLINTEND(*-int-to-ptr)

	if (!page_init_result) {
		kprintf("Failed to initialize page allocator. Halting.\n");
		return 1;
	}

	if (!reserve_kerenel_img_pages()) {
		kprintf("Error while reserving kernel binary pages\n");
		return 1;
	}

#ifdef RUN_TESTS
	run_internal_tests();
#endif

	page_dump_status();

	kprintf("Hello World!\n");

	return 0; // returns calls exit with code 0.
}
