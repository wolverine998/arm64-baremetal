#ifndef __REGISTERS__
#define __REGISTERS__

// SCTLR_EL1 bits
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

// SPSR_EL
#define SPSR_M_EL0 (0x0ULL << 0)
#define SPSR_M_EL1T (0x4ULL << 0)
#define SPSR_M_EL1H (0x5ULL << 0)
#define SPSR_M_EL3T (0xCULL << 0)
#define SPSR_M_EL3H (0xDULL << 0)

#endif
