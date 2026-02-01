#ifndef __KERNELMMU__
#define __KERNELMMU__

#include <stdint.h>

typedef struct {
  uint32_t total_ram;
  uint32_t used_ram;
  uint32_t free_ram;
} meminfo_t;

extern meminfo_t meminfo;

// translation tables
extern uint64_t kernel_l1[512];
extern uint64_t user_l1[512];

void seccore_setup_mmu();

#endif
