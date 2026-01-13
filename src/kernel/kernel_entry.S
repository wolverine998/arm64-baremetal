.section ".text.kernel"
.global _kernel_entry

_kernel_entry:
msr daifset, #0xF

// setup vbar_el1
adrp x0, kernel_vector
add x0, x0, :lo12:kernel_vector
msr vbar_el1, x0

mrs x0, cpacr_el1
orr x0, x0, #(3 << 20)
msr cpacr_el1, x0

mrs x0, mpidr_el1
and x0, x0, #0xFF
ldr x1, =_kernel_stack
mov x2, #32768
mul x2, x0, x2
sub x1, x1, x2
bic x1, x1, #0xF
mov sp, x1
mov x29, #0

cmp x0, #0
b.eq primary

secondary:
bl sec_entry
wfi

primary:
bl c_entry
wfi // should not get here
