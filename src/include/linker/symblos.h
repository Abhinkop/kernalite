/**
 * @file symblos.h
 * @brief Symbol definitions from the linker script.
 * @author Abhin Parekadan Jose
 */
#ifndef LINKER_SYMBLOS_H
#define LINKER_SYMBLOS_H

#include <stdint.h>

/**
 * @brief Start of the page allocator bitmap.
 */
extern uint8_t page_allocator_bit_map_start[];

/**
 * @brief End of the page allocator bitmap.
 */
extern uint8_t page_allocator_bit_map_end[];

#endif // LINKER_SYMBLOS_H
