#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

__attribute__((aligned(4096))) uint64_t l1_table[512];
__attribute__((aligned(4096))) uint64_t l2_low[512]; // 0GB - 1GB
__attribute__((aligned(4096))) uint64_t l2_ram[512]; // 1GB - 2GB

volatile uint64_t mmu_ready = 0;

void setup_mmu(void) {
  // 1. MAIR: Index 0=Device, Index 1=Normal
  write_sysreg(MAIR_EL3, (0x00ULL << 0) | (0xFFULL << 8));
  write_sysreg(MAIR_EL1, (0x00ULL << 0) | (0xFFULL << 8));

  // 2. TCR: 32-bit VA space, 4KB granules
  uint64_t tcr = (32ULL << 0) | (3ULL << 12) | (1ULL << 10) | (1ULL << 8);
  write_sysreg(TCR_EL3, tcr);

  // 3. Clear Tables
  for (int i = 0; i < 512; i++) {
    l1_table[i] = 0;
    l2_low[i] = 0;
    l2_ram[i] = 0;
  }

  // 4. Link Tables (Descriptor 0x3 is a Table link)
  l1_table[0] = ((uintptr_t)l2_low | PTE_TYPE_TABLE);
  l1_table[1] = ((uintptr_t)l2_ram | PTE_TYPE_TABLE);

  // 5. Map RAM (Identity map 256MB)
  for (int i = 0; i < 128; i++) {
    uint64_t addr = RAM_BASE + (i * 0x200000);
    l2_ram[i] =
        addr | PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW | PTE_S | PTE_TYPE_BLOCK;
  }

  // 6. Map Device I/O (UART and Bluetooth)
  l2_low[(UART_BASE >> 21) & 0x1FF] = UART_BASE | PROT_DEVICE |
                                      AP_EL0_NO_ELX_RW | PTE_UXN | PTE_S |
                                      PTE_TYPE_BLOCK;

  // map GIC V3
  l2_low[(GICD_BASE >> 21) & 0x1FF] = GICD_BASE | PROT_DEVICE |
                                      AP_EL0_NO_ELX_RW | PTE_UXN | PTE_S |
                                      PTE_TYPE_BLOCK;

  l2_low[(GICR_BASE >> 21) & 0x1FF] = GICR_BASE | PROT_DEVICE |
                                      AP_EL0_NO_ELX_RW | PTE_UXN | PTE_S |
                                      PTE_TYPE_BLOCK;

  // 7. Commit and Enable
  write_sysreg(TTBR0_EL3, (uintptr_t)l1_table);
  asm volatile("tlbi alle3; dsb sy; isb");

  uint64_t sctlr = read_sysreg(SCTLR_EL3);
  sctlr |= SCTLR_M | SCTLR_A | SCTLR_C | SCTLR_I | SCTLR_SA;
  write_sysreg(SCTLR_EL3, sctlr);
  asm volatile("isb");

  mmu_ready = 1;
}

void setup_mmu_secondary(void) {
  while (!mmu_ready)
    ;

  // 1. Set the same MAIR (Memory Attributes) as Core 0
  write_sysreg(MAIR_EL3, (0x00ULL << 0) | (0xFFULL << 8));
  write_sysreg(MAIR_EL1, (0x00ULL << 0) | (0xFFULL << 8));

  // 2. Set the same TCR (Control Register)
  uint64_t tcr = (32ULL << 0) | (3ULL << 12) | (1ULL << 10) | (1ULL << 8);
  write_sysreg(TCR_EL3, tcr);

  // 3. Point to the tables Core 0 already created
  write_sysreg(TTBR0_EL3, (uintptr_t)l1_table);

  // 4. Invalidate local TLB and ensure memory visibility
  asm volatile("tlbi alle3; dsb sy; isb");

  // 5. Flip the "On" switch
  uint64_t sctlr = read_sysreg(SCTLR_EL3);
  sctlr |= SCTLR_M | SCTLR_A | SCTLR_C | SCTLR_I | SCTLR_SA;
  write_sysreg(SCTLR_EL3, sctlr);

  asm volatile("isb");
}
