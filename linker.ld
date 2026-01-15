ENTRY(_start)

RAM_BASE = 0x40080000;
KERNEL_VIRT_BASE = 0xFFFFFFFF00000000;

SECTIONS
{
    . = RAM_BASE;

    /* 1. EL3 MONITOR CODE */
    /* Use the standard '.text' name so the linker doesn't create an orphan */
    .pbl : {
        *el3_*(.text .text.*)
        *el3_*(.rodata .rodata.*)
        *el3_*(.bss .bss.*)
        *el3_*(.data .data.*)
    }

    /* EL3 Stack */
    . = ALIGN(16);
    . += 0x20000;
    _el3_stack_top = .;

    /* Secure execution environment code */
    . = ALIGN(4096);
    _seeos_begin = .;
    .see : {
      *see_*(.text .text.*)
      *see_*(.rodata .rodata.*)
      *see_*(.bss .bss.*)
      *see_*(.data .data.*)
    }
    _seeos_end = .;

    . = ALIGN(16);
    . += 0x20000;
    _seeos_stack_top = .;

    . = ALIGN(4096);
    _app_start = .;

    .app_text : {
        *app_*(.text.app.entry)
        *app_*(.text .text.*)
        *app_*(.rodata .rodata.*)
        *app_*(.bss .bss.*)
        *app_*(.data .data.*)
  }

  . = ALIGN(16);
  . += 0x20000;
  _app_stack = .;

  . = ALIGN(4096);
  _app_end = .;

  . = ALIGN(4096);
  _kernel_phys_start = .;

  /* 2. KERNEL BOUNDARY */
  . += KERNEL_VIRT_BASE;
  _kernel_start = .;

  /* 3. KERNEL CODE */
  /* We name this .kernel_text to keep it distinct from the Monitor .text */
  .kernel_text : AT(_kernel_phys_start) {
      *kernel_*(.text.kernel .text .text.*)
      *kernel_*(.rodata .rodata.*)
      *kernel_*(.bss .bss.*)
      *kernel_*(.data .data.*)
  }

   . = ALIGN(4096);
   _kernel_end = .;

    /* Kernel Stack */
    . = ALIGN(16);
    . += 0x40000;
    _kernel_stack = .;
}
