/* Ensure the header is at the very beginning of the binary */
.section ".header.arm64", "ax"
.balign 8

    b       _start          // code0: Jump to actual entry point
    .long   0               // code1: Reserved
    .quad   0               // text_offset: 0 means "place me at the start of RAM"
    .quad   0               // image_size: 0 is acceptable for simple loaders
    .quad   0xA             // flags: LE, 4K Page, Phys placement anywhere
    .quad   0               // res2
    .quad   0               // res3
    .quad   0               // res4
    .ascii  "ARM\x64"       // magic: Magic number
    .long   0               // res5

.section ".text"
.global _start

_start: 
    // 1. Only allow Core 0 to proceed
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbnz    x0, halt

    // 2. Setup Stack
    // The stack pointer MUST be 16-byte aligned on AArch64
    ldr     x0, =stack_top
    mov     sp, x0

    // 3. Clear BSS (Recommended for C environments)
    ldr     x0, =__bss_start
    ldr     x1, =__bss_size
clear_bss:
    cbz     x1, jump_main
    str     xzr, [x0], #8
    subs    x1, x1, #1
    bne     clear_bss

jump_main:
    bl      main           // Branch to your C kernel entry point

halt:
    wfe
    b       halt
