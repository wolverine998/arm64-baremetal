#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern void _seeos_entry();

void main() {
  setup_mmu();
  gic_init_global();
  gic_init_core(0);

  cpus[0].state = ON;

  uart_puts("Booted into EL3\n");

  mask_interrupts(0);

  uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
  write_sysreg(scr_el3, scr);

  uint64_t spsr = SPSR_M_EL1H;
  write_sysreg(spsr_el3, spsr);

  write_sysreg(elr_el3, (uint64_t)_kernel_entry);

  asm volatile("isb; eret");
}

void init_sec_core(int core_id) {
  setup_mmu_secondary();
  gic_init_core(core_id);
  cpus[core_id].state = ON;

  // unmask fiq
  uint32_t mask = SERROR | IRQ | DEBUG;
  write_sysreg(daif, mask);

  uint64_t scr = RW_AARCH64 | FIQ_ROUTE;
  write_sysreg(scr_el3, scr);

  uart_puts("Parking core\n");
  cpus[core_id].state = OFF;
}
