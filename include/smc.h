#ifndef __SMC__
#define __SMC__

#include "../include/psci.h"
#include <stdint.h>

typedef struct {
  uint64_t function_id;
  uint64_t res1;
  uint64_t res2;
  uint64_t res3;
  uint64_t res4;
  uint64_t res5;
} smc_res_t;

/* Upper 16 bits - World ID, Lower 16 bits - Function ID
 * World ID(31:16) - 0x1000(SEEOS) - 0x2000(FIRMWARE)
 * Function ID - Specific function tied to the World ID */
#define FIRMWARE_WORLD_ID 0x2000
#define SEEOS_WORLD_ID 0x1000

// firmware function id's
#define SMC_VERSION 0x20000010
#define SMC_YIELD 0x20000020
#define SMC_RESUME 0x20000030

// seeos functions
#define SEEOS_VERSION 0x10000010
#define SEEOS_DOORBELL 0x10000020

// state macros
#define SEEOS_INITIALIZED 0x01
#define SEEOS_INVALID_CALL 0x02
#define SEEOS_SUCCESS 0x04
#define SEEOS_PREEMPTED 0x06

extern volatile uint32_t core_ready[8];

static inline uint64_t smc_psci_cpu_on(uint32_t target_core, uint64_t entry) {
  register uint64_t x0 asm("x0") = PSCI_CPU_ON;
  register uint64_t x1 asm("x1") = (uint64_t)target_core;
  register uint64_t x2 asm("x2") = entry;

  asm volatile("smc #0" : "+r"(x0) : "r"(x1), "r"(x2) : "memory");
  return x0;
}

static inline int smc_psci_cpu_off(uint32_t target_core) {
  register uint64_t x0 asm("x0") = PSCI_CPU_OFF;
  register uint64_t x1 asm("x1") = target_core;

  asm volatile("smc #0" : "+r"(x0) : "r"(x1));
  return x0;
}

static inline void smc_call(uint32_t function_id, smc_res_t *res, uint64_t reg1,
                            uint64_t reg2, uint64_t reg3, uint64_t reg4,
                            uint64_t reg5) {
  register uint64_t x0 asm("x0") = function_id;
  register uint64_t x1 asm("x1") = reg1;
  register uint64_t x2 asm("x2") = reg2;
  register uint64_t x3 asm("x3") = reg3;
  register uint64_t x4 asm("x4") = reg4;
  register uint64_t x5 asm("x5") = reg5;

  asm volatile("smc #0"
               : "+r"(x0), "+r"(x1), "+r"(x2), "+r"(x3), "+r"(x4),
                 "+r"(x5)::"memory");

  res->function_id = x0;
  res->res1 = x1;
  res->res2 = x2;
  res->res3 = x3;
  res->res4 = x4;
  res->res5 = x5;
}

#endif
