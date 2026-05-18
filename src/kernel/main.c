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
 * @brief Setup dummy page table mappings for testing.
 * @param root Pointer to the root page table.
 */
void setup_dummy_mappings(page_table_t *root)
{
	page_table_init(root);

	// Group 1: Share L0, L1, L2 — only L3 entries differ
	// VA bits [47:39]=0, [38:30]=0, [29:21]=0, [20:12]=1,2,3,4
	map_page(root, 0xFFFFF00000001000, 0x1000,
		 (page_permissions_t){ .read = 1, .write = 0, .execute = 0 });
	map_page(root, 0xFFFFF00000002000, 0x2000,
		 (page_permissions_t){ .read = 1, .write = 1, .execute = 0 });
	map_page(root, 0xFFFFF00000003000, 0x3000,
		 (page_permissions_t){ .read = 1, .write = 0, .execute = 1 });
	map_page(root, 0xFFFFF00000004000, 0x4000,
		 (page_permissions_t){ .read = 1, .write = 1, .execute = 1 });

	// Group 2: Different L1 region — forces new L2 table allocation
	// VA bits [47:39]=0, [38:30]=1,2,3,4, [29:21]=0, [20:12]=0
	map_page(root, 0xFFFFF00040000000, 0x5000,
		 (page_permissions_t){ .read = 0, .write = 0, .execute = 1 });
	map_page(root, 0xFFFFF00080000000, 0x6000,
		 (page_permissions_t){ .read = 1, .write = 0, .execute = 0 });
	map_page(root, 0xFFFFF000C0000000, 0x7000,
		 (page_permissions_t){ .read = 0, .write = 1, .execute = 0 });
	map_page(root, 0xFFFFF00100000000, 0x8000,
		 (page_permissions_t){ .read = 1, .write = 1, .execute = 0 });

	// Group 3: Different L0 region — forces new L1 table allocation
	// VA bits [47:39]=1,2,3,4, [38:30]=0, [29:21]=0, [20:12]=0
	map_page(root, 0xFFFFF08000000000, 0x9000,
		 (page_permissions_t){ .read = 0, .write = 0, .execute = 1 });
	map_page(root, 0xFFFFF10000000000, 0xA000,
		 (page_permissions_t){ .read = 1, .write = 0, .execute = 1 });
	map_page(root, 0xFFFFF18000000000, 0xB000,
		 (page_permissions_t){ .read = 0, .write = 1, .execute = 1 });
	map_page(root, 0xFFFFF20000000000, 0xC000,
		 (page_permissions_t){ .read = 1, .write = 1, .execute = 1 });

	dump_memory_map(root);
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

	// NOLINTNEXTLINE(*-int-to-ptr)
	const void *fdt_addr = (const void *)boot_args_ptr[0];
	if (!check_fdt(fdt_addr)) {
		kprintf("FDT validation failed. Halting.\n");
		return;
	}

	Memory_map_t mmap;
	// NOLINTNEXTLINE(*-int-to-ptr)
	if (get_mem(fdt_addr, &mmap) < 0) {
		kprintf("Failed to parse memory map from FDT. Halting.\n");
		return;
	}

	if (mmap.count != 1) {
		kprintf("Current implementation only supports a single memory region. Halting.\n");
		return;
	}

	print_memory_map(&mmap);

	// NOLINTBEGIN(*-int-to-ptr)
	bool page_init_result =
		page_init((void *)mmap.regions[0].base, mmap.regions[0].size);
	// NOLINTEND(*-int-to-ptr)

	if (!page_init_result) {
		kprintf("Failed to initialize page allocator. Halting.\n");
		return;
	}

	if (!reserve_kerenel_img_pages()) {
		kprintf("Error while reserving kernel binary pages\n");
		return;
	}

	page_dump_status();

	page_table_t *root_table = (page_table_t *)page_alloc(1);
	setup_dummy_mappings(root_table);

	kprintf("Hello World!\n");

	// Trigger a breakpoint to test exception handling
	asm volatile("brk #0");

	/* System should not return; if it does, boot.s handles it with a halt
	 * loop.
	 */
}
