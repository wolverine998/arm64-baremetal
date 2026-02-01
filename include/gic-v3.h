#ifndef __GICV3__
#define __GICV3__

#include "mmu.h"
#include <stdint.h>

// --- Raw Access Helpers (The "Fuck Structs" Way) ---
#define GET_GICR_BASE(core_id) (GICR_BASE + (core_id * 0x20000))

// --- Physical Base Addresses (QEMU Virt) ---
#define GICD_BASE 0x08000000
#define GITS_BASE 0x08080000
#define GICR_BASE 0x080A0000

#define GICD_VIRT_BASE (GICD_BASE + KERNEL_VIRT_BASE)
#define GITS_VIRT_BASE (GITS_BASE + KERNEL_VIRT_BASE)
#define GICR_VIRT_BASE (GICR_BASE + KERNEL_VIRT_BASE)

// --- GICD (Distributor) Offsets ---
#define GICD_CTLR 0x0000
#define GICD_IGROUPR(n) (0x0080 + (n) * 4)
#define GICD_ISENABLER(n) (0x0100 + (n) * 4)
#define GICD_ICENABLER(n) (0x0180 + (n) * 4)
#define GICD_ISPENDR(n) (0x0200 + (n) * 4)
#define GICD_IPRIORITYR(n) (0x0400 + (n)) // Byte access
#define GICD_IROUTER(n) (0x6000 + (n) * 8)
// --- GICD_CTLR Bits
#define GICD_CTLR_RWP (1U << 31)
#define GICD_CTLR_E1NWF (1U << 7)
#define GICD_CTLR_DS (1U << 6)
#define GICD_CTLR_ARE_NS (1U << 5) // Corrected: Bit 5
#define GICD_CTLR_ARE_S (1U << 4)  // Corrected: Bit 4
#define GICD_CTLR_ENGRP1S (1U << 2)
#define GICD_CTLR_ENGRP1NS (1U << 1)
#define GICD_CTLR_ENGRP0 (1U << 0)

// --- GICR (Redistributor) Offsets ---
#define GICR_CTLR 0x0000
#define GICR_PROPBASER 0x0070
#define GICR_PENDBASER 0x0078
#define GICR_SGI_OFFSET 0x10000 // The 64KB "Page 1" jump
#define GICR_WAKER 0x0014
#define GICR_IGROUPR0 (GICR_SGI_OFFSET + 0x0080)
#define GICR_ISENABLER0 (GICR_SGI_OFFSET + 0x0100)
#define GICR_IPRIORITYR(n) (GICR_SGI_OFFSET + 0x0400 + (n))

// GICR_CTLR bits
#define GICR_CTLR_ENLPIS (1 << 0)
#define GICR_CTLR_RWP (1 << 3)
#define GICR_CTLR_UWP (1 << 31)

// Redistributor bits
#define GICR_PROCESSOR_SLEEP (1 << 1)
#define GICR_CHILDREN_ASLEEP (1 << 2)

// ITS registers
#define GITS_CTLR 0x0000
#define GITS_CBASER 0x0080
#define GITS_CWRITER 0x0088
#define GITS_CREADR 0x0090
#define GITS_BASER0 0x0100
#define GITS_BASER1 0x0108

// GITS_CTLR bits
#define GITS_CTLR_ENABLED (1 << 0)
#define GITS_CTLR_QUIESCENT (1 << 31)

#define INTERRUPT_ID_MASK(imm) (imm & 0xFFFFFF)
#define INTERRUPT_INDEX(intid) (intid / 32)
#define INTERRUPT_BIT_POSITION(intid) (intid % 32)

static inline void write_gicd(uint64_t offset, uint32_t val) {
  *(volatile uint32_t *)(GICD_BASE + offset) = val;
}

static inline void write_gicd_8(uint64_t offset, uint8_t val) {
  *(volatile uint8_t *)(GICD_BASE + offset) = val;
}

static inline void write_gicd_64(uint64_t offset, uint64_t val) {
  *(volatile uint64_t *)(GICD_BASE + offset) = val;
}

static inline uint32_t read_gicd(uint64_t offset) {
  return *(volatile uint32_t *)(GICD_BASE + offset);
}

static inline void write_gicr(uintptr_t base, uint32_t offset, uint32_t val) {
  *(volatile uint32_t *)(base + offset) = val;
}

static inline void write_gicr_8(uintptr_t base, uint32_t offset, uint32_t val) {
  *(volatile uint8_t *)(base + offset) = val;
}

static inline uint32_t read_gicr(uintptr_t base, uint32_t offset) {
  return *(volatile uint32_t *)(base + offset);
}

static inline void write_gicr_64(uintptr_t base, uint64_t offset,
                                 uint64_t val) {
  *(volatile uint64_t *)(base + offset) = val;
}

static inline uint64_t read_gicr_64(uintptr_t base, uint64_t offset) {
  return *(volatile uint64_t *)(base + offset);
}

static inline void write_gits_64(uint64_t offset, uint64_t val) {
  *(volatile uint64_t *)(GITS_VIRT_BASE + offset) = val;
}

static inline uint64_t read_gits_64(uint64_t offset) {
  return *(volatile uint64_t *)(GITS_VIRT_BASE + offset);
}

static inline void write_gits_32(uint32_t offset, uint32_t val) {
  *(volatile uint32_t *)(GITS_VIRT_BASE + offset) = val;
}

static inline uint32_t read_gits_32(uint32_t offset) {
  return *(volatile uint32_t *)(GITS_VIRT_BASE + offset);
}

static inline void gic_el3_conf_spi(uint32_t int_id, uint8_t priority,
                                    int group) {
  uint32_t index = INTERRUPT_INDEX(int_id);
  uint32_t bit = INTERRUPT_BIT_POSITION(int_id);

  uint32_t igroup = read_gicd(GICD_IGROUPR(index));

  if (group)
    igroup |= (1 << bit);
  else
    igroup &= ~(1 << bit);

  write_gicd(GICD_IGROUPR(index), igroup);
  write_gicd(GICD_ISENABLER(index), (1 << bit));
  write_gicd_8(GICD_IPRIORITYR(int_id), priority);
}

static inline void gic_el3_route_spi(uint32_t int_id, uint64_t affinity) {
  if (int_id < 32)
    return;

  write_gicd_64(GICD_IROUTER(int_id), affinity);
}

static inline void gic_el3_set_spi_pending(uint32_t int_id) {
  if (int_id < 32)
    return;

  write_gicd(GICD_ISPENDR(INTERRUPT_INDEX(int_id)),
             (1 << INTERRUPT_BIT_POSITION(int_id)));
}

// --- CPU Interface (System Registers) ---
static inline void gic_enable_sre() {
  uint64_t sre;
  asm volatile("mrs %0, ICC_SRE_EL3" : "=r"(sre));
  asm volatile("msr ICC_SRE_EL3, %0" : : "r"(sre | 0xF));
  asm volatile("isb");
}

static inline void gic_enable_sre_el1() {
  uint64_t sre;
  asm volatile("mrs %0, ICC_SRE_EL1" : "=r"(sre));
  asm volatile("msr ICC_SRE_EL1, %0" : : "r"(sre | 0x1));
  asm volatile("isb");
}

static inline uint32_t gic_read_iar0() {
  uint64_t iar;
  asm volatile("mrs %0, ICC_IAR0_EL1" : "=r"(iar));
  return (uint32_t)iar;
}

static inline void gic_write_eoir0(uint32_t iar) {
  asm volatile("msr ICC_EOIR0_EL1, %0" : : "r"((uint64_t)iar));
}

static inline uint32_t gic_read_iar1() {
  uint64_t iar;
  asm volatile("mrs %0, ICC_IAR1_EL1" : "=r"(iar));
  return (uint32_t)iar;
}

static inline void gic_write_eoir1(uint32_t iar) {
  asm volatile("msr ICC_EOIR1_EL1, %0" : : "r"((uint64_t)iar));
}

static inline void send_sgi0_to_core(uint8_t target_core, uint8_t sgi_id) {
  uint64_t sgi_val = (1 << target_core) | ((uint64_t)sgi_id << 24);
  asm volatile("msr ICC_SGI0R_EL1, %0" : : "r"(sgi_val));
  asm volatile("dsb sy; isb");
}

static inline void send_sgi1_to_core(uint8_t target_core, uint8_t sgi_id) {
  uint64_t sgi_val = (target_core) | ((uint64_t)sgi_id << 24);
  asm volatile("msr ICC_SGI1R_EL1, %0" : : "r"(sgi_val));
  asm volatile("dsb sy; isb");
}

static inline void cpu_set_priority_mask(uint32_t priority) {
  asm volatile("msr ICC_PMR_EL1, %0" : : "r"((uint64_t)priority));
}

static inline void cpu_enable_group0_interrupts() {
  asm volatile("msr ICC_IGRPEN0_EL1, %0" : : "r"(1ULL));
  asm volatile("isb");
}

static inline void cpu_enable_group1_interrupts() {
  asm volatile("msr ICC_IGRPEN1_EL1, %0" : : "r"(1ULL));
  asm volatile("isb");
}

void gic_init_global();
void gic_init_core(int core_id);
void gic_enable_redistributor(uintptr_t rd_base);
void gic_disable_redistributor(uintptr_t rd_base);

#endif
