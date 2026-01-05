#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();

void main() {
  setup_mmu();
  gic_init_global();
  gic_init_core(0);

  uart_puts("Booted into EL3\n");

  send_sgi0_to_core((1 << 1), 1);

  uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
  write_sysreg(scr_el3, scr);

  uint64_t spsr = SPSR_M_EL1H;
  write_sysreg(spsr_el3, spsr);

  write_sysreg(elr_el3, (uint64_t)_kernel_entry);
  asm volatile("isb; eret;");
}

void init_sec_core(int core_id) {
  uint64_t scr = RW_AARCH64 | FIQ_ROUTE;
  write_sysreg(scr_el3, scr);
  asm volatile("isb");

  // turn up the mmu and the gic
  setup_mmu_secondary();
  gic_init_core(core_id);
}
