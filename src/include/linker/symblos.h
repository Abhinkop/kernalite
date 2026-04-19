/**
 * @file symblos.h
 * @brief Symbol definitions for kernel initialization and memory management.
 * @details This header defines the symbols that are used to interface between
 * the assembly bootstrap code and the C environment of the kernel. It includes
 * the storage for bootloader arguments and references to the memory regions
 * reserved for the initial page tables as defined in the linker script.
 * * @author Abhin Parekadan Jose
 */

#ifndef UTILS_SYMBOLS_H
#define UTILS_SYMBOLS_H

#include <stdint.h>

/**
 * @name Bootloader Arguments
 * @{
 */

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

/** @} */

/**
 * @name MMU Initialization Symbols
 * @{
 */

/**
 * @brief Start of the identity map page directory.
 * * This symbol marks the base of the memory region reserved in the
 * linker script for the initial Level 0 translation table.
 * @note This address must be 4KB (granule) aligned.
 */
extern void *init_idmap_pg_dir;

/**
 * @brief End of the identity map page directory region.
 * * Used to calculate the total size of the reserved page table area
 * for initialization (e.g., zeroing memory via memset) and bounds checking.
 */
extern void *init_idmap_pg_end;

/**
 * @brief The starting address of the kernel image.
 * Matches the beginning of the .text section.
 */
extern uint8_t kernel_start;

/**
 * @brief The end address of the kernel image.
 * This is typically placed after the .bss section and any alignment padding.
 * Memory after this address is generally considered available for the page allocator.
 */
extern uint8_t kernel_end;

/** @} */

#endif /* UTILS_SYMBOLS_H */
