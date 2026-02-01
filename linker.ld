ENTRY(_start)

RAM_BASE = 0x40000000;
KERNEL_VIRT_BASE = 0xFFFFFFFF00000000;

SECTIONS
{
    /* --- PHYSICAL START --- */
    . = RAM_BASE;

    .pbl : {
        *el3_*(.text* .rodata* .data* .bss*)
    }

    .el3_stack (NOLOAD) : {
        . = ALIGN(16);
        . += 0x32000;
        _el3_stack_top = .;
    }

    /* Move SEE and APP out of the way */
    .see : ALIGN(4096) {
        _seeos_begin = .;
        *see_*(.text* .rodata* .data* .bss*)
        _seeos_end = .;
    }

    .see_stack (NOLOAD) : {
        . = ALIGN(16);
        . += 0x5F000;
        _seeos_stack_top = .;
    }

    .app : ALIGN(4096) {
        _app_start = .;
        *app_*(.text.app.entry .text* .rodata* .data* .bss*)
        . = ALIGN(16);
        . += 0x28000;
        _app_stack = .;
        _app_end = .;
    }

    /* --- KERNEL START --- */
    /* 1. Get the current physical address before switching to Virtual */
    _kernel_phys_start = ALIGN(4096);

    /* 2. Set the Virtual Location Counter */
    . = KERNEL_VIRT_BASE + _kernel_phys_start;

    /* 3. KERNEL CODE: Uses AT() to stay at the correct Physical spot */
    .kernel_text : AT(ADDR(.kernel_text) - KERNEL_VIRT_BASE) {
        _kernel_start = .;
        *kernel_*(.text.kernel .text .text.*)
        *kernel_*(.rodata .rodata.*)
    }

    /* 4. KERNEL DATA: The 64KB gap is handled here automatically */
    .kernel_data : AT(ADDR(.kernel_data) - KERNEL_VIRT_BASE) {
        . = ALIGN(65536);
        *kernel_*(.data .data.* .bss .bss.*)
        *(COMMON)
        _kernel_end = .;
    }

    /* 5. KERNEL STACK: Dedicated space, no more heap aliasing */
    .kernel_stack (NOLOAD) : AT(ADDR(.kernel_stack) - KERNEL_VIRT_BASE) {
        . = ALIGN(4096);
        . += 0x5F000;
        _kernel_stack = .;
    }

    .kernel_heap : AT(ADDR(.kernel_heap) - KERNEL_VIRT_BASE) {
        _heap_begin = ALIGN(4096);
    }
}
