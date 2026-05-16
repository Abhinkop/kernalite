#include "mmu/mmu.h"

#include "mmu/page_table.h"

/**
 * @brief Global physical memory offset used for linear address translation.
 * * This variable stores the base physical address of RAM (e.g., 0x40000000 on QEMU virt).
 * It is initialized dynamically from the Device Tree (FDT) during early boot.
 * All pa_to_va and va_to_pa calculations depend on this value being set correctly
 * before the MMU is enabled.
 */
uintptr_t g_phys_offset = 0;
