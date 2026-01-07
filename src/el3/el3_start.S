.section ".text.boot"
.global _start
_start:
    msr daifset, #0xf

    // setup vector table
    adrp x0, el3_vector
    add x0, x0, :lo12:el3_vector
    msr vbar_el3, x0

    // Identify core: only core 0 continues
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cmp x0, #0
    b.eq primary_core

secondary_core:
    ldr x1, =_el3_stack_top
    mov x2, #16384
    mul x2, x0, x2
    sub x1, x1, x2
    bic x1, x1, #0xF
    mov sp, x1
    mov x29, #0

    bl init_sec_core

    // clear daif
    msr daifclr, #0xF
    isb

halt_secondary:
    wfi
    b halt_secondary

primary_core:
    ldr x0, =_el3_stack_top
    bic x0, x0, #0xF
    mov sp, x0
    bl main
1:  wfi
    b 1b
