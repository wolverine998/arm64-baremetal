ENTRY(_start)

SECTIONS
{
    /* Start EL3 Monitor at base of RAM */
    . = 0x40000000;

    .text : {
        *(.text.boot)
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    /* Uninitialized data */
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    /* EL3 STACK: Uses '.' to reserve 16KB right after BSS */
    . = ALIGN(16);
    . += 0x4000;
    _el3_stack_top = .;

}
