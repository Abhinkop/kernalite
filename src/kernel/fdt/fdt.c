/**
 * @file fdt.c
 * @brief Implementation of Device Tree Blob (FDT) parsing and validation functions.
 *
 * Validates the FDT header, extracts memory map entries, and exposes kernel
 * helpers for consuming device tree data.
 *
 * @author Abhin Parekadan Jose
 * @date 2026-05-16
 */

#include "fdt/fdt.h"

#include "utils/kprintf.h"

#include <libfdt.h>

bool check_fdt(const void *ptr)
{
	if (!ptr) {
		kprintf("FDT Error: null DTB address\n");
		return false;
	}

	int err = fdt_check_header(ptr);
	switch (err) {
	case 0:
		kprintf("Valid fdt\n");
		return true;
	case -FDT_ERR_BADMAGIC:
		kprintf("FDT Error: Bad magic number\n");
		break;
	case -FDT_ERR_BADVERSION:
		kprintf("FDT Error: Unsupported FDT version\n");
		break;
	case -FDT_ERR_BADSTATE:
		kprintf("FDT Error: Bad state\n");
		break;
	default:
		kprintf("FDT Error: Unknown error code %d\n", err);
		break;
	}
	return false;
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
int get_mem(const void *fdt, Memory_map_t *mmap)
{
	if (!fdt || !mmap) {
		return -FDT_ERR_INTERNAL;
	}

	int node = -1;
	mmap->count = 0;

	/* Find the root node to determine address/size cell requirements */
	int root = fdt_path_offset(fdt, "/");
	if (root < 0) {
		return root;
	}

	int address_cell = fdt_address_cells(fdt, root);
	int size_cell = fdt_size_cells(fdt, root);

	if (address_cell < 1 || address_cell > 2 || size_cell < 1 ||
	    size_cell > 2) {
		kprintf("FDT Error: unsupported address/size cell counts %d/%d\n",
			address_cell, size_cell);
		return -FDT_ERR_BADSTATE;
	}

	/* Search for nodes marked with device_type = "memory".
     * Standard FDT path is /memory, but some systems use /memory@0 or others.
     */
	while ((node = fdt_node_offset_by_prop_value(fdt, node, "device_type",
						     "memory", 7)) >= 0) {
		int len;
		const fdt32_t *reg = fdt_getprop(fdt, node, "reg", &len);

		if (!reg || len <= 0) {
			continue;
		}

		size_t entry_size =
			(size_t)(address_cell + size_cell) * sizeof(fdt32_t);
		if ((size_t)len < entry_size || (len % entry_size) != 0) {
			kprintf("FDT Error: memory \"reg\" length %d is not a multiple of entry size %zu\n",
				len, entry_size);
			return -FDT_ERR_BADSTATE;
		}

		size_t num_entries = (size_t)len / entry_size;

		for (size_t i = 0; i < num_entries; i++) {
			if (mmap->count >= MAX_MEM_REGIONS) {
				break;
			}

			uint64_t base = 0;
			uint64_t size = 0;
			const fdt32_t *ptr =
				reg + (i * (address_cell + size_cell));

			/* Extract address (handles varying cell counts) */
			for (unsigned long j = 0;
			     j < (unsigned long)(address_cell); j++) {
				base = (base << 32) | fdt32_to_cpu(ptr[j]);
			}

			/* Extract size (handles varying cell counts) */
			for (unsigned long j = 0;
			     j < (unsigned long)(size_cell); j++) {
				size = (size << 32) |
				       fdt32_to_cpu(ptr[address_cell + j]);
			}

			/* Ignore zero-sized regions if they appear in the DT */
			if (size > 0) {
				mmap->regions[mmap->count].base = base;
				mmap->regions[mmap->count].size = size;
				mmap->count++;
			}
		}
	}

	return 0;
}
// NOLINTEND(readability-function-cognitive-complexity)
