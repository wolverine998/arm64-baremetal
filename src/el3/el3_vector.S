.section ".text.vector"
.global el3_vector
.balign(0x800)

/* * MACRO: SAVE_CONTEXT
 * Saves all GPRs and system registers to the stack.
 * Inlining this via a macro ensures x30 is the REAL user LR.
 */
.macro SAVE_CONTEXT
    sub sp, sp, #816
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

    mrs x10, spsr_el3
    mrs x11, elr_el3
    mrs x12, esr_el3
    mrs x13, far_el3

    stp x10, x11, [sp, #248]
    stp x12, x13, [sp, #264] 

    stp q0, q1, [sp, #288]
    stp q2, q3, [sp, #320]
    stp q4, q5, [sp, #352]
    stp q6, q7, [sp, #384]
    stp q8, q9, [sp, #416]
    stp q10, q11, [sp, #448]
    stp q12, q13, [sp, #480]
    stp q14, q15, [sp, #512]
    stp q16, q17, [sp, #544]
    stp q18, q19, [sp, #576]
    stp q20, q21, [sp, #608]
    stp q22, q23, [sp, #640]
    stp q24, q25, [sp, #672]
    stp q26, q27, [sp, #704]
    stp q28, q29, [sp, #736]
    stp q30, q31, [sp, #768]

    add x14, sp, #800
    mrs x10, fpsr
    mrs x11, fpcr
    stp x10, x11, [x14]
.endm

/* * MACRO: RESTORE_CONTEXT
 * Restores state and prepares for eret.
 */
.macro RESTORE_CONTEXT
    ldp x10, x11, [sp, #248]
    msr spsr_el3, x10
    msr elr_el3, x11

    add x14, sp, #800
    ldp x10, x11, [x14]
    msr fpsr, x10
    msr fpcr, x11

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

    ldp q0, q1, [sp, #288]
    ldp q2, q3, [sp, #320]
    ldp q4, q5, [sp, #352]
    ldp q6, q7, [sp, #384]
    ldp q8, q9, [sp, #416]
    ldp q10, q11, [sp, #448]
    ldp q12, q13, [sp, #480]
    ldp q14, q15, [sp, #512]
    ldp q16, q17, [sp, #544]
    ldp q18, q19, [sp, #576]
    ldp q20, q21, [sp, #608]
    ldp q22, q23, [sp, #640]
    ldp q24, q25, [sp, #672]
    ldp q26, q27, [sp, #704]
    ldp q28, q29, [sp, #736]
    ldp q30, q31, [sp, #768]

    add sp, sp, #816
    eret
.endm

el3_vector:
// current el with sp_el0
// sync
b .
.balign(0x80)
// fiq
b .
.balign(0x80)
// irq
b .
.balign(0x80)
// serror
b .
.balign(0x80)
// current el with sp_elx
b sync_handler
.balign(0x80)
b . // irq
.balign(0x80)
b fiq_handler
.balign(0x80)
b .
// lower el with aarch64
.balign(0x80)
b sync_lower
.balign(0x80)
b .
.balign(0x80)
b fiq_handler
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

sync_handler:
SAVE_CONTEXT
mov x0, sp
bl el3_sync
RESTORE_CONTEXT

fiq_handler:
SAVE_CONTEXT
mov x0, sp
bl el3_fiq
RESTORE_CONTEXT

sync_lower:
SAVE_CONTEXT
mov x0, sp
bl el3_sync_lower
RESTORE_CONTEXT
