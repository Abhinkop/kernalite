/**
 * @file page_table.h
 * @brief Page table definitions and helpers for virtual memory mapping.
 *
 * Provides architecture-specific macros and declarations for building and
 * manipulating page tables used by the kernel's memory management.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-05-17
 */

#ifndef PAGE_TABLE_PAGE_TABLE_H
#define PAGE_TABLE_PAGE_TABLE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Size of each memory page in bytes.
 * from  table D4-20 in the Armv8-A architecture reference manual.
 */
#define PAGE_SIZE 4096

/**
 * @brief Number of pointers per page table.
 * from  table D4-21 in the Armv8-A architecture reference manual.
 */
#define PTRS_PER_TABLE 512

/**
 * @brief Base virtual address for the kernel.
 * @note The actual base is 0xFFFF000000000000UL,
 *       but we use 0 untill we map kernel to high memory.
 */
#define KERNEL_BASE_VA 0x0

/** @brief Type for representing physical addresses. */
typedef uint64_t phy_addr;

/** @brief Type for representing virtual addresses. */
typedef uint64_t virt_addr;

/** @brief Type for representing page permissions. */
typedef struct page_permissions {
	bool read : 1;
	bool write : 1;
	bool execute : 1;
} page_permissions_t;

/**
 * @brief AArch64 VMSAv8-64 Stage 1 Table Descriptor.
 *
 * Represents a page table entry at lookup levels 0, 1, or 2 that points
 * to the next level page table. Valid only when bit[0]=1 and bit[1]=1.
 *
 * The layout depends on the translation granule and whether 52-bit physical
 * addresses are enabled (TCR_ELx.DS=1 with FEAT_LPA).
 *
 * Reference: ARM DDI 0487 Table D8-50
 * "Stage 1 VMSAv8-64 Table descriptor fields"
 */
typedef struct {
	union {
		uint64_t value; /**< Raw 64-bit value of the descriptor. */
		struct {
			/**
             * @brief [0] Valid descriptor flag.
             * Must be 1 for this descriptor to be valid.
             * If 0, any access through this entry will fault.
             */
			uint64_t valid : 1;

			/**
             * @brief [1] Table descriptor flag.
             * Must be 1 (at levels 0-2) to indicate this is a table descriptor
             * pointing to the next level page table.
             * If 0, this is a block descriptor (maps a large region directly).
             */
			uint64_t table : 1;

			/**
             * @brief [7:2] Ignored.
             * These bits are ignored by hardware and available for software use.
             */
			uint64_t ignored_7_2 : 6;

			/**
            * @brief [9:8] Next Level Table Address bits [51:50].
            * - TCR_ELx.DS=0 (48-bit PA): IGNORED.
            * - TCR_ELx.DS=1 (52-bit PA, FEAT_LPA) with 4KB or 16KB granule: NLTA[51:50].
            *   These bits extend the next level table address from 48 to 52 bits.
            *   Not used with 64KB granule as it uses a different encoding (see [47:12]).
            */
			uint64_t nlta_51_50 : 2;

			/**
            * @brief [10] Table descriptor Access Flag (AF).
            * - Hardware managed AF disabled: IGNORED.
            * - Hardware managed AF enabled (FEAT_HAFDBS): set by hardware on first
            *   access to this table descriptor itself.
            *
            * Note: This is distinct from the AF in page/block descriptors (L3),
            * which tracks access to the mapped page. The AF here tracks access
            * to the table descriptor itself. The OS can clear AF and use it to
            * implement LRU-based TLB eviction — if AF is still 0 after some period,
            * the table has not been accessed and is a candidate for reclamation.
            */
			uint64_t access_flag : 1;

			/**
             * @brief [11] Ignored.
             * Ignored by hardware, available for software use.
             */
			uint64_t ignored_11 : 1;

			/**
             * @brief [47:12] Next Level Table Address bits [47:12].
             * Physical address of the next level page table (page aligned).
             * Exact bits used depend on granule size:
             * - 4KB  granule: NLTA[47:12] (all 36 bits used)
             * - 16KB granule: NLTA[47:14] (bits [13:12] are RES0)
             * - 64KB granule: NLTA[47:16] (bits [15:12] are RES0)
             */
			uint64_t nlta_47_12 : 36;

			/**
             * @brief [49:48] Next Level Table Address bits [49:48].
             * - DS=0 or 64KB granule: RES0, must be zero.
             * - DS=1 with 4KB or 16KB granule (FEAT_LPA): NLTA[49:48].
             *   Extends the next level table address beyond 48 bits.
             */
			uint64_t nlta_49_48 : 2;

			/**
             * @brief [50] Reserved, RES0.
             * Must be written as zero, reserved for future use.
             */
			uint64_t res0_50 : 1;

			/**
             * @brief [51] Ignored.
             * Ignored by hardware, available for software use.
             */
			uint64_t ignored_51 : 1;

			/**
             * @brief [52] Stage 1 Protected Attribute.
             * - PnCH=0: IGNORED.
             * - PnCH=1: marks all memory reachable through this entry as
             *   Stage 1 Protected. Used for security-sensitive regions such
             *   as cryptographic key storage. Affects cacheability and
             *   access behavior of subordinate entries.
             */
			uint64_t protected_attr : 1;

			/**
             * @brief [58:53] Ignored.
             * Ignored by hardware, available for software use.
             */
			uint64_t ignored_58_53 : 6;

			/**
             * @brief [59] Privileged Execute-Never Table bit (PXNTable).
             * Hierarchical control — applies to all pages reachable below.
             * - Two privilege levels: PXNTable.
             *   If 1, execution at EL1 (privileged) is not permitted for
             *   any page reachable through this table entry.
             * - Single privilege level or NV/NV1={1,1}: RES0, must be zero.
             */
			uint64_t pxn_table : 1;

			/**
             * @brief [60] Execute-Never Table bit (XNTable / UXNTable / PXNTable).
             * Hierarchical control — applies to all pages reachable below.
             * Interpretation depends on translation regime:
             * - Single privilege level:        XNTable.
             *   If 1, execution is not permitted at any level.
             * - Two privilege levels (EL0+EL1): UXNTable.
             *   If 1, execution at EL0 (unprivileged) is not permitted.
             * - EL1&0 with NV/NV1={1,1}:       PXNTable.
             *   If 1, execution at EL1 (privileged) is not permitted.
             */
			uint64_t xn_table : 1;

			/**
             * @brief [62:61] Access Permission Table bits (APTable[1:0]).
             * Hierarchical control — restricts access permissions for all
             * pages reachable through this table entry. Only applies when
             * hierarchical permissions are enabled.
             * @see aptable_values_t for the encoding of these bits.
             */
			uint64_t ap_table : 2;

			/**
             * @brief [63] Non-Secure Table bit (NSTable).
             * - Secure state: NSTable.
             *   If 1, all memory reachable through this entry is Non-Secure.
             *   If 0, memory is Secure.
             * - Non-Secure state: RES0, must be zero.
             *   (No Secure/Non-Secure distinction exists outside Secure state.)
             */
			uint64_t ns_table : 1;
		} __attribute__((packed));
	};
} page_table_entry_t;

/**
 * @brief Access Permission Table bits (APTable[1:0]).
 * Hierarchical control that restricts access permissions for all pages
 * reachable through a table descriptor entry.
 *
 * +-------+----------------------------+----------------------------+
 * | Value | EL0 (Unprivileged)         | EL1 (Privileged)           |
 * +-------+----------------------------+----------------------------+
 * | 0b00  | No restriction             | No restriction             |
 * | 0b01  | No write                   | No write                   |
 * | 0b10  | No access                  | No restriction             |
 * | 0b11  | No access                  | No write                   |
 * +-------+----------------------------+----------------------------+
 *
 * Note: APTable[0] is treated as 0 when HCR_EL2.{NV,NV1}={1,1}.
 */
typedef enum aptable_values {
	APTABLE_EL0_NO_RESTRICTION_EL1_NO_RESTRICTION = 0b00,
	APTABLE_EL0_NO_WRITE_EL1_NO_WRITE = 0b01,
	APTABLE_EL0_NO_ACCESS_EL1_NO_RESTRICTION = 0b10,
	APTABLE_EL0_NO_ACCESS_EL1_NO_WRITE = 0b11
} aptable_values;

/**
 * @struct page_table_t
 * @brief Represents a single page table level (512 entries).
 */
typedef struct {
	page_table_entry_t entries[PTRS_PER_TABLE];
} page_table_t;

/**
 * @brief Initialize a page table.
 * @param table Pointer to the page table to initialize.
 */
void page_table_init(page_table_t *table);

/**
 * @brief Map a virtual address to a physical address in the page table.
 * @param root Pointer to the root page table.
 * @param v_addr The virtual address to map.
 * @param phy_addr The physical address to map to.
 * @param perms The permissions for the page table entry.
 * @return true on success, false on failure (e.g., if allocation fails).
 */
bool map_page(page_table_t *root, virt_addr v_addr, phy_addr phy_addr,
	      page_permissions_t perms);

/**
 * @brief Dump the memory map for debugging.
 * @param root Pointer to the root page table to dump.
 */
void dump_memory_map(page_table_t *root);

#endif /* PAGE_TABLE_PAGE_TABLE_H */
