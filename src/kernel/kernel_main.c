#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern char _kernel_start[], _kernel_stack[];
extern void app_entry();
extern char _app_start[], _app_end[], _app_stack[];
extern char user_l1[];

volatile power_state_t k_cpus[MAX_CPUS] = {0};

void c_entry() {
  k_cpus[0] = ON;
  kernel_puts("Kernel booted successfully\n");
  gic_enable_sre_el1();
  cpu_enable_group1_interrupts();

  cpu_set_priority_mask(255);
  mask_interrupts(0);

  smc_res_t res;
  smc_call(SEEOS_VERSION, &res, 0, 0, 0, 0, 0);
  smc_call(PSCI_CPU_ON, &res, 3, VA_TO_PA((uint64_t)_kernel_entry), 0, 0, 0);

  uint64_t *l1 = (uint64_t *)user_l1;
  uint64_t k_start = (uint64_t)_kernel_start;
  uint64_t k_end = (uint64_t)_kernel_stack;
  uint64_t start = (uint64_t)_app_start;
  uint64_t end = (uint64_t)_app_stack;

  while (k_cpus[3] != ON) {
    wait_for_event();
  }

  unmap_region(l1, k_start, k_end - k_start);
  map_region(l1, start, start, end - start,
             PROT_NORMAL_MEM | AP_EL0_RW_ELX_RW | PTE_PXN);

  uint64_t spsr = SPSR_M_EL0;
  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, (uint64_t)_app_stack);
  write_sysreg(elr_el1, (uint64_t)app_entry);

  asm volatile("isb; eret");
}

void sec_entry() {
  uint32_t core_id = (read_sysreg(mpidr_el1) & 0xFF);
  cpu_set_priority_mask(255);
  mask_interrupts(0);

  kernel_printf("Core %d booted successfully\n", core_id);

  k_cpus[core_id] = ON;
  asm volatile("dsb sy");
}
