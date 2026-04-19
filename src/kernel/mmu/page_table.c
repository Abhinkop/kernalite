#include "mmu/page_table.h"

#include "allocator/page_allocater.h"
#include "utils/printf.h"
#include "string.h"

#include <stddef.h>

/**
 * @brief Recursively traverses the translation tables to log all active mappings.
 *
 * Performs a depth-first search through the page table hierarchy. It identifies
 * whether an entry is a pointer to another table (PTE_TABLE) or a terminal
 * mapping (Block/Page), and calculates the virtual address based on the current
 * level's bit-width.
 *
 * @param table    Pointer to the current page table level being inspected.
 * @param level    The current translation level (0=L0, 1=L1, 2=L2, 3=L3).
 * @param base_va  The accumulated virtual address base for the current table.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters, misc-no-recursion)
void page_table_dump_recursive(page_table_t *table, int level,
			       uintptr_t base_va)
{
	for (int i = 0; i < 512; i++) {
		uint64_t entry = table->entries[i];

		if (!(entry & PTE_VALID)) {
			continue;
		}

		// Calculate the virtual address range this entry covers
		// Level 0 = 512GB, L1 = 1GB, L2 = 2MB, L3 = 4KB
		uintptr_t step = 1ULL << (39 - (level * 9));
		uintptr_t v_addr = base_va + (i * step);

		// Print indentation based on level
		for (int space = 0; space < level; space++)
			printf("  ");

		if (level < 3 && (entry & PTE_TABLE)) {
			printf("L%d Entry %d: VA 0x%lx -> Table at 0x%lx\n",
			       level, i, v_addr,
			       (uintptr_t)(entry & ~0xFFFULL));

			page_table_t *next_table =
				// NOLINTNEXTLINE(*-int-to-ptr)
				(page_table_t *)(entry & ~0xFFFULL);
			page_table_dump_recursive(next_table, level + 1,
						  v_addr);
		} else {
			printf("L%d [MAP] VA 0x%lx -> PA 0x%lx (Flags: 0x%lx)\n",
			       level, v_addr, (uintptr_t)(entry & ~0xFFFULL),
			       (uint64_t)(entry & 0xFFF));
		}
	}
}

/**
 * @brief Map a virtual address to a physical address.
 * * @param root Pointer to the L0 table.
 * @param v_addr   Virtual address to map.
 * @param phy_addr   Physical address to point to.
 * @param flags Attributes (R/W, User/Kernel, etc).
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void map_page(page_table_t *root, uintptr_t v_addr, uintptr_t phy_addr,
	      uint64_t flags)
{
	// Get Indices
	size_t l0_idx = (v_addr >> 39) & 0x1FF;
	size_t l1_idx = (v_addr >> 30) & 0x1FF;
	size_t l2_idx = (v_addr >> 21) & 0x1FF;
	size_t l3_idx = (v_addr >> 12) & 0x1FF;

	// --- Level 0 -> Level 1 ---
	if (!(root->entries[l0_idx] & PTE_VALID)) {
		page_table_t *new_tab = (page_table_t *)page_alloc(1);
		memset(new_tab, 0, PAGE_SIZE);
		for (int i = 0; i < 512; i++)
			new_tab->entries[i] = 0;
		root->entries[l0_idx] = (uintptr_t)new_tab | PTE_TABLE |
					PTE_VALID;
	}
	page_table_t *level1 =
		// NOLINTNEXTLINE(*-int-to-ptr)
		(page_table_t *)(root->entries[l0_idx] & ~0xFFFULL);

	// --- Level 1 -> Level 2 ---
	if (!(level1->entries[l1_idx] & PTE_VALID)) {
		page_table_t *new_tab = (page_table_t *)page_alloc(1);
		memset(new_tab, 0, PAGE_SIZE);
		for (int i = 0; i < 512; i++)
			new_tab->entries[i] = 0;
		level1->entries[l1_idx] = (uintptr_t)new_tab | PTE_TABLE |
					  PTE_VALID;
	}
	page_table_t *level2 =
		// NOLINTNEXTLINE(*-int-to-ptr)
		(page_table_t *)(level1->entries[l1_idx] & ~0xFFFULL);

	// --- Level 2 -> Level 3 ---
	if (!(level2->entries[l2_idx] & PTE_VALID)) {
		page_table_t *new_tab = (page_table_t *)page_alloc(1);
		memset(new_tab, 0, PAGE_SIZE);
		for (int i = 0; i < 512; i++)
			new_tab->entries[i] = 0;
		level2->entries[l2_idx] = (uintptr_t)new_tab | PTE_TABLE |
					  PTE_VALID;
	}
	page_table_t *level3 =
		// NOLINTNEXTLINE(*-int-to-ptr)
		(page_table_t *)(level2->entries[l2_idx] & ~0xFFFULL);

	// --- Level 3: The actual mapping ---
	// We map the physical address and set the terminal "PAGE" bit
	level3->entries[l3_idx] = (phy_addr & ~0xFFFULL) | flags | PTE_PAGE |
				  PTE_VALID | PTE_AF;
}

/**
 * @brief Public interface to perform a diagnostic dump of the entire page table.
 *
 * Useful for verifying memory attributes and identity mappings before or after
 * the MMU is enabled. Logs output to the configured serial console.
 *
 * @param root Pointer to the Level 0 (L0) translation table to begin the walk.
 */
void page_table_dump(page_table_t *root)
{
	printf("--- Page Table Dump Start ---\n");
	page_table_dump_recursive(root, 0, 0);
	printf("--- Page Table Dump End ---\n");
}
