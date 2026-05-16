/**
 * @file symblos.h
 * @brief Symbol definitions from the linker script.
 * @author Abhin Parekadan Jose
 */
#ifndef LINKER_SYMBLOS_H
#define LINKER_SYMBLOS_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Start of the page allocator bitmap.
 */
extern uint8_t page_allocator_bit_map_start[];

/**
 * @brief End of the page allocator bitmap.
 */
extern uint8_t page_allocator_bit_map_end[];

/**
 * @brief Image start and end symbols provided by the linker script.
 *
 * These are linker-defined symbols and do not occupy storage; treat them
 * as pointers whose difference yields the image size.
 */
extern const char image_start;
extern const char image_end;

/*
 * Return the size of the linked image in bytes.
 * Uses the linker-provided `image_start` and `image_end` symbols.
 */
static inline size_t get_image_size(void)
{
	return (size_t)(&image_end - &image_start);
}

#endif // LINKER_SYMBLOS_H
