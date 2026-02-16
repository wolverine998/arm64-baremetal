#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

__attribute__((aligned(4096))) uint64_t l1_table[512];
__attribute__((aligned(4096))) uint64_t l2_pool[12][512]; // pool of tables

volatile uint64_t mmu_ready = 0;
volatile uint8_t pool_index = 0;

uint64_t *alloc_table() {
  if (pool_index >= 12)
    return 0;

  uint64_t *table = l2_pool[pool_index++];

  for (int i = 0; i < 512; i++) {
    table[i] = 0;
  }

  return table;
}

void map_block_range(uint64_t start, uint64_t size, uint64_t flags) {
  uint64_t l1_index = TLB_L1_INDEX(start);

  // we only need to map l3 table descriptors
  // check if there is a l2 table already
  // if not, allocate new one

  // well map the 4GB for now
  // we just need to use the RAM_BASE index
  if (!(l1_table[l1_index] & 1)) {
    l1_table[l1_index] = (uint64_t)alloc_table() | PTE_TYPE_TABLE;
  }
  uint64_t *l2 = (uint64_t *)(l1_table[l1_index] & ~0xFFFULL);

  // map in chunks of 2 MB
  for (int i = 0; i < size; i++) {
    uint64_t addr = start + (i * 0x200000);
    uint64_t l2_index = TLB_L2_INDEX(addr);
    l2[l2_index] = addr | flags | PTE_TYPE_BLOCK;
  }
}

void setup_mmu(void) {
  // 1. MAIR: Index 0=Device, Index 1=Normal
  uint64_t mair = MAIR_DEVICE_NGNRNE | MAIR_RAM | MAIR_DEVICE_NGNRE;
  write_sysreg(MAIR_EL3, mair);
  write_sysreg(MAIR_EL1, mair);

  // 2. TCR: 32-bit VA space, 4KB granules
  uint64_t tcr = TCR_EL1_T0SZ0_32 | TCR_TG0_4KB |
                 TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC |
                 TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC | TCR_SH0_INNER_SHAREABLE;
  write_sysreg(TCR_EL3, tcr);

  // 3. Clear Tables
  for (int i = 0; i < 512; i++) {
    l1_table[i] = 0;
  }

  // Map 6MB for the EL3 monitor
  // Thats 3 blocks
  map_block_range(RAM_BASE, 6, PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW);

  // GICD - GICR
  map_block_range(GICD_BASE, 1, PROT_DEVICE | AP_EL0_NO_ELX_RW | PTE_UXN);
  map_block_range(GICR_BASE, 1, PROT_DEVICE | AP_EL0_NO_ELX_RW | PTE_UXN);
  // UART
  map_block_range(UART_BASE, 1, PROT_DEVICE_NGNRE | AP_EL0_NO_ELX_RW | PTE_UXN);

  // 7. Commit and Enable
  write_sysreg(TTBR0_EL3, l1_table);
  asm volatile("tlbi alle3; dsb sy; isb");

  uint64_t sctlr = read_sysreg(SCTLR_EL3);
  sctlr |= SCTLR_M | SCTLR_C | SCTLR_I | SCTLR_SA;
  write_sysreg(SCTLR_EL3, sctlr);
  asm volatile("isb");

  mmu_ready = 1;
}

void setup_mmu_secondary(void) {
  while (!mmu_ready)
    ;

  // 1. Set the same MAIR (Memory Attributes) as Core 0
  uint64_t mair = MAIR_DEVICE_NGNRNE | MAIR_RAM | MAIR_DEVICE_NGNRE;
  write_sysreg(MAIR_EL3, mair);
  write_sysreg(MAIR_EL1, mair);

  // 2. Set the same TCR (Control Register)
  uint64_t tcr = TCR_EL1_T0SZ0_32 | TCR_TG0_4KB |
                 TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC |
                 TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC | TCR_SH0_INNER_SHAREABLE;
  write_sysreg(TCR_EL3, tcr);

  // 3. Point to the tables Core 0 already created
  write_sysreg(TTBR0_EL3, l1_table);

  // 4. Invalidate local TLB and ensure memory visibility
  asm volatile("tlbi alle3; dsb sy; isb");

  // 5. Flip the "On" switch
  uint64_t sctlr = read_sysreg(SCTLR_EL3);
  sctlr |= SCTLR_M | SCTLR_C | SCTLR_I | SCTLR_SA;
  write_sysreg(SCTLR_EL3, sctlr);

  asm volatile("isb");
}
