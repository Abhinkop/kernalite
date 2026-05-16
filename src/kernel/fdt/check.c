/**
 * @file fdt.c
 * @brief Implementation of Device Tree Blob (FDT) parsing and validation functions.
 */

#include "fdt/check.h"

#include "utils/kprintf.h"

#include <libfdt.h>

bool check_fdt(uintptr_t phys_addr)
{
	if (phys_addr == 0) {
		kprintf("FDT Error: null DTB address\n");
		return false;
	}

	// NOLINTNEXTLINE(*-int-to-ptr)
	const void *ptr = (const void *)phys_addr;

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
