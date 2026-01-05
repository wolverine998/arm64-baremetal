#ifndef __TRAP_FRAME__
#define __TRAP_FRAME__

#include <stdint.h>

typedef struct {
  uint64_t regs[31];
  uint64_t spsr;
  uint64_t elr;
  uint64_t esr;
  uint64_t far;
} trap_frame_t;
#endif
