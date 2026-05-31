/**
 * @file linker_defines.h
 * @brief #define values used in the linker script and referenced in C code.
 */
#ifndef LINKER_LINKER_DEFINES_H
#define LINKER_LINKER_DEFINES_H

/** @brief Size of the bitmap region in the linker script */
#define LINKER_BITMAP_SIZE 0x10000

/** @brief Size of each physical memory page
 * Size of each physical memory page from  table D4-20 in the Armv8-A
 * architecture reference manual.*/
#define LINKER_PAGE_SIZE 0x1000

/** @brief Number of pages in the identity map */
#define ID_MAP_NUM_PAGES 0x6

/** @brief Size of the identity map */
#define ID_MAP_SIZE (ID_MAP_NUM_PAGES * LINKER_PAGE_SIZE)

#endif /* LINKER_LINKER_DEFINES_H */
