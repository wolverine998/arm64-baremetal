.section ".text.kernel"
.global _kernel_entry

_kernel_entry:
msr daifset, #0xF

ldr x0, =_kernel_stack
mov sp, x0

bl c_entry
wfe // should not get here
