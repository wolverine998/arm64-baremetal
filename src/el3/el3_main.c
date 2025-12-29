#include "../include/mmu.h"
#include "../include/registers.h"
#include "../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();

void main() {
  setup_mmu();

  uart_puts("Booted into EL3\n");
  uart_puts("Welcome to EL3...\n");

  uint64_t scr = RW_AARCH64 | NS;
  write_sysreg(scr_el3, scr);

  uint64_t spsr = SPSR_M_EL1H;
  write_sysreg(spsr_el3, spsr);

  write_sysreg(elr_el3, (uint64_t)_kernel_entry);
  asm volatile("isb; eret;");
}
