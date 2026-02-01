#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

extern char _seeos_begin[];

__attribute__((aligned(4096))) uint64_t seeos_l1_table[512];
__attribute__((aligned(4096))) uint64_t seeos_l2_low[512]; // 0GB - 1GB
__attribute__((aligned(4096))) uint64_t seeos_l2_ram[512]; // 1GB - 2GB

void seeos_init_mmu_global(void) {
  uint64_t start = (uint64_t)_seeos_begin;
  // 3. Clear Tables
  for (int i = 0; i < 512; i++) {
    seeos_l1_table[i] = 0;
    seeos_l2_low[i] = 0;
    seeos_l2_ram[i] = 0;
  }

  uint32_t ram_l1_idx = (start >> 30) & 0x1FF;
  uint32_t uart_l1_idx = (SECURE_UART1 >> 30) & 0x1FF;

  // 4. Link Tables (Descriptor 0x3 is a Table link)
  seeos_l1_table[uart_l1_idx] = ((uintptr_t)seeos_l2_low | PTE_TYPE_TABLE);
  seeos_l1_table[ram_l1_idx] = ((uintptr_t)seeos_l2_ram | PTE_TYPE_TABLE);

  // 5. Map RAM (Identity map 256MB)
  for (int i = 0; i < 4; i++) {
    uint64_t addr = start + (i * 0x20000);
    seeos_l2_ram[(addr >> 21) & 0x1FF] =
        addr | PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW | PTE_TYPE_BLOCK;
  }

  // 6. Map Device I/O (UART and GIC)
  seeos_l2_low[(UART_BASE >> 21) & 0x1FF] = UART_BASE | PROT_DEVICE_NGNRE |
                                            AP_EL0_NO_ELX_RW | PTE_UXN |
                                            PTE_PXN | PTE_TYPE_BLOCK;

  // map GIC V3
  seeos_l2_low[(GICD_BASE >> 21) & 0x1FF] = GICD_BASE | PROT_DEVICE |
                                            AP_EL0_NO_ELX_RW | PTE_UXN |
                                            PTE_PXN | PTE_TYPE_BLOCK;

  seeos_l2_low[(GICR_BASE >> 21) & 0x1FF] = GICR_BASE | PROT_DEVICE |
                                            AP_EL0_NO_ELX_RW | PTE_UXN |
                                            PTE_PXN | PTE_TYPE_BLOCK;
}

void seeos_enable_mmu(void) {
  // 2. Set the same TCR (Control Register)
  uint64_t tcr = (32ULL << 0) | (3ULL << 12) | (1ULL << 10) | (1ULL << 8);
  write_sysreg(TCR_EL1, tcr);

  // 3. Point to the tables Core 0 already created
  write_sysreg(TTBR0_EL1, (uintptr_t)seeos_l1_table);
  instruction_barrier();
  enable_mmu_el1();
}
