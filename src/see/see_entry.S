.section ".text.see"
.global _seeos_entry

_seeos_entry:
msr daifset, #0xF

adrp x9, seeos_vector
add x9, x9, :lo12:seeos_vector
msr vbar_el1, x9

mrs x9, mpidr_el1
and x9, x9, #0xFF
ldr x10, =_seeos_stack_top
mov x11, #32768
mul x11, x9, x11
sub x10, x10, x11
bic x10, x10, #0xF
mov sp, x10
mov x29, #0

cmp x9, #0
b.eq seeos_prim_path

bl seeos_secondary_core_init
b .

seeos_prim_path:
bl seeos_primary_core_init
b .
