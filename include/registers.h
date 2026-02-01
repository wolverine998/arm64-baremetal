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

// ESR
#define ESR_EC_MASK(esr) ((esr >> 26) & 0x3F)
#define EC_SVC (0x15)
#define EC_SMC (0x17)
#define EC_DATA_ABORT_LOWER (0x24)
#define EC_DATA_ABORT (0x25)
#define ESR_ISS_MASK(esr) (esr & 0x1FFFFFF)

// ISS
#define ISS_WNR(iss) ((iss >> 6) & 0x1)
#define ISS_VALID(iss) ((iss >> 24) & 0x1)

// DFSC
#define ISS_DFSC_MASK(iss) (iss & 0x3F)
#define DFSC_TRANSLATION_L0 (0x4)
#define DFSC_TRANSLATION_L1 (0x5)
#define DFSC_TRANSLATION_L2 (0x6)
#define DFSC_TRANSLATION_L3 (0x7)

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
    asm volatile("tlbi vmalle1is; dsb ish");
}

static inline void data_barrier() { asm volatile("dsb sy"); }
static inline void instruction_barrier() { asm volatile("isb"); }
static inline void wait_for_event() { asm volatile("wfe"); }
static inline void wait_for_interrupt() { asm volatile("wfi"); }

static inline uint32_t get_core_id() {
  uint32_t core_id = (read_sysreg(mpidr_el1) & 0xFF);
  return core_id;
}

#endif
