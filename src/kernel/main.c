/**
 * @file main.c
 * @brief Main kernel entry point and primitive UART driver.
 * @author Abhin Parekadan Jose
 * @date 2024-06-01
 * * This file contains the initialization sequence for the C environment
 * and a basic polling-based UART driver for QEMU 'virt' board or similar
 * ARM-based systems.
 */

#include <stddef.h>
#include <stdint.h>
#include <libfdt.h>

#include "linker/symblos.h"
#include "utils/fdt.h"
#include "utils/printf.h"
#include "drivers/uart.h"
#include "allocator/page_allocater.h"

uart_device_t uart0; // Global UART device instance for early boot logging

bool is_valid_fdt(uint64_t phys_addr)
{
	// 1. Cast the physical address to a pointer
	// NOLINTNEXTLINE(*-int-to-ptr)
	void *ptr = (void *)phys_addr;
	const int ret = fdt_check_header(ptr);
	return ret == 0;
}

// NOLINTNEXTLINE(misc-include-cleaner)
void putchar(char chr)
{
	// NOLINTNEXTLINE(misc-include-cleaner)
	uart0.putc(&uart0, chr);
}

void print_kernel_bounds()
{
	uintptr_t start = (uintptr_t)&kernel_start;
	uintptr_t end = (uintptr_t)&kernel_end;

	size_t kernel_size = end - start;

	printf("Kernel Start: 0x%lx\n", start);
	printf("Kernel End:   0x%lx\n", end);
	printf("Kernel Size:  %u bytes\n", kernel_size);
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
	set_serial_console((serial_t){ .putc = putchar, .getc = NULL });
	print_kernel_bounds();
	Memory_map_t mmap;
	// NOLINTNEXTLINE(*-int-to-ptr)
	int ret = get_mem((const void *)boot_args_ptr[0], &mmap);
	if (ret < 0) {
		printf("Failed to parse FDT memory map\n");
	} else {
		printf("Memory Map (%d regions):\n", mmap.count);
		for (int i = 0; i < mmap.count; i++) {
			printf("Region %d: Base=0x%lx, Size=0x%lx\n", i,
			       (unsigned long)mmap.regions[i].base,
			       (unsigned long)mmap.regions[i].size);

			uintptr_t region_base = (uintptr_t)mmap.regions[i].base;
			uintptr_t region_size = (uintptr_t)mmap.regions[i].size;
			uintptr_t region_end = region_base + region_size;

			uintptr_t k_end = (uintptr_t)&kernel_end;

			// Only use regions that actually contain or follow the kernel
			if (region_end > k_end) {
				// If the kernel is inside this region, start allocator after kernel
				uintptr_t alloc_start = (region_base > k_end) ?
								region_base :
								k_end;
				size_t alloc_size = region_end - alloc_start;

				printf("Initializing allocator for Region %d:\n",
				       i);
				printf("  Start: 0x%lx, Size: 0x%lx\n",
				       alloc_start, alloc_size);

				// NOLINTNEXTLINE(*-int-to-ptr)
				page_init((void *)alloc_start, alloc_size);
			}
		}
	}
	if (is_valid_fdt(boot_args_ptr[0])) {
		// FDT is valid, you can parse it here or pass it to other functions
		printf("Valid FDT\n");
		page_dump_status();
	} else {
		// Handle invalid FDT case (e.g., print an error message)
		printf("Invalid FDT\n");
	}
	printf("Hello World!\n");

	/* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
