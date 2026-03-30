/* CP/M .COM linker script for Z80
 * Binary loaded at 0x0100 (Transient Program Area)
 * BDOS entry at 0x0005 (set up by CP/M or z88dk-ticks)
 */
ENTRY(_start)

SECTIONS {
    . = 0x0100;

    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }

    __bss_start = .;
    .bss : { *(.bss .bss.* COMMON) }
    __bss_size = . - __bss_start;
}
