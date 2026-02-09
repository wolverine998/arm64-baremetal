#ifndef __KERNEL_GIC_V3__
#define __KERNEL_GIC_V3__

#include "gic-v3.h"
#include "mmu.h"
#include "registers.h"

static inline void write_gicd_virtual(uint64_t offset, uint32_t val) {
  *(volatile uint32_t *)(GICD_VIRT_BASE + offset) = val;
}

static inline void write8_gicd_virtual(uint64_t offset, uint8_t val) {
  *(volatile uint8_t *)(GICD_VIRT_BASE + offset) = val;
}
static inline void write64_gicd_virtual(uint64_t offset, uint64_t val) {
  *(volatile uint64_t *)(GICD_VIRT_BASE + offset) = val;
}

static inline uint32_t read_gicd_virtual(uint64_t offset) {
  return *(volatile uint32_t *)(GICD_VIRT_BASE + offset);
}

static inline uint8_t read8_gicd_virtual(uint64_t offset) {
  return *(volatile uint8_t *)(GICD_VIRT_BASE + offset);
}

static inline uint64_t read64_gicd_virtual(uint64_t offset) {
  return *(volatile uint64_t *)(GICD_VIRT_BASE + offset);
}

static inline void gic_conf_spi(uint32_t int_id, uint8_t priority, int group) {
  uint32_t index = INTERRUPT_INDEX(int_id);
  uint32_t bit = INTERRUPT_BIT_POSITION(int_id);

  uint32_t igroup = read_gicd_virtual(GICD_IGROUPR(index));

  if (group)
    igroup |= (1 << bit);
  else
    igroup &= ~(1 << bit);

  write_gicd_virtual(GICD_IGROUPR(index), igroup);
  write_gicd_virtual(GICD_ISENABLER(index), (1 << bit));
  write8_gicd_virtual(GICD_IPRIORITYR(int_id), priority);
}

static inline void gic_conf_sgi(uint32_t int_id, uint8_t priority, int group) {
  uint64_t rd_base = PA_TO_VA(GET_GICR_BASE(get_core_id()));

  uint32_t igroup = read_gicr(rd_base, GICR_IGROUPR0);

  if (group)
    igroup |= (1 << int_id);
  else
    igroup &= ~(1 << int_id);

  write_gicr(rd_base, GICR_IGROUPR0, igroup);
  write_gicr(rd_base, GICR_ISENABLER0, (1 << int_id));
  write_gicr_8(rd_base, GICR_IPRIORITYR(int_id), priority);
}

static inline void gic_route_spi(uint32_t int_id, uint64_t affinity) {
  if (int_id < 32)
    return;

  write64_gicd_virtual(GICD_IROUTER(int_id), affinity);
}

static inline void gic_set_spi_pending(uint32_t int_id) {
  if (int_id < 32)
    return;

  write_gicd_virtual(GICD_ISPENDR(INTERRUPT_INDEX(int_id)),
                     (1 << INTERRUPT_BIT_POSITION(int_id)));
}

static inline void gic_enable_spi(uint32_t int_id) {
  if (int_id < 32)
    return;

  uint32_t index = INTERRUPT_INDEX(int_id);
  uint32_t bit = (1 << INTERRUPT_BIT_POSITION(int_id));

  write_gicd_virtual(GICD_ISENABLER(index), bit);
}

static inline void gic_disable_spi(uint32_t int_id) {
  if (int_id < 32)
    return;

  uint32_t index = INTERRUPT_INDEX(int_id);
  uint32_t bit = (1 << INTERRUPT_BIT_POSITION(int_id));

  write_gicd_virtual(GICD_ICENABLER(index), bit);
}

static inline void gic_enable_sgi(uint32_t int_id) {
  if (int_id > 15)
    return;

  uint32_t core_id = get_core_id();

  // get the current redistributor
  uint64_t rd_base = PA_TO_VA(GET_GICR_BASE(core_id));

  write_gicr(rd_base, GICR_ISENABLER0, 1 << int_id);
}

static inline void gic_disable_sgi(uint32_t int_id) {
  if (int_id > 15)
    return;

  uint32_t core_id = get_core_id();

  // get the current redistributor
  uint64_t rd_base = PA_TO_VA(GET_GICR_BASE(core_id));

  write_gicr(rd_base, GICR_ICENABLER0, 1 << int_id);
}

#endif
