#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include "../../include/psci.h"
#include "../../include/registers.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();

void el3_cpu_off(uint32_t target_core) {
  uart_puts("Turning off core\n");
  // first, disable redistributor
  gic_disable_redistributor(GET_GICR_BASE(target_core));
  cpu_set_priority_mask(0);

  // disable mmu, caches, and stack check
  uint64_t sctlr = read_sysreg(sctlr_el1);
  sctlr &= ~(SCTLR_M | SCTLR_A | SCTLR_I | SCTLR_C | SCTLR_SA | SCTLR_SA0);
  asm volatile("tlbi vmalle1; dsb sy");
  write_sysreg(sctlr_el1, sctlr);
  write_sysreg(ttbr0_el1, 0);

  cpus[target_core].state = OFF;
  cpus[target_core].entry_point = 0;

  while (1) {
    asm volatile("wfi");
  }
}

void el3_sync(trap_frame_t *frame) {
  uart_puts("Some exception occured\n");

  uart_puts("FAR: ");
  uart_hex(frame->far);
}

void el3_sync_lower(trap_frame_t *frame) {
  uint64_t esr = frame->esr;
  uint32_t ec = ESR_EC_MASK(esr);
  if (ec == EC_SMC) {
    switch (frame->regs[0]) {
    case PSCI_CPU_ON: {
      uint32_t target_core = (uint32_t)frame->regs[1];
      uint64_t target_addr = frame->regs[2];
      frame->regs[0] = psci_fn_cpu_on(target_core, target_addr);
      break;
    }
    case PSCI_CPU_OFF: {
      uint32_t target_core = (uint32_t)frame->regs[1];
      frame->regs[0] = psci_fn_cpu_off(target_core);
      break;
    }
    default: {
      frame->regs[0] = PSCI_ERR_NOT_SUPPORTED;
      break;
    }
    }
  }
}

void el3_fiq(trap_frame_t *frame) {
  // mark cpu on
  uint32_t iar = gic_read_iar0();
  uint32_t interrupt_id = INTERRUPT_ID_MASK(iar);
  uint64_t mpidr;
  asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
  uint64_t core_id = mpidr & 0xFF;

  if (interrupt_id == SGI_CORE_WAKE) {

    if (cpus[core_id].entry_point != 0) {
      // prepare drop
      uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
      uint64_t spsr = SPSR_M_EL1H;

      write_sysreg(scr_el3, scr);
      frame->spsr = spsr;
      frame->elr = cpus[core_id].entry_point;
    }

    gic_write_eoir0(iar);
  } else if (interrupt_id == SGI_CORE_SLEEP) {
    // just jump to sleep function
    gic_write_eoir0(iar);
    el3_cpu_off(core_id);
  }
}
