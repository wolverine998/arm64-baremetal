.section ".text.seeos"
.global seeos_vector
.balign(0x800)

/* * MACRO: SAVE_CONTEXT
 * Saves all GPRs and system registers to the stack.
 * Inlining this via a macro ensures x30 is the REAL user LR.
 */
.macro SAVE_CONTEXT
    sub sp, sp, #288
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]

    mrs x10, spsr_el1
    mrs x11, elr_el1
    mrs x12, esr_el1
    mrs x13, far_el1

    stp x10, x11, [sp, #248]
    stp x12, x13, [sp, #264]
.endm

/* * MACRO: RESTORE_CONTEXT
 * Restores state and prepares for eret.
 */
.macro RESTORE_CONTEXT
    ldp x10, x11, [sp, #248]
    msr spsr_el1, x10
    msr elr_el1, x11

    ldp x0, x1, [sp, #0]
    ldp x2, x3, [sp, #16]
    ldp x4, x5, [sp, #32]
    ldp x6, x7, [sp, #48]
    ldp x8, x9, [sp, #64]
    ldp x10, x11, [sp, #80]
    ldp x12, x13, [sp, #96]
    ldp x14, x15, [sp, #112]
    ldp x16, x17, [sp, #128]
    ldp x18, x19, [sp, #144]
    ldp x20, x21, [sp, #160]
    ldp x22, x23, [sp, #176]
    ldp x24, x25, [sp, #192]
    ldp x26, x27, [sp, #208]
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]

    add sp, sp, #288
    eret
.endm

seeos_vector:
// current el with sp_el0 (sync, fiq, irq, serror)
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .

// current el with sp_elx
.balign(0x80)
b seeos_sync
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .

// lower el with aarch64
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .

// lower el with aarch32
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .
.balign(0x80)
b .

seeos_sync:
SAVE_CONTEXT
mov x0, sp
mov x29, #0
bl seeos_sync_handler
RESTORE_CONTEXT
