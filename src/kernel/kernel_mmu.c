#include "../../include/gic-v3.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/uart.h"
#include <stdint.h>

// A pool of pages for tables.
// Increased to 128 to be safe.
__attribute__((aligned(4096))) static uint64_t page_pool[128][512];
static int pool_ptr = 0;

__attribute__((aligned(4096))) uint64_t el1_l1[512];

extern char _kernel_start[], _kernel_end[];
extern char _app_start[], _app_end[];

volatile uint64_t mmu_lock = 1;

static uint64_t *allocate_table() {
  if (pool_ptr >= 128)
    return 0;
  uint64_t *table = page_pool[pool_ptr++];
  for (int i = 0; i < 512; i++)
    table[i] = 0;
  return table;
}

void map_page_4k(uint64_t va, uint64_t pa, uint64_t flags) {
  uint64_t l1_idx = (va >> 30) & 0x1FF;
  if (!(el1_l1[l1_idx] & 1)) {
    el1_l1[l1_idx] = (uint64_t)allocate_table() | PTE_TYPE_TABLE;
  }
  uint64_t *l2 = (uint64_t *)(el1_l1[l1_idx] & ~0xFFFULL);

  uint64_t l2_idx = (va >> 21) & 0x1FF;
  if (!(l2[l2_idx] & 1)) {
    l2[l2_idx] = (uint64_t)allocate_table() | PTE_TYPE_TABLE;
  }
  uint64_t *l3 = (uint64_t *)(l2[l2_idx] & ~0xFFFULL);

  uint64_t l3_idx = (va >> 12) & 0x1FF;

  // Force Type 3 (Page) and ensure Access Flag is set
  l3[l3_idx] = (pa & ~0xFFFULL) | flags | PTE_TYPE_PAGE;
}

void map_region(uint64_t start, uint64_t end, uint64_t flags) {
  start &= ~0xFFFULL;
  for (uint64_t addr = start; addr < end; addr += 4096) {
    map_page_4k(addr, addr, flags); // Identity map: VA == PA
  }
}

void kernel_setup_mmu() {
  pool_ptr = 0;
  for (int i = 0; i < 512; i++)
    el1_l1[i] = 0;

  // 1. Map Kernel
  map_region(RAM_BASE, (uint64_t)_kernel_end,
             PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW);

  // 2. Map App
  map_region((uint64_t)_kernel_end, (uint64_t)_app_end,
             PROT_NORMAL_MEM | AP_EL0_RW_ELX_RW);

  // 3. Map UART
  map_page_4k(UART_BASE, UART_BASE,
              PROT_DEVICE | PTE_UXN | PTE_PXN | AP_EL0_RW_ELX_RW);

  // 4. Map GIC v3
  map_page_4k(GICD_BASE, GICD_BASE,
              PROT_DEVICE | PTE_UXN | PTE_PXN | AP_EL0_NO_ELX_RW);
  map_page_4k(GICR_BASE, GICR_BASE,
              PROT_DEVICE | PTE_UXN | PTE_PXN | AP_EL0_NO_ELX_RW);

  // Ensure TCR matches your 4KB granule and 32-bit (4GB) address space
  uint64_t tcr = TCR_EL1_T0SZ0_32 | // T0SZ: 32 bits (4GB)
                 TCR_EL1_T1SZ_32 |  // T1SZ: 32 bits (4GB)
                 TCR_TG0_4KB | TCR_SH0_INNER_SHAREABLE |
                 TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC |
                 TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC;

  write_sysreg(TCR_EL1, tcr);
  write_sysreg(TTBR0_EL1, el1_l1);

  asm volatile("dsb sy; isb");

  // Enable MMU
  uint64_t sctlr = read_sysreg(SCTLR_EL1);
  sctlr |= (SCTLR_M | SCTLR_A | SCTLR_C | SCTLR_I | SCTLR_SA | SCTLR_SA0);
  write_sysreg(SCTLR_EL1, sctlr);

  asm volatile("isb");

  mmu_lock = 0;
}

void seccore_setup_mmu() {
  while (mmu_lock)
    ;
  // Ensure TCR matches your 4KB granule and 32-bit (4GB) address space
  uint64_t tcr = TCR_EL1_T0SZ0_32 | // T0SZ: 32 bits (4GB)
                 TCR_EL1_T1SZ_32 |  // T1SZ: 32 bits (4GB)
                 TCR_TG0_4KB | TCR_SH0_INNER_SHAREABLE |
                 TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC |
                 TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC;

  write_sysreg(TCR_EL1, tcr);
  write_sysreg(TTBR0_EL1, el1_l1);

  asm volatile("dsb sy; isb");

  // Enable MMU
  uint64_t sctlr = read_sysreg(SCTLR_EL1);
  sctlr |= (SCTLR_M | SCTLR_A | SCTLR_C | SCTLR_I | SCTLR_SA | SCTLR_SA0);
  write_sysreg(SCTLR_EL1, sctlr);

  asm volatile("isb");
}
