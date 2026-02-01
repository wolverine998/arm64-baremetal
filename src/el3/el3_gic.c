#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include <stdint.h>

void gic_init_global() {
  write_gicd(GICD_CTLR, 0);

  uint32_t ctlr = GICD_CTLR_ENGRP0 | GICD_CTLR_ENGRP1NS | GICD_CTLR_ENGRP1S |
                  GICD_CTLR_ARE_NS | GICD_CTLR_ARE_S;
  write_gicd(GICD_CTLR, ctlr);

  while (read_gicd(GICD_CTLR) & GICD_CTLR_RWP)
    ;

  // unlock interupts for kernel
  for (int i = SPI_RESERVED_1; i <= SPI_RESERVED_3; i++) {
    gic_el3_conf_spi(i, 0, 1);
  }
}

void gic_init_core(int core_id) {
  uintptr_t rd_base = GET_GICR_BASE(core_id);
  gic_enable_redistributor(rd_base);

  gic_enable_sre();

  uint32_t igroupr0 = read_gicr(rd_base, GICR_IGROUPR0);
  igroupr0 &= ~(1 << SGI_CORE_WAKE);
  igroupr0 &= ~(1 << SGI_CORE_SLEEP);
  write_gicr(rd_base, GICR_IGROUPR0, igroupr0);

  // 3. Simple approach: Direct byte write for Priority
  // No masking, no shifting. ID 1 gets Priority 0. ID 31 gets Priority 0.
  write_gicr_8(rd_base, GICR_IPRIORITYR(SGI_CORE_WAKE), 16);
  write_gicr_8(rd_base, GICR_IPRIORITYR(SGI_CORE_SLEEP), 32);

  // 4. Enable (Write-1-to-Set, so no masking needed)
  write_gicr(rd_base, GICR_ISENABLER0, (1U << SGI_CORE_WAKE));
  write_gicr(rd_base, GICR_ISENABLER0, (1U << SGI_CORE_SLEEP));

  cpu_enable_group0_interrupts();
  cpu_set_priority_mask(255);
}

void gic_enable_redistributor(uintptr_t rd_base) {
  uint32_t val = read_gicr(rd_base, GICR_WAKER);

  val &= ~(GICR_PROCESSOR_SLEEP);
  write_gicr(rd_base, GICR_WAKER, val);

  while ((read_gicr(rd_base, GICR_WAKER) & GICR_CHILDREN_ASLEEP))
    ;
}

void gic_disable_redistributor(uintptr_t rd_base) {
  uint32_t val = read_gicr(rd_base, GICR_WAKER);

  val |= GICR_PROCESSOR_SLEEP;
  write_gicr(rd_base, GICR_WAKER, val);

  while (!(read_gicr(rd_base, GICR_WAKER) & GICR_CHILDREN_ASLEEP))
    ;
}
