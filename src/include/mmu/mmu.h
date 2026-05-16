/**
 * @file mmu.h
 * @brief Memory Management Unit utilities with dynamic physical offsetting.
 */

#ifndef MMU_MMU_H
#define MMU_MMU_H

#include <stdint.h>

/**
 * @brief The starting virtual address of the kernel space (TTBR1).
 */
#define KERNEL_VIRT_OFFSET 0xFFFF000000000000ULL

/**
 * @brief Global variable storing the start of physical RAM.
 * Initialized from the Device Tree during early boot.
 */
extern uintptr_t g_phys_offset;

/**
 * @brief Converts a physical address to a virtual address.
 * @param phy_addr The physical address to be converted.
 * @return Virtual address pointer.
 */
static inline void *pa_to_va(uintptr_t phy_addr)
{
    //NOLINTNEXTLINE(*-int-to-ptr)
	return (void *)(phy_addr + (KERNEL_VIRT_OFFSET - g_phys_offset));
}

/**
 * @brief Converts a virtual address to a physical address.
 * @param va The virtual address pointer.
 * @return Physical address.
 */
static inline uintptr_t va_to_pa(void *v_addr)
{
	return (uintptr_t)v_addr - (KERNEL_VIRT_OFFSET - g_phys_offset);
}

#endif /* MMU_MMU_H */
