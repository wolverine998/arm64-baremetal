#ifndef __CPU_STATE__
#define __CPU_STATE__

#include <stdint.h>

#define MAX_CPUS 8

typedef enum { OFF = 0, ON, IDLE, BOOTING } power_state_t;

typedef struct {
  volatile power_state_t state;
  volatile uint64_t entry_point;
  volatile uint64_t context;
} cpu_state_t;

extern cpu_state_t cpus[MAX_CPUS];

#endif
