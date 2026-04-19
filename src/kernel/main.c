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
#include "mmu/page_table.h"

uart_device_t uart0; // Global UART device instance for early boot logging

// NOLINTNEXTLINE(misc-include-cleaner)
void putchar(char chr)
{
	// NOLINTNEXTLINE(misc-include-cleaner)
	uart0.putc(&uart0, chr);
}

/**
 * @brief Creates an identity mapping (VA = PA) for a range of memory.
 * * This is used to ensure that the CPU can continue executing code seamlessly
 * when the MMU is enabled, and to provide access to physical RAM and MMIO.
 * * @param root   Pointer to the L0 page table.
 * @param start  The starting physical address.
 * @param size   The size of the region in bytes.
 * @param flags  Memory attributes (e.g., PTE_AF, PTE_MEMATTR_NC).
 */
void setup_identity_map(page_table_t *root, uintptr_t start, size_t size,
			uint64_t flags)
{
	// Align start down to page boundary
	uintptr_t addr = start & ~0xFFFULL;
	// Align end up to page boundary
	uintptr_t end = (start + size - 1 + 4095) & ~0xFFFULL;

	printf("MMU setup_identity_map: Identity mapping 0x%lx - 0x%lx\n", addr,
	       end);

	for (; addr < end; addr += PAGE_SIZE) {
		map_page(root, addr, addr, flags);
	}
}

void setup_mair()
{
	// Attribute 0: Device-nGnRE (standard for MMIO/UART)
	// Attribute 1: Normal Memory, Outer/Inner Write-Back Non-transient
	uint64_t mair = (0x00 << 0) | (0xFF << 8);
	asm volatile("msr mair_el1, %0" : : "r"(mair));
}

void setup_tcr()
{
	uint64_t tcr = 0;
	tcr |= (16ULL << 0); // T0SZ: 48-bit VA
	tcr |= (3ULL
		<< 8); // IRGN0: Inner Write-Back Read-Alloc Write-Alloc Cacheable
	tcr |= (3ULL
		<< 10); // ORGN0: Outer Write-Back Read-Alloc Write-Alloc Cacheable
	tcr |= (3ULL << 12); // TG0: 4KB granule
	tcr |= (3ULL << 14); // SH0: Inner Shareable
	tcr |= (2ULL << 32); // IPS: 40-bit PA

	// Recommended: Disable TTBR1 to prevent the MMU from hunting in upper addresses
	tcr |= (1ULL << 23); // EPD1: Disable table walks for TTBR1

	asm volatile("msr tcr_el1, %0" : : "r"(tcr));
	asm volatile("isb");
}

/**
 * @brief Enables the MMU and transitions to virtual addressing.
 * @param l0_phys_addr The physical address of the L0 page table.
 */
void enable_mmu(uintptr_t l0_phys_addr)
{
	setup_mair();
	setup_tcr();

	// Set the table pointer
	asm volatile("msr ttbr0_el1, %0" : : "r"(l0_phys_addr));
	asm volatile("isb");

	// Read System Control Register
	uint64_t sctlr;
	asm volatile("mrs %0, sctlr_el1" : "=r"(sctlr));

	// Enable MMU (bit 0) and Data Cache (bit 2) and Instruction Cache (bit 12)
	sctlr |= (1 << 0) | (1 << 2) | (1 << 12);
	sctlr &= ~(
		1ULL
		<< 1); // Clear A (Alignment check) - optional but safer for debug
	sctlr &= ~(
		1ULL
		<< 25); // Clear EE (Exception Endianness) - ensure Little Endian
	sctlr |= (1ULL << 0); // Set M (MMU Enable)

	printf("MMU: Enabling... Identity mapping ensures we don't crash here.\n");

	asm volatile("dsb sy\n\t" // Complete all memory writes
		     "msr sctlr_el1, %0\n\t" // Enable MMU
		     "isb\n\t" // Flush pipeline
		     :
		     : "r"(sctlr)
		     : "memory");

	// The ISB ensures all instructions following this see the MMU as ON
	asm volatile("isb");

	printf("MMU: Successfully enabled and running in Virtual Memory!\n");
}

void init_mmu(uint64_t mem_start, size_t mem_size)
{
	void *l0_buffer = page_alloc(1);
	if (!l0_buffer) {
		printf("MMU ERROR: Failed to allocate L0 root table!\n");
		return;
	}

	for (char *ptr = (char *)l0_buffer; ptr < (char *)l0_buffer + PAGE_SIZE;
	     ptr++) {
		*ptr = 0;
	}

	uintptr_t l0_phys_addr = (uintptr_t)l0_buffer;
	page_table_t *kernel_l0 = (page_table_t *)l0_phys_addr;

	for (int i = 0; i < 512; i++) {
		kernel_l0->entries[i] = 0;
	}

	uintptr_t k_start = (uintptr_t)mem_start;
	uintptr_t k_end = (uintptr_t)&kernel_end;
	size_t k_size =
		k_end - k_start + (2 *PAGE_SIZE); // Ensure we cover the entire kernel with page alignment
	k_size &= ~(PAGE_SIZE - 1); // Align to page boundary

	if (k_size > mem_size) {
		printf("MMU WARNING: Kernel size (0x%lx) exceeds available memory (0x%lx). Truncating kernel mapping.\n",
		       k_size, mem_size);
		k_size = mem_size;
	}

	printf("MMU: Identity mapping kernel: 0x%lx - 0x%lx\n", k_start, k_end);
	setup_identity_map(kernel_l0, k_start, k_size, PTE_AF | (1ULL << 2));

	printf("MMU: Identity mapping uart: 0x%lx - 0x%lx\n", 0x09000000,
	       0x09000000 + PAGE_SIZE);
	setup_identity_map(kernel_l0, 0x09000000, PAGE_SIZE,
			   PTE_AF | PTE_MEMATTR_NC);

	page_table_dump(kernel_l0);

	enable_mmu(l0_phys_addr);
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
	Memory_map_t mmap;

	// NOLINTNEXTLINE(*-int-to-ptr)
	int ret = get_mem((const void *)boot_args_ptr[0], &mmap);
	if (ret < 0) {
		printf("Failed to parse FDT memory map\n");
		return;
	}

	printf("Memory Map (%d regions):\n", mmap.count);
	printf("Region %d: Base=0x%lx, Size=0x%lx\n", 0,
	       (unsigned long)mmap.regions[0].base,
	       (unsigned long)mmap.regions[0].size);

	uintptr_t region_base = (uintptr_t)mmap.regions[0].base;
	uintptr_t region_size = (uintptr_t)mmap.regions[0].size;
	uintptr_t region_end = region_base + region_size;
	uintptr_t k_end = (uintptr_t)&kernel_end;
	if (region_end > k_end) {
		uintptr_t alloc_start = (region_base > k_end) ? region_base :
								k_end;
		size_t alloc_size = region_end - alloc_start;

		printf("Initializing allocator for Region %d:\n", 0);
		printf("  Start: 0x%lx, Size: 0x%lx\n", alloc_start,
		       alloc_size);

		// NOLINTNEXTLINE(*-int-to-ptr)
		page_init((void *)alloc_start, alloc_size);
	}

	init_mmu(region_base, region_size);
	page_dump_status();

	printf("Hello World!\n");

	/* System should not return; if it does, boot.s handles it with a halt loop.
   */
}
