/**
 * @file page_table.c
 * @brief Page table support implementation for kernel virtual memory.
 *
 * This module contains the kernel page table implementation used to
 * manage virtual memory mappings and page table creation routines.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-05-17
 */

#include "page_table/page_table.h"

#include "allocator/page_allocator.h"
#include "utils/kprintf.h"

#include <stddef.h>
#include <stdint.h>

#define PAGE_SHIFT 12U
#define IDX_MASK 0x1FFUL

/**
 * @brief Set the next level table address in a page table entry.
 * @param entry Pointer to the page table entry to modify.
 * @param addr Physical address of the next level page table (must be page
 * aligned).
 */
inline void set_page_table_entry_address(page_table_entry_t *entry,
					 phy_addr addr)
{
	// Mask for bits [47:12]
	const uint64_t addr_mask = 0xFFFFFFFFF000UL;
	entry->nlta_47_12 = (addr & addr_mask) >> PAGE_SHIFT;
}

/**
 * @brief Get the physical address from a page table entry.
 * @param entry Pointer to the page table entry.
 * @return The physical address.
 */
inline phy_addr get_page_table_entry_address(const page_table_entry_t *entry)
{
	// Shift to align address to bits [47:12]
	return ((phy_addr)entry->nlta_47_12 << PAGE_SHIFT);
}

/**
 * @brief Convert a virtual address to a physical address.
 * @param v_addr The virtual address to convert.
 * @return The corresponding physical address.
 */
phy_addr va_to_pa(virt_addr v_addr)
{
	return (phy_addr)v_addr - KERNEL_BASE_VA;
}

/**
 * @brief Convert a physical address to a virtual address.
 * @param p_addr The physical address to convert.
 * @return The corresponding virtual address.
 */
virt_addr pa_to_va(phy_addr p_addr)
{
	return (virt_addr)KERNEL_BASE_VA + p_addr;
}

/**
 * @brief Set the next level table address in a page table entry.
 * @param entry Pointer to the page table entry to modify.
 * @param addr Physical address of the next level page table (must be page
 * aligned).
 */
void set_next_level_table(page_table_entry_t *entry, phy_addr next_table_addr)
{
	set_page_table_entry_address(entry, next_table_addr);
	entry->valid = 1; // Mark as valid
	entry->table = 1; // Mark as table
}

/**
 * @brief Allocate a new page table and set it as the next level table in the
 * entry.
 * @param entry Pointer to the page table entry to modify.
 * @return true if successful, false otherwise.
 */
bool allocate_new_table(page_table_entry_t *entry)
{
	page_table_t *new_table = (page_table_t *)page_alloc(1);
	if (new_table == NULL) {
		kprintf("Failed to allocate new page table\n");
		return false;
	}
	page_table_init(new_table);
	phy_addr phys_addr = va_to_pa((virt_addr)new_table);
	set_next_level_table(entry, phys_addr);
	return true;
}

/**
 * @brief Get the next level table from a page table entry.
 * @param entry Pointer to the page table entry.
 * @return Pointer to the next level page table, or NULL if allocation failed.
 */
page_table_t *get_next_level_table(page_table_entry_t *entry)
{
	if (!entry->valid && !allocate_new_table(entry)) {
		return NULL;
	}

	phy_addr next_table_addr = get_page_table_entry_address(entry);

	// NOLINTNEXTLINE(*-int-to-ptr)
	return (page_table_t *)pa_to_va(next_table_addr);
}

/**
 * @brief Recursively dump the memory map for debugging.
 * @param root Pointer to the root page table to dump.
 * @param level The current level of the page table.
 * @param v_addr_prefix The virtual address prefix for the current level.
 */
// NOLINTNEXTLINE(misc-no-recursion)
void dump_memory_map_recursive(page_table_t *root, uint64_t level,
			       uint64_t v_addr_prefix)
{
	// 9 bits per level, starting from L0 and 12 bits for page offset
	const uint8_t level_shift = (9 * (3 - level)) + 12;
	// 9 bits mask for the current level index
	const uint64_t level_mask = IDX_MASK << level_shift;

	for (uint64_t i = 0; i < 512; i++) {
		page_table_entry_t entry = root->entries[i];

		if (!entry.valid) {
			continue;
		}

		uint64_t partial_v_addr = (i << level_shift) |
					  (v_addr_prefix & ~(level_mask));
		if (level < 3) {
			// NOLINTNEXTLINE(*-int-to-ptr)
			page_table_t *next_table = get_next_level_table(&entry);
			dump_memory_map_recursive(next_table, level + 1,
						  partial_v_addr);
		} else {
			phy_addr phys_addr =
				get_page_table_entry_address(&entry);
			kprintf("VA 0x%lx -> PA 0x%lx [Flags: 0x%lx]",
				partial_v_addr, phys_addr, entry.value & 0xFFF);
			if (entry.xn_table) {
				kprintf(" XN");
			}

			if (entry.pxn_table) {
				kprintf(" PXN");
			}

			if (entry.ap_table ==
			    APTABLE_EL0_NO_RESTRICTION_EL1_NO_RESTRICTION) {
				kprintf(" RW(EL0) RW(EL1)");
			} else if (entry.ap_table ==
				   APTABLE_EL0_NO_WRITE_EL1_NO_WRITE) {
				kprintf(" RO(EL0) RO(EL1)");
			} else if (entry.ap_table ==
				   APTABLE_EL0_NO_ACCESS_EL1_NO_RESTRICTION) {
				kprintf(" NO-ACCESS(EL0) RW(EL1)");
			} else if (entry.ap_table ==
				   APTABLE_EL0_NO_ACCESS_EL1_NO_WRITE) {
				kprintf(" NO-ACCESS(EL0) RO(EL1)");
			}

			kprintf("\n");
		}
	}
}

void page_table_init(page_table_t *table)
{
	for (size_t i = 0; i < PTRS_PER_TABLE; i++) {
		table->entries[i].value = 0;
	}
}

bool map_page(page_table_t *root, const virt_addr v_addr,
	      const phy_addr phy_addr, const page_permissions_t perms)
{
	if (phy_addr & 0xFFF) {
		kprintf("Error: Physical address 0x%lx is not page aligned\n",
			phy_addr);
		return false;
	}

	// Get Indices
	const uint64_t l0_idx_start_bit = 39;
	const uint64_t l1_idx_start_bit = 30;
	const uint64_t l2_idx_start_bit = 21;
	const uint64_t l3_idx_start_bit = 12;
	const uint64_t idx_mask = IDX_MASK; // 9 bits for each level index

	const uint64_t l0_idx = (v_addr >> l0_idx_start_bit) & idx_mask;
	const uint64_t l1_idx = (v_addr >> l1_idx_start_bit) & idx_mask;
	const uint64_t l2_idx = (v_addr >> l2_idx_start_bit) & idx_mask;
	const uint64_t l3_idx = (v_addr >> l3_idx_start_bit) & idx_mask;

	page_table_t *level1 = get_next_level_table(&root->entries[l0_idx]);
	if (!level1) {
		kprintf("Failed to get level 1 page table for VA 0x%lx\n",
			v_addr);
		return false;
	}
	page_table_t *level2 = get_next_level_table(&level1->entries[l1_idx]);
	if (!level2) {
		kprintf("Failed to get level 2 page table for VA 0x%lx\n",
			v_addr);
		return false;
	}
	page_table_t *level3 = get_next_level_table(&level2->entries[l2_idx]);
	if (!level3) {
		kprintf("Failed to get level 3 page table for VA 0x%lx\n",
			v_addr);
		return false;
	}

	level3->entries[l3_idx].value = 0;
	set_page_table_entry_address(&level3->entries[l3_idx], phy_addr);
	level3->entries[l3_idx].valid = 1; // Mark as valid
	// TODO: Need to find a beter name for .table bit in L3 entries.
	// This is a bit of a hack to reuse the same struct for both table and
	// block/page descriptors. on L3, this bit needs to for 4kb page
	level3->entries[l3_idx].table = 1;

	if (perms.execute) {
		// Clear XN bit to allow execution
		level3->entries[l3_idx].xn_table = 0;
		// Clear UXN bit to allow execution at EL0
		level3->entries[l3_idx].pxn_table = 0;
	} else {
		// Set XN bit to prevent execution
		level3->entries[l3_idx].xn_table = 1;
		// Set UXN bit to prevent execution at EL0
		level3->entries[l3_idx].pxn_table = 1;
	}

	if (perms.write) {
		// Allow read/write at EL0 and EL1
		level3->entries[l3_idx].ap_table =
			APTABLE_EL0_NO_RESTRICTION_EL1_NO_RESTRICTION;
	} else {
		// Allow read-only at EL0 and EL1
		level3->entries[l3_idx].ap_table =
			APTABLE_EL0_NO_WRITE_EL1_NO_WRITE;
	}

	if (!perms.read) {
		// No access at EL0, allow read/write at EL1
		level3->entries[l3_idx].ap_table =
			APTABLE_EL0_NO_ACCESS_EL1_NO_WRITE;
	}
	return true;
}

void dump_memory_map(page_table_t *root)
{
	kprintf("--- Memory Map Dump Start ---\n");
	dump_memory_map_recursive(root, 0, 0);
	kprintf("--- Memory Map Dump End ---\n");
}
