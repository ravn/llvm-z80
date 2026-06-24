/* cpm.ld — linker script for CP/M .COM files (ld.lld + llvm-objcopy)
 *
 * Workflow:
 *   1. ld.lld --gc-sections -T cpm.ld objs... -o prog.elf
 *   2. llvm-objcopy -O binary --only-section=.text prog.elf prog.com
 *
 * All code + rodata + initialized data land in .text so step 2 extracts
 * the complete binary with one --only-section flag.
 * BSS is NOLOAD (not in the file); the crt0 zeroes it at startup.
 */

MEMORY {
    tpa : ORIGIN = 0x0100, LENGTH = 0xDBFF
}

SECTIONS {
    . = 0x0100;

    .text : {
        *(.text.crt0)
        *(.text*)
        *(.rodata*)
        *(.data*)
    } > tpa

    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } > tpa

    __bss_length = __bss_end - __bss_start;

    /* Heap grows upward from end of BSS.  The ceiling leaves headroom below
       the stack (SP starts at 0xFFFE) and the 0xDC00 BDOS stub used by the
       ticks harness — 0xC000 gives a generous, safe TPA heap.
       Names are single-underscore so the C mangling (which prepends '_')
       resolves: C `heap_start` -> asm `_heap_start`. */
    _heap_start = __bss_end;
    _heap_end   = 0xC000;

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
