/**
 * @file check.h
 * @brief Header that interfaces with libfdt for Device Tree Blob (DTB) parsing
 * and validation.
 */

#ifndef FDT_CHECK_H
#define FDT_CHECK_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Check the validity of the Device Tree Blob (FDT).
 * @param phys_addr The physical address of the FDT.
 * @return true if the FDT is valid, false otherwise.
 */
bool check_fdt(uintptr_t phys_addr);

#endif // FDT_CHECK_H
