/**
 * @file fdt.c
 * @brief Implementation of FDT memory discovery.
 */

#include "utils/fdt.h"

#include <libfdt.h>
#include <stdint.h>

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

	unsigned long address_cell = fdt_address_cells(fdt, root);
	unsigned long size_cell = fdt_size_cells(fdt, root);

	/* * Search for nodes marked with device_type = "memory".
     * Standard FDT path is /memory, but some systems use /memory@0 or others.
     */
	while ((node = fdt_node_offset_by_prop_value(fdt, node, "device_type",
						     "memory", 7)) >= 0) {
		int len;
		const fdt32_t *reg = fdt_getprop(fdt, node, "reg", &len);

		if (!reg || len <= 0) {
			continue;
		}

		unsigned long entry_size =
			(address_cell + size_cell) * sizeof(fdt32_t);
		unsigned long num_entries = len / entry_size;

		for (unsigned long i = 0; i < num_entries; i++) {
			if (mmap->count >= MAX_MEM_REGIONS) {
				break;
			}

			uint64_t base = 0;
			uint64_t size = 0;
			const fdt32_t *ptr =
				reg + (i * (address_cell + size_cell));

			/* Extract address (handles varying cell counts) */
			for (unsigned long j = 0; j < address_cell; j++) {
				base = (base << 32) | fdt32_to_cpu(ptr[j]);
			}

			/* Extract size (handles varying cell counts) */
			for (unsigned long j = 0; j < size_cell; j++) {
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
