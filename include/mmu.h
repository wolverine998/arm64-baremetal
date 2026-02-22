#ifndef MMU_H
#define MMU_H

#include "registers.h"
#include <stdint.h>

// --- Descriptor Types ---
#define PTE_TYPE_TABLE (3ULL << 0)
#define PTE_TYPE_BLOCK (1ULL << 0)
#define PTE_TYPE_PAGE (3ULL << 0)

// --- Attribute Bits ---
#define PTE_S (0ULL << 5)
#define PTE_NS (1ULL << 5)       // Non-Secure
#define PTE_AF (1ULL << 10)      // Access Flag
#define PTE_SH_OUTER (2ULL << 8) // Outer Shareable
#define PTE_SH_INNER (3ULL << 8) // Inner Shareable
#define PTE_ATTR_INDX(idx) ((idx) << 2)
#define PTE_PXN (1ULL << 53)
#define PTE_UXN (1ULL << 54)

// AP Permissions
#define AP_EL0_NO_ELX_RW (0x0ULL << 6)
#define AP_EL0_RW_ELX_RW (0x1ULL << 6)
#define AP_EL0_NO_ELX_RO (0x2ULL << 6)
#define AP_EL0_RO_ELX_RO (0x3ULL << 6)

// --- MAIR Indices ---
#define ATTR_I_DEVICE_NGNRNE 0
#define ATTR_I_NORMAL 1
#define ATTR_I_DEVICE_NGNRE 2

// --- Memory Prot Settings ---
#define PROT_NORMAL_MEM (PTE_AF | PTE_SH_INNER | PTE_ATTR_INDX(ATTR_I_NORMAL))
#define PROT_DEVICE                                                            \
  (PTE_AF | PTE_SH_OUTER | PTE_ATTR_INDX(ATTR_I_DEVICE_NGNRNE))
#define PROT_DEVICE_NGNRE                                                      \
  (PTE_AF | PTE_SH_OUTER | PTE_ATTR_INDX(ATTR_I_DEVICE_NGNRE))

// TCR_EL1 TG0 GRANULE SIZE
#define TCR_TG0_4KB (0ULL << 14)
#define TCR_TG0_64KB (1ULL << 14)
#define TCR_TG0_16KB (2ULL << 14)

// TCR_EL1 TG1 GRANULE SIZE
#define TCR_TG1_16KB (1ULL << 30)
#define TCR_TG1_4KB (2ULL << 30)
#define TCR_TG1_64KB (3ULL << 30)

// shareablity EL1
#define TCR_SH1_NON_SHAREABLE (0ULL << 28)
#define TCR_SH1_OUTER_SHAREABLE (2ULL << 28)
#define TCR_SH1_INNER_SHAREABLE (3ULL << 28)

// outer chacheability TTBR1_EL1
#define TCR_ORGN1_NORMAL_MEMORY_ONC (0ULL << 26)
#define TCR_ORGN1_NORMAL_MEMORY_OWBRA_WAC (1ULL << 26)
#define TCR_ORGN1_NORMAL_MEMORY_OWTRA_NWAC (2ULL << 26)
#define TCR_ORGN1_NORMAL_MEMORY_OWBRA_NWAC (3ULL << 26)

// inner chacheability TTBR1_EL1
#define TCR_IRGN1_NORMAL_MEMORY_INC (0ULL << 24)
#define TCR_IRGN1_NORMAL_MEMORY_IWBRA_WAC (1ULL << 24)
#define TCR_IRGN1_NORMAL_MEMORY_IWTRA_NWAC (2ULL << 24)
#define TCR_IRGN1_NORMAL_MEMORY_IWBRA_NWAC (3ULL << 24)

// T1SZ TCR_EL1
#define TCR_EL1_T1SZ_32 (32 << 16)
#define TCR_EL1_T1SZ_30 (30 << 16)

// T0SZ TCR_EL1
#define TCR_EL1_T0SZ0_32 (32 << 0)
#define TCR_EL1_T0SZ0_30 (30 << 0)

// inner chacheability TTBR0_EL1
#define TCR_IRGN0_NORMAL_MEMORY_INC (0ULL << 8)
#define TCR_IRGN0_NORMAL_MEMORY_IWBRA_WAC (1ULL << 8)
#define TCR_IRGN0_NORMAL_MEMORY_IWTRA_NWAC (2ULL << 8)
#define TCR_IRGN0_NORMAL_MEMORY_IWBRA_NWAC (3ULL << 8)

// outer chacheability TTBR0_EL1
#define TCR_ORGN0_NORMAL_MEMORY_ONC (0ULL << 10)
#define TCR_ORGN0_NORMAL_MEMORY_OWBRA_WAC (1ULL << 10)
#define TCR_ORGN0_NORMAL_MEMORY_OWTRA_NWAC (2ULL << 10)
#define TCR_ORGN0_NORMAL_MEMORY_OWBRA_NWAC (3ULL << 10)

// shareablity TTBR0_EL1
#define TCR_SH0_NON_SHAREABLE (0ULL << 12)
#define TCR_SH0_OUTER_SHAREABLE (2ULL << 12)
#define TCR_SH0_INNER_SHAREABLE (3ULL << 12)

// MAIR attributes
#define MAIR_DEVICE_NGNRNE (0x00ULL << 0)
#define MAIR_RAM (0xFFULL << 8)
#define MAIR_DEVICE_NGNRE (0x04ULL << 16)

// --- Address Map ---
#define RAM_BASE 0x40000000ULL
#define SEC_RAM_BASE 0x0E000000ULL
#define KERNEL_VIRT_BASE 0xFFFFFFFF00000000

#define RAM_SIZE_MB 512
#define RAM_SIZE_BYTES (RAM_SIZE_MB * 1024 * 1024)
#define RAM_END (RAM_BASE + RAM_SIZE_BYTES)

#define VA_TO_PA(ptr) ((uint64_t)(ptr) - (uint64_t)(KERNEL_VIRT_BASE))
#define PA_TO_VA(ptr) ((uint64_t)(ptr) + (uint64_t)(KERNEL_VIRT_BASE))

/* Some helpful helpers
 * for extracting the virtual address
 * translation table index */

#define TLB_L1_INDEX(x) ((x >> 30) & 0x1FF)
#define TLB_L2_INDEX(x) ((x >> 21) & 0x1FF)
#define TLB_L3_INDEX(x) ((x >> 12) & 0x1FF)

// mmu helper functions
void setup_mmu();
void setup_mmu_secondary();
void kernel_setup_mmu();
void map_page_4k(uint64_t *root, uint64_t va, uint64_t pa, uint64_t flags);
void map_region(uint64_t *root, uint64_t va_start, uint64_t pa_start,
                uint64_t size, uint64_t flags);
void unmap_region(uint64_t *root, uint64_t va, uint64_t size);
void unmap_region_virtual(uint64_t *root, uint64_t va, uint64_t size);
void map_page_virtual(uint64_t *root, uint64_t va, uint64_t pa, uint64_t flags);
void map_region_virtual(uint64_t *root, uint64_t va_start, uint64_t pa_start,
                        uint64_t size, uint64_t flags);
void map_block_range(uint64_t start, uint64_t size, uint64_t flags);
void seeos_init_mmu_global();
void seeos_enable_mmu();

static inline void enable_mmu_el1() {
  uint64_t sctlr = read_sysreg(sctlr_el1);
  sctlr |= SCTLR_M | SCTLR_I | SCTLR_C | SCTLR_SA | SCTLR_SA0;
  write_sysreg(sctlr_el1, sctlr);
  asm volatile("isb");
}

static inline void disable_mmu_el1() {
  uint64_t sctlr = read_sysreg(sctlr_el1);
  sctlr &= ~(SCTLR_M);
  write_sysreg(sctlr_el1, sctlr);
  write_sysreg(ttbr0_el1, 0);
  write_sysreg(ttbr1_el1, 0);
  write_sysreg(tcr_el1, 0);
  asm volatile("isb");
  asm volatile("dsb ishst");
  asm volatile("tlbi vmalle1is");
  asm volatile("dsb ish");
  asm volatile("isb");
}
/* Flush EL1 cached entries in mmu table.
 * Broadcasted to all PE's in Inner Domain
 */
static inline void tlb_flush_all_e1() {
  asm volatile("dsb ishst\n"
               "tlbi vmalle1is\n"
               "dsb ish\n"
               "isb \n");
}

static inline void tlb_flush_page_el1(uint64_t va) {
  uint64_t operand = va >> 12;
  asm volatile("dsb ishst\n"
               "tlbi vaale1is, %0\n"
               "dsb ish\n"
               "isb\n" ::"r"(operand)
               : "memory");
}

static inline void tlb_flush_range_el1(uint64_t va, uint8_t num,
                                       uint8_t scale) {
  uint64_t xt = 0;

  xt |= (va >> 12) & 0x1FFFFFFFFFULL;

  // ttl level 3
  xt |= (3ULL << 37);

  // num bits 43:39
  xt |= ((uint64_t)(num & 0x1F) << 39);

  // scale bits 45:44
  xt |= ((uint64_t)(scale & 0x3) << 44);

  // translation granule bits 47:46
  xt |= (1ULL << 46);

  asm volatile("dsb ishst\n"
               "tlbi rvaae1is, %0\n"
               "dsb ish\n"
               "isb\n" ::"r"(xt)
               : "memory");
}

#endif
