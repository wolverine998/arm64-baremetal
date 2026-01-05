#ifndef __REGISTERS__
#define __REGISTERS__

// --- System register helpers ---
#define read_sysreg(reg)                                                       \
  ({                                                                           \
    uint64_t val;                                                              \
    asm volatile("mrs %0, " #reg : "=r"(val));                                 \
    val;                                                                       \
  })

#define write_sysreg(reg, val)                                                 \
  asm volatile("msr " #reg ", %0" ::"r"((uint64_t)(val)))

// SCTLR_EL1 bits
#include <stdint.h>
#define SCTLR_M (1 << 0)
#define SCTLR_A (1 << 1)
#define SCTLR_C (1 << 2)
#define SCTLR_SA (1 << 3)
#define SCTLR_SA0 (1 << 4)
#define SCTLR_I (1 << 12)

// SCR bits
#define RW_AARCH64 (1 << 10)
#define SMC_DISABLE (1 << 7)
#define EA_ROUTE (1 << 3)
#define FIQ_ROUTE (1 << 2)
#define IRQ_ROUTE (1 << 1)
#define NS (1 << 0)
#define S (0 << 0)

// HCR bits
#define HCR_RW_AARCH64 (1ULL << 31)

// SPSR_EL
#define SPSR_M_EL0 (0x0ULL << 0)
#define SPSR_M_EL1T (0x4ULL << 0)
#define SPSR_M_EL1H (0x5ULL << 0)
#define SPSR_M_EL3T (0xCULL << 0)
#define SPSR_M_EL3H (0xDULL << 0)
#define FIQ (1 << 6)
#define IRQ (1 << 7)
#define SERROR (1 << 8)
#define DEBUG (1 << 9)

// Helper methods
static inline void mask_interrupts(int8_t mask) {
  if (mask == 1)
    asm volatile("msr daifset, #0xf");
  else
    asm volatile("msr daifclr, #0xf");
}

static inline void flush_tlbi(int8_t el3) {
  if (el3 == 1)
    asm volatile("tlbi alle3is; dsb ish");
  else
    asm volatile("tbli alle1is; dsb ish");
}

static inline void data_barrier() { asm volatile("dsb sy"); }

static inline void instruction_barrier() { asm volatile("isb"); }

#endif
