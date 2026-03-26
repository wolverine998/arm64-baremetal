#include "../../include/see_mmu.h"
#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/see_cmd.h"
#include "../../include/stdlib.h"
#include "../../include/uart.h"
#include <stdint.h>

__attribute__((aligned(4096))) uint64_t seeos_l1_table[512];

__attribute__((aligned(4096))) uint64_t page_tables[MAX_TABLES][512];
uint32_t tables_used = 0;

uint64_t *see_alloc_table() {
  if (tables_used >= MAX_TABLES) {
    seeos_printf("[SEEOS] Could not allocate page table");
    return 0;
  }

  uint64_t *table = page_tables[tables_used++];
  mem_zero(table, 512);

  return table;
}

void see_map_page(uint64_t *l1, uint64_t va, uint64_t pa, uint64_t flags) {
  uint64_t va_addr = va & 0xFFFFFFFFULL;

  uint64_t l1_idx = TLB_L1_INDEX(va_addr);

  if (!(l1[l1_idx] & 1)) {
    l1[l1_idx] = (uint64_t)see_alloc_table() | PTE_TYPE_TABLE;
  }

  uint64_t *l2 = (uint64_t *)(l1[l1_idx] & ~0xFFFULL);
  uint64_t l2_idx = TLB_L2_INDEX(va_addr);

  if (!(l2[l2_idx] & 1)) {
    l2[l2_idx] = (uint64_t)see_alloc_table() | PTE_TYPE_TABLE;
  }

  uint64_t *l3 = (uint64_t *)(l2[l2_idx] & ~0xFFFULL);
  uint64_t l3_idx = TLB_L3_INDEX(va_addr);

  l3[l3_idx] = (pa & ~0xFFFULL) | flags | PTE_TYPE_PAGE;
}

void see_map_region(uint64_t *l1, uint64_t va_start, uint64_t pa_start,
                    uint64_t size, uint64_t flags) {
  uint64_t va = va_start & ~0xFFFULL;
  uint64_t pa = pa_start & ~0xFFFULL;

  for (int i = 0; i < size; i += 4096) {
    see_map_page(l1, va + i, pa + i, flags);
  }
}

void seeos_init_mmu_global(void) {
  uint64_t start;
  asm volatile("ldr %0, =_seeos_start" : "=r"(start));
  // 3. Clear L1 table
  mem_zero(seeos_l1_table, 512);

  see_map_region(seeos_l1_table, start, start, 0x300000,
                 PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW);

  see_map_page(seeos_l1_table, SMEM_BUFFER, SMEM_BUFFER,
               PROT_NORMAL_NC | AP_EL0_NO_ELX_RW | PTE_UXN | PTE_PXN | PTE_NS);

  see_map_page(seeos_l1_table, SECURE_UART1, SECURE_UART1,
               PROT_DEVICE_NGNRE | AP_EL0_NO_ELX_RW | PTE_UXN | PTE_PXN);

  see_map_region(seeos_l1_table, GICD_BASE, GICD_BASE, 0x10000,
                 PROT_DEVICE | AP_EL0_NO_ELX_RW | PTE_UXN | PTE_PXN);

  see_map_region(seeos_l1_table, GICR_BASE, GICR_BASE, 0x80000,
                 PROT_DEVICE | AP_EL0_NO_ELX_RW | PTE_UXN | PTE_PXN);
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
