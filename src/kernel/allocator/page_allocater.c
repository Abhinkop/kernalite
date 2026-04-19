#include "allocator/page_allocater.h"

#include <stdint.h>
#include <stddef.h>

#include "utils/printf.h"

static uint8_t *mem_base; /**< Start of the managed memory region */
static uint8_t *bitmap; /**< Pointer to the bitmap (lives at mem_base) */
static size_t total_pages; /**< Total number of 4KB pages in the pool */

void page_init(void *mem_start, size_t mem_size)
{
	printf("PAGE: Initializing allocator at %p (size: %u bytes)\n",
	       mem_start, mem_size);

	// 1. Align the starting address to a 4KB boundary
	uintptr_t start_addr = (uintptr_t)mem_start;
	uintptr_t aligned_start = (start_addr + (PAGE_SIZE - 1)) &
				  ~(PAGE_SIZE - 1);

	if (aligned_start != start_addr) {
		printf("PAGE: Aligned start to 0x%lx (offset: %lu bytes)\n",
		       aligned_start, aligned_start - start_addr);
	}

	// Adjust size based on the alignment shift
	mem_size -= (aligned_start - start_addr);

	// NOLINTNEXTLINE(*-int-to-ptr)
	mem_base = (uint8_t *)aligned_start;
	total_pages = mem_size / PAGE_SIZE;

	// 2. Calculate bitmap requirements
	size_t bitmap_size_bytes = (total_pages + 7) / 8;
	size_t pages_for_bitmap =
		(bitmap_size_bytes + PAGE_SIZE - 1) / PAGE_SIZE;

	printf("PAGE: Total pages: %u, Bitmap requires: %u bytes (%u pages)\n",
	       total_pages, bitmap_size_bytes, pages_for_bitmap);

	// The bitmap starts at the beginning of our aligned memory
	bitmap = mem_base;

	// 3. Zero out the bitmap (all pages initially free)
	for (size_t i = 0; i < bitmap_size_bytes; i++) {
		bitmap[i] = 0;
	}

	// 4. Reserve the pages used by the bitmap itself
	for (size_t i = 0; i < pages_for_bitmap; i++) {
		bitmap[i / 8] |= (1 << (i % 8));
	}

	printf("PAGE: Usable memory starts at 0x%lx\n",
	       (uintptr_t)mem_base + (pages_for_bitmap * PAGE_SIZE));
}

void *page_alloc(size_t num_pages)
{
	if (num_pages == 0) {
		return NULL;
	}

	if (num_pages > total_pages) {
		printf("PAGE: Allocation failed. Request (%u) exceeds total capacity (%u)\n",
		       num_pages, total_pages);
		return NULL;
	}

	size_t continuous_found = 0;
	size_t start_index = 0;

	for (size_t i = 0; i < total_pages; i++) {
		uint8_t is_used = bitmap[i / 8] & (1 << (i % 8));

		if (!is_used) {
			if (continuous_found == 0) {
				start_index = i;
			}
			continuous_found++;

			if (continuous_found == num_pages) {
				// Mark pages as used
				for (size_t j = start_index;
				     j < start_index + num_pages; j++) {
					bitmap[j / 8] |= (1 << (j % 8));
				}

				void *ptr = (void *)(mem_base +
						     (start_index * PAGE_SIZE));
				printf("PAGE: Allocated %u pages at %p\n",
				       num_pages, ptr);
				return ptr;
			}
		} else {
			continuous_found = 0;
		}
	}

	printf("PAGE: Allocation failed. No contiguous block of %u pages found.\n",
	       num_pages);
	return NULL;
}

void page_free(void *ptr, size_t num_pages)
{
	if (!ptr)
		return;

	if (ptr < (void *)mem_base ||
	    ptr >= (void *)(mem_base + (total_pages * PAGE_SIZE))) {
		printf("PAGE ERROR: Attempted to free out-of-bounds pointer %p\n",
		       ptr);
		return;
	}

	// Calculate start index
	size_t start_index = ((uint8_t *)ptr - mem_base) / PAGE_SIZE;

	// Boundary check for the range
	if (start_index + num_pages > total_pages) {
		printf("PAGE WARNING: Free range out of bounds. Truncating %u to %u pages\n",
		       num_pages, total_pages - start_index);
		num_pages = total_pages - start_index;
	}

	printf("PAGE: Freeing %u pages at %p\n", num_pages, ptr);

	// Mark pages as free
	for (size_t i = start_index; i < start_index + num_pages; i++) {
		bitmap[i / 8] &= ~(1 << (i % 8));
	}
}

void page_dump_status(void)
{
	if (total_pages == 0) {
		printf("Page Allocator: Not initialized\n");
		return;
	}

	printf("--- Physical Page Dump ---\n");
	printf("Managed Range: 0x%lx - 0x%lx (%u pages)\n", (uintptr_t)mem_base,
	       (uintptr_t)mem_base + (total_pages * PAGE_SIZE), total_pages);

	size_t start_idx = 0;
	// Get status of the first page to initialize the tracker
	uint8_t current_status = (bitmap[0] & (1 << 0)) ? 1 : 0;

	for (size_t i = 1; i <= total_pages; i++) {
		uint8_t status = 0;

		// If we haven't reached the end, check the current bit
		if (i < total_pages) {
			status = (bitmap[i / 8] & (1 << (i % 8))) ? 1 : 0;
		}

		// If status changed OR we reached the end of the bitmap
		if (status != current_status || i == total_pages) {
			uintptr_t block_start =
				(uintptr_t)mem_base + (start_idx * PAGE_SIZE);
			uintptr_t block_end =
				(uintptr_t)mem_base + (i * PAGE_SIZE);
			size_t block_size_pages = i - start_idx;

			printf("  [0x%lx - 0x%lx] %s (%u pages)\n", block_start,
			       block_end - 1, // Inclusive range display
			       current_status ? "USED" : "FREE",
			       block_size_pages);

			// Update tracker for the next block
			start_idx = i;
			current_status = status;
		}
	}
	printf("--- End of Dump ---\n");
}
