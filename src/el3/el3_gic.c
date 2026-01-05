#include "../../include/gic-v3.h"
#include <stdint.h>

void gic_init_global() {
  *(volatile uint32_t *)(GICD_BASE + GICD_CTLR) = 0;

  *(volatile uint32_t *)(GICD_BASE + GICD_CTLR) =
      GICD_CTLR_ENGRP0 | GICD_CTLR_ENGRP1NS | GICD_CTLR_ENGRP1S |
      GICD_CTLR_ARE_S | GICD_CTLR_ARE_NS;

  while (*(volatile uint32_t *)(GICD_BASE + GICD_CTLR) & GICD_CTLR_RWP)
    ;
}

void gic_init_core(int core_id) {
  uintptr_t rd_base = GET_GICR_BASE(core_id);

  // 1. System interfaces
  cpu_enable_group0_interrupts();
  cpu_enable_group1_interrupts();
  gic_enable_sre();

  // 2. Wake up (Standard R/W still needs a mask to be safe)
  write_gicr(rd_base, GICR_WAKER,
             read_gicr(rd_base, GICR_WAKER) & ~GICR_PROCESSOR_SLEEP);
  while (read_gicr(rd_base, GICR_WAKER) & GICR_CHILDREN_ASLEEP)
    ;

  uint32_t igroupr0 = read_gicr(rd_base, GICR_IGROUPR0);
  igroupr0 &= ~(1 << 1);
  igroupr0 &= ~(1 << 31);
  write_gicr(rd_base, GICR_IGROUPR0, igroupr0);

  // 3. Simple approach: Direct byte write for Priority
  // No masking, no shifting. ID 1 gets Priority 0. ID 31 gets Priority 0.
  write_gicr_8(rd_base, GICR_IPRIORITYR(1), 0);
  write_gicr_8(rd_base, GICR_IPRIORITYR(31), 0);

  // 4. Enable (Write-1-to-Set, so no masking needed)
  write_gicr(rd_base, GICR_ISENABLER0, (1U << 1));
  write_gicr(rd_base, GICR_ISENABLER0, (1U << 31));

  cpu_set_priority_mask(255);
}
