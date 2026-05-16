ENTRY(primary_entry)

PHDRS
{
    text_perm PT_LOAD FLAGS(5); /* Read (4) + Execute (1) = 5 */
    rodata_perm PT_LOAD FLAGS(4); /* Read (4) = 6 */
    data_perm PT_LOAD FLAGS(6); /* Read (4) + Write (2) = 6 */
}

SECTIONS
{
    /* QEMU loads the kernel at 0x40000000 for aarch64 virt */
	. = 0x40000000;

	.header . : { KEEP(*(.header.arm64)) } :text_perm
	.text : { *(.text) } :text_perm
    .rodata : ALIGN(8) { *(.rodata) } :rodata_perm
    .data   : ALIGN(8) { *(.data) } :data_perm
    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(16);
        __bss_end = .;
    } :data_perm
    .data.page_tables : {
        page_allocator_bit_map_start = .;
        . += 0x10000; /* Reserve 64 KiB for the page bitmap (supports up to 2 GiB of RAM at 4 KiB pages) */
        page_allocator_bit_map_end = .;
    }
    __bss_size = (__bss_end - __bss_start) >> 3;

    /* Define a small stack area (16KB) */
    .stack (NOLOAD) : ALIGN(16) {
        . += 0x4000;
        stack_top = .;
    }

    _end = .;
}
