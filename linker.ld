ENTRY(_start)

SECTIONS
{
    /* QEMU Virt RAM starts at 0x40000000 */
    . = 0x40000000;

    /* Code - Ensure the entry point is first */
    .text : {
        *(.text.boot)
        *(.text .text.*)
    }

    /* Read-only constants */
    .rodata : {
        *(.rodata .rodata.*)
    }

    /* Initialized data */
    .data : {
        *(.data .data.*)
    }

    /* Uninitialized data - Align to 8-byte for easy zeroing in assembly */
    . = ALIGN(8);
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* Define a symbol for the end of the binary for stack placement logic */
    . = ALIGN(16);
    __end = .;
}
