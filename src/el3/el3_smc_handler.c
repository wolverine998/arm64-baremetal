#include "../../include/cpu_state.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"

#define VERSION 101

void el3_smc_handler(trap_frame_t *frame, uint64_t fun_id) {
  switch (fun_id) {
  case PSCI_CPU_ON: {
    uint32_t target_core = (uint32_t)frame->regs[1];
    uint64_t target_addr = frame->regs[2];
    frame->regs[1] = psci_fn_cpu_on(target_core, target_addr);
    break;
  }
  case PSCI_CPU_OFF: {
    uint32_t target_core = (uint32_t)frame->regs[1];
    frame->regs[1] = psci_fn_cpu_off(target_core);
    break;
  }
  case SMC_YIELD: {
    uint32_t core_id = get_core_id();
    save_context(&cpus[core_id].s_context);
    for (int i = 19; i <= 30; i++) {
      cpus[core_id].s_context.regs[i] = frame->regs[i];
    }
    cpus[core_id].s_context.elr = frame->elr;
    restore_context(&cpus[core_id].ns_context);
    for (int i = 19; i <= 30; i++) {
      frame->regs[i] = cpus[core_id].ns_context.regs[i];
    }
    uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
    write_sysreg(scr_el3, scr);
    frame->spsr = SPSR_M_EL1H;
    frame->elr = cpus[core_id].ns_context.elr;
    break;
  }
  case SMC_VERSION: {
    frame->regs[1] = VERSION;
    break;
  }
  default: {
    frame->regs[1] = PSCI_ERR_NOT_SUPPORTED;
    break;
  }
  }
}
