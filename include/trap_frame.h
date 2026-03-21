#ifndef __TRAP_FRAME__
#define __TRAP_FRAME__

#include <stdint.h>

typedef struct {
  uint64_t regs[31]; // 248
  uint64_t spsr;
  uint64_t elr;
  uint64_t esr;
  uint64_t far;

  uint64_t padding0; // 280

  uint64_t vregs[32][2] __attribute__((aligned(16)));
  uint32_t fpsr;
  uint32_t fpcr;

  uint64_t padding1;
} __attribute__((aligned(16))) trap_frame_t;
#endif
