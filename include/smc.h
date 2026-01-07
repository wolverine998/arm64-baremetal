#ifndef __SMC__

#define __SMC__

#include <stdint.h>

#include "../include/psci.h"

extern volatile uint32_t core_ready[8];

static inline uint64_t smc_psci_cpu_on(uint32_t target_core, uint64_t entry) {
  register uint64_t x0 asm("x0") = PSCI_CPU_ON;
  register uint64_t x1 asm("x1") = (uint64_t)target_core;
  register uint64_t x2 asm("x2") = entry;

  asm volatile("smc #0" : "+r"(x0) : "r"(x1), "r"(x2) : "memory");
  return x0;
}

static inline void smc_psci_cpu_off(uint32_t target_core) {
  register uint64_t x0 asm("x0") = PSCI_CPU_OFF;
  register uint64_t x1 asm("x1") = target_core;

  asm volatile("smc #0" : : "r"(x0), "r"(x1));
}

#endif
