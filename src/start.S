.section ".text.boot"
.global _start
_start:
    msr daifset, #0xf          // mask interrupts

    // Setup stack pointer
    ldr x0, =__end
    add x0, x0, #0x4000        // 16KB stack space
    bic x0, x0, #0xf           // FORCE 16-byte alignment (Mandatory for MMU)
    mov sp, x0

    // Identify core: only core 0 continues
    mrs x0, mpidr_el1
    and x0, x0, #3
    cmp x0, #0
    b.eq primary_core

halt_secondary:
    wfi
    b halt_secondary

primary_core:
    bl main
1:  wfe
    b 1b

