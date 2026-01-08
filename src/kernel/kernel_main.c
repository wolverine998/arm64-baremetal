#include "../../include/gic-v3.h"
#include "../../include/kernel_mmu.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern void app_entry();
extern char _app_stack[];

void c_entry() {
  kernel_setup_mmu();
  kernel_puts("MMU enabled!!\n");
  gic_enable_sre_el1();
  cpu_set_priority_mask(255);
  cpu_enable_group1_interrupts();

  mask_interrupts(0);

  // boot other cores that might be offline
  smc_psci_cpu_on(2, (uint64_t)_kernel_entry);

  uint64_t spsr = SPSR_M_EL0;

  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, (uint64_t)_app_stack);
  write_sysreg(elr_el1, (uint64_t)app_entry);

  asm volatile("eret");

  while (1)
    ;
}

void sec_entry() {
  seccore_setup_mmu();
  cpu_set_priority_mask(255);

  mask_interrupts(0);

  uint32_t core_id = (read_sysreg(mpidr_el1) & 0xFF);
  kernel_printf("Core %d booted successfully\n", core_id);

  while (1)
    ;
}
