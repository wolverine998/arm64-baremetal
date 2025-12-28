#ifndef MMU_H
#define MMU_H

#include <stdint.h>

// --- System register helpers ---
#define read_sysreg(reg)                                                       \
  ({                                                                           \
    uint64_t val;                                                              \
    asm volatile("mrs %0, " #reg : "=r"(val));                                 \
    val;                                                                       \
  })

#define write_sysreg(reg, val)                                                 \
  asm volatile("msr " #reg ", %0" ::"r"((uint64_t)(val)))

// --- Descriptor Types ---
#define PTE_TYPE_TABLE (3ULL << 0)
#define PTE_TYPE_BLOCK (1ULL << 0)
#define PTE_TYPE_PAGE (3ULL << 0)

// --- Attribute Bits ---
#define PTE_S (0ULL << 5)
#define PTE_NS (1ULL << 5)       // Non-Secure
#define PTE_AF (1ULL << 10)      // Access Flag
#define PTE_SH_INNER (3ULL << 8) // Inner Shareable
#define PTE_ATTR_INDX(idx) ((idx) << 2)
#define PTE_PXN (1ULL << 53)
#define PTE_UXN (1ULL << 54)

// AP Permissions
#define AP_EL0_NO_ELX_RW (0ULL << 6)
#define AP_EL0_RW_ELX_RW (1ULL << 6)
#define AP_EL0_NO_ELX_RO (2ULL << 6)
#define AP_EL0_RO_ELX_RO (3ULL << 6)

// --- MAIR Indices ---
#define ATTR_I_DEVICE 0
#define ATTR_I_NORMAL 1

// --- Memory Prot Settings ---
#define PROT_NORMAL_MEM                                                        \
  (PTE_TYPE_BLOCK | PTE_AF | PTE_SH_INNER | PTE_ATTR_INDX(ATTR_I_NORMAL))
#define PROT_DEVICE (PTE_TYPE_BLOCK | PTE_AF | PTE_ATTR_INDX(ATTR_I_DEVICE))

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

// T0SZ TCR_EL1
#define TCR_EL1_T0SZ0_32 (32 << 0)

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

// --- Address Map ---
#define RAM_BASE 0x40000000
#define UART_BASE 0x09000000
#define UART_VA 0x00600000
#define BT_BASE 0x02F00000

// mmu helper functions
void setup_mmu();
void map_page_4k(uint64_t va, uint64_t pa, uint64_t flags);

#endif
