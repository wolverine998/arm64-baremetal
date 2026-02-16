#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include <stddef.h>
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

  gic_el3_conf_sgi(rd_base, SGI_CORE_WAKE, 0x10, 0);
  gic_el3_conf_sgi(rd_base, SGI_CORE_SLEEP, 0x20, 0);
  gic_el3_conf_ppi(rd_base, 30, 0x0, 1);

  // unlock kernel SGI's
  for (int i = SGI_RES8_IGROUP1; i <= SGI_RES15_IGROUP1; i++) {
    gic_el3_conf_sgi(rd_base, i, 0xF0, 1);
  }

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
