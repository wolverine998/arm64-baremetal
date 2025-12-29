#include "../include/mmu.h"
#include "../include/registers.h"
#include "../include/uart.h"
#include <stdint.h>

extern void app_entry();
extern char _app_stack[];

void c_entry() {
  kernel_setup_mmu();

  uart_puts("MMU enabled successfully\n");

  uint64_t spsr = SPSR_M_EL0;
  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, (uint64_t)_app_stack);
  write_sysreg(elr_el1, (uint64_t)app_entry);
  asm volatile("eret");

  while (1) {
    asm volatile("wfe");
  }
}
