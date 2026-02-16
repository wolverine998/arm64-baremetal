#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

__attribute__((aligned(4096))) uint64_t seeos_l1_table[512];
__attribute__((aligned(4096))) uint64_t seeos_l2_low[512]; // 0GB - 1GB
__attribute__((aligned(4096))) uint64_t seeos_l2_ram[512]; // 1GB - 2GB

void seeos_init_mmu_global(void) {
  uint64_t start;
  asm volatile("ldr %0, =_seeos_start" : "=r"(start));
  // 3. Clear Tables
  for (int i = 0; i < 512; i++) {
    seeos_l1_table[i] = 0;
    seeos_l2_low[i] = 0;
    seeos_l2_ram[i] = 0;
  }

  // 4. Link Tables (Descriptor 0x3 is a Table link)
  seeos_l1_table[0] = (uint64_t)seeos_l2_low | PTE_TYPE_TABLE;
  seeos_l1_table[1] = (uint64_t)seeos_l2_ram | PTE_TYPE_TABLE;

  // 5. Map RAM (Identity map 6MB)
  for (int i = 0; i < 3; i++) {
    uint64_t addr = start + (i * 0x200000);
    uint32_t l2_indx = (addr >> 21) & 0x1FF;
    seeos_l2_ram[l2_indx] =
        addr | PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW | PTE_TYPE_BLOCK;
  }

  // 6. Map Device I/O (UART and GIC)
  seeos_l2_low[(SECURE_UART1 >> 21) & 0x1FF] =
      SECURE_UART1 | PROT_DEVICE_NGNRE | AP_EL0_NO_ELX_RW | PTE_UXN | PTE_PXN |
      PTE_TYPE_BLOCK;

  // map GIC V3
  seeos_l2_low[(GICD_BASE >> 21) & 0x1FF] = GICD_BASE | PROT_DEVICE |
                                            AP_EL0_NO_ELX_RW | PTE_UXN |
                                            PTE_PXN | PTE_TYPE_BLOCK;

  seeos_l2_low[(GICR_BASE >> 21) & 0x1FF] = GICR_BASE | PROT_DEVICE |
                                            AP_EL0_NO_ELX_RW | PTE_UXN |
                                            PTE_PXN | PTE_TYPE_BLOCK;
}

void seeos_enable_mmu() {
  uint64_t tcr = TCR_EL1_T0SZ0_32 | TCR_TG0_4KB | TCR_SH0_INNER_SHAREABLE |
                 TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC |
                 TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC;

  write_sysreg(TCR_EL1, tcr);
  write_sysreg(TTBR0_EL1, seeos_l1_table);
  asm volatile("tlbi vmalle1is");
  asm volatile("dsb sy; isb");

  uint64_t sctlr = read_sysreg(SCTLR_EL1);
  sctlr |= SCTLR_M | SCTLR_C | SCTLR_I | SCTLR_SA | SCTLR_SA0;
  write_sysreg(SCTLR_EL1, sctlr);

  asm volatile("isb");
}
