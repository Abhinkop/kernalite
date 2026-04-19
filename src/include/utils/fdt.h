/**
 * @file fdt.h
 * @brief Device Tree parsing utilities for memory discovery.
 *
 * Provides structures and functions to extract hardware information 
 * from the Flattened Device Tree (FDT) passed by the bootloader.
 */

#ifndef UTILS_FDT_H
#define UTILS_FDT_H

#include <stddef.h>
#include <stdint.h>

/** @brief Maximum number of distinct memory regions the kernel will track. */
#define MAX_MEM_REGIONS 16

/**
 * @brief Represents a single physical memory span.
 */
typedef struct {
    uint64_t base; /**< Start physical address of the region. */
    uint64_t size; /**< Size of the region in bytes. */
} Memory_region_t;

/**
 * @brief Container for the system physical memory layout.
 */
typedef struct {
    Memory_region_t regions[MAX_MEM_REGIONS]; /**< Array of discovered regions. */
    int count; /**< Actual number of regions populated. */
} Memory_map_t;

/**
 * @brief Scans the FDT for memory nodes and populates the provided map.
 * * This function searches the FDT for nodes with `device_type = "memory"`,
 * respects the root node's `#address-cells` and `#size-cells`, and 
 * handles both 32-bit and 64-bit address formats.
 *
 * @param fdt Pointer to the FDT header in memory.
 * @param mmap Pointer to a Memory_map_t structure to be populated.
 * @return 0 on success, or a negative libfdt error code on failure.
 */
int get_mem(const void *fdt, Memory_map_t *mmap);

#endif /* UTILS_FDT_H */
