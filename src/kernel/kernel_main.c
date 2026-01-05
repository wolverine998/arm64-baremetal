#include "../../include/gic-v3.h"
#include "../../include/kernel_mmu.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void app_entry();
extern char _app_stack[];

void c_entry() {
  kernel_setup_mmu();
  gic_enable_sre_el1();
  cpu_set_priority_mask(0xFF);
  cpu_enable_group1_interrupts();

  kernel_puts("MMU enabled successfully\n");

  uint64_t spsr = SPSR_M_EL0;

  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, (uint64_t)_app_stack);
  write_sysreg(elr_el1, (uint64_t)app_entry);

  asm volatile("eret");

  while (1) {
    asm volatile("wfi");
  }
}

void sec_entry() {
  seccore_setup_mmu();

  kernel_puts("Core successfully powered on\n");

  while (1)
    ;
}
