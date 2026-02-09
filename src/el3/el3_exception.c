#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern void _seeos_entry();

extern void el3_smc_handler(trap_frame_t *frame, uint64_t function_id);

void el3_cpu_off(uint32_t target_core) {
  uart_puts("Turning off core\n");
  // first, disable redistributor
  uint64_t rd_base = GET_GICR_BASE(target_core);
  gic_disable_redistributor(rd_base);

  // disable mmu, caches, and stack check
  uint64_t sctlr = read_sysreg(sctlr_el1);
  sctlr &= ~(SCTLR_M | SCTLR_I | SCTLR_C | SCTLR_SA | SCTLR_SA0);
  write_sysreg(sctlr_el1, sctlr);
  write_sysreg(ttbr0_el1, 0);
  write_sysreg(ttbr1_el1, 0);
  instruction_barrier();
  tlb_flush_all_e1();

  uint32_t mask = SERROR | IRQ | DEBUG;
  write_sysreg(daif, mask);

  cpus[target_core].state = OFF;
  cpus[target_core].entry_point = 0;

  while (1) {
    asm volatile("wfi");
  }
}

void el3_seeos_jump(trap_frame_t *frame) {
  uint64_t scr = RW_AARCH64 | FIQ_ROUTE;
  write_sysreg(scr_el3, scr);
  write_sysreg(spsr_el3, SPSR_M_EL1H);
  write_sysreg(elr_el3, (uint64_t)_seeos_entry);
  asm volatile("isb; eret");
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
    uint32_t core_id = get_core_id();
    uint32_t function_id = (uint32_t)frame->regs[0];
    uint16_t world_id = (function_id >> 16);
    switch (world_id) {
    case FIRMWARE_WORLD_ID: {
      el3_smc_handler(frame, function_id);
      break;
    }
    case SEEOS_WORLD_ID: {
      cpus[core_id].ns_context.elr = frame->elr;
      save_context(&cpus[core_id].ns_context);
      for (int i = 19; i <= 30; i++) {
        cpus[core_id].ns_context.regs[i] = frame->regs[i];
      }

      if (cpus[core_id].s_context.initialized) {
        restore_context(&cpus[core_id].s_context);
        for (int i = 19; i <= 30; i++) {
          frame->regs[i] = cpus[core_id].s_context.regs[i];
        }
        uint64_t scr = RW_AARCH64 | FIQ_ROUTE;
        write_sysreg(scr_el3, scr);
        frame->spsr = SPSR_M_EL1H;
        frame->elr = cpus[core_id].s_context.elr;
      } else {
        disable_mmu_el1();
        write_sysreg(ttbr0_el1, 0);

        uint64_t scr = RW_AARCH64 | FIQ_ROUTE;
        write_sysreg(scr_el3, scr);
        frame->spsr = SPSR_M_EL1H;
        frame->elr = (uint64_t)_seeos_entry;
      }
      break;
    }
    default: {
      uart_puts("Wrong World ID\n");
      break;
    }
    }
  }
}

void el3_fiq(trap_frame_t *frame) {
  // mark cpu on
  uint32_t iar = gic_read_iar0();
  uint32_t interrupt_id = INTERRUPT_ID_MASK(iar);
  uint64_t core_id = get_core_id();

  if (interrupt_id == SGI_CORE_WAKE) {
    if (cpus[core_id].entry_point != 0) {
      gic_enable_redistributor(GET_GICR_BASE(core_id));
      uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
      write_sysreg(scr_el3, scr);

      frame->spsr = SPSR_M_EL1H;
      frame->elr = cpus[core_id].entry_point;
      cpus[core_id].state = ON;
    }
  } else if (interrupt_id == SGI_CORE_SLEEP) {
    // just jump to sleep function
    el3_cpu_off(core_id);
  }
  gic_write_eoir0(iar);
}
