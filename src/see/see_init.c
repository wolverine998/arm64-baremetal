#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include "../../include/mmu.h"
#include "../../include/see_crypto.h"
#include "../../include/smc.h"
#include "../../include/uart.h"
#include <stdint.h>

static uint8_t master_key[16] = {0x02, 0x04, 0xA2, 0xA4, 0x0A, 0x5B,
                                 0x0D, 0x00, 0x53, 0x5A, 0x0F, 0xC3,
                                 0xE2, 0x12, 0x0D, 0xFF};

volatile uint32_t seeos_mmu_ready = 0;

uint32_t seeos_doorbell() {
  seeos_printf("[SEEOS] Device signaling interrupt ID %d\n", SPI_RESERVED_1);
  gic_el3_set_spi_pending(SPI_RESERVED_1);
  return SEEOS_SUCCESS;
}

void seeos_servicer(uint64_t fun_id, uint64_t arg1, uint64_t arg2,
                    uint64_t arg3) {
  smc_res_t res;
  res.function_id = fun_id;
  res.res1 = arg1;
  res.res2 = arg2;
  res.res3 = arg3;

  while (1) {
    uint64_t r1 = 0;
    uint64_t r2 = 0;
    uint64_t r3 = 0;

    switch (res.function_id) {
    case SEEOS_VERSION:
      r1 = 0x10000;
      break;
    case SEEOS_DOORBELL:
      r1 = seeos_doorbell();
      break;
    case SEEOS_WRAP_KEY: {
      wrapped_key_t key = see_wrap_key(res.res1);
      r1 = key.wrapped_key1;
      r2 = key.wrapped_key2;
      break;
    }
    case SEEOS_UNWRAP_KEY: {
      wrapped_key_t key = {res.res1, res.res2};
      r1 = see_unwrap_key(key);
      break;
    }
    default:
      r1 = SEEOS_INVALID_CALL;
      break;
    }

    smc_call(SMC_YIELD, &res, r1, r2, r3, 0, 0);
  }
}

void seeos_primary_core_init(uint64_t function_id, uint64_t arg1, uint64_t arg2,
                             uint64_t arg3) {
  seeos_init_mmu_global();
  seeos_enable_mmu();
  see_init_crypto(master_key);

  seeos_mmu_ready = 1;

  seeos_printf("[SEEOS]: Primary core initialized\n");
  seeos_servicer(function_id, arg1, arg2, arg3);
}

void seeos_secondary_core_init(uint64_t function_id, uint64_t arg1,
                               uint64_t arg2, uint64_t arg3) {
  while (!seeos_mmu_ready)
    ;

  seeos_enable_mmu();
  seeos_printf("[SEEOS]: Secondary core initialized\n");
  seeos_servicer(function_id, arg1, arg2, arg3);
}
