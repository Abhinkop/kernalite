/**
 * @file page_table.h
 * @brief AArch64 Page Table definitions for 4KB granule.
 */

#ifndef MMU_PAGE_TABLE_H
#define MMU_PAGE_TABLE_H

#include <stdint.h>

#define PTRS_PER_TABLE 512
#define PAGE_SIZE 4096

/* Page Table Entry Bits */
#define PTE_VALID (1ULL << 0)
#define PTE_TABLE (1ULL << 1) /* Is this a link to a next-level table? */
#define PTE_BLOCK (0ULL << 1) /* Is this a terminal block? (L1/L2 only) */
#define PTE_PAGE (1ULL << 1) /* Is this a terminal 4KB page? (L3 only) */
#define PTE_USER (1ULL << 6) /* Access Permissions: AP[1] */
#define PTE_RO (1ULL << 7) /* Access Permissions: AP[2] */
#define PTE_AF (1ULL << 10) /* Access Flag (must be 1 to avoid fault) */
#define PTE_ISH (3ULL << 8) /* Inner Shareable */
#define PTE_MEMATTR_NC (0ULL << 2) /* Index into MAIR register: Non-cacheable */

/**
 * @struct page_table_t
 * @brief Represents a single page table level (512 entries).
 */
typedef struct {
	uint64_t entries[PTRS_PER_TABLE];
} page_table_t;

/**
 * @brief Map a virtual address to a physical address.
 *
 * @param root Pointer to the L0 table.
 * @param v_addr Virtual address to map.
 * @param phy_addr Physical address to point to.
 * @param flags Attributes (R/W, User/Kernel, etc).
 */
void map_page(page_table_t *root, uintptr_t v_addr, uintptr_t phy_addr,
	      uint64_t flags);

/**
 * @brief Public interface to perform a diagnostic dump of the entire page table.
 *
 * Useful for verifying memory attributes and identity mappings before or after
 * the MMU is enabled. Logs output to the configured serial console.
 *
 * @param root Pointer to the Level 0 (L0) translation table to begin the walk.
 */
void page_table_dump(page_table_t *root);

#endif /* MMU_PAGE_TABLE_H */
