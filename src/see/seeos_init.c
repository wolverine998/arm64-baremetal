#include "../../include/mmu.h"
#include "../../include/smc.h"
#include "../../include/uart.h"

volatile uint32_t seeos_mmu_ready = 0;

void seeos_primary_core_init(uint64_t function_id, uint64_t arg1,
                             uint64_t arg2) {
  seeos_init_mmu_global();
  seeos_enable_mmu();

  seeos_mmu_ready = 1;

  seeos_puts_atomic("[SEEOS]: Primary core MMU enabled\n");

  smc_res_t res;
  res.function_id = function_id;
  res.res1 = arg1;
  res.res2 = arg2;

  while (1) {
    uint64_t r1 = 0;
    uint64_t r2 = 0;

    switch (res.function_id) {
    case SEEOS_VERSION:
      r1 = 0x10000;
      break;
    default:
      r1 = SEEOS_INVALID_CALL;
      break;
    }

    smc_call(SMC_YIELD, &res, r1, r2, 0, 0, 0);
  }
}

void seeos_secondary_core_init() {
  while (!seeos_mmu_ready)
    ;

  seeos_enable_mmu();
}
