#ifndef __REGISTERS__
#define __REGISTERS__

// AARCH64 SPSR_EL3 register bits
#define SPSR_EL3_EL0 (0ULL << 0)
#define SPSR_EL3_EL1T (4ULL << 0)
#define SPSR_EL3_EL1H (5ULL << 0)
#define SPSR_EL3_EL2T (8ULL << 0)
#define SPSR_EL3_EL2H (9ULL << 0)
#define SPSR_EL3_EL3T (12ULL << 0)
#define SPSR_EL3_EL3H (13ULL << 0)

#endif
