#include "../include/mmu.h"
#include <stdint.h>

__attribute__((aligned(4096))) uint64_t l1_table[512];
__attribute__((aligned(4096))) uint64_t l2_low[512];     // 0GB - 1GB
__attribute__((aligned(4096))) uint64_t l2_ram[512];     // 1GB - 2GB
__attribute__((aligned(4096))) uint64_t el1_l1[512];     // 1GB - 2GB
__attribute__((aligned(4096))) uint64_t el1_l2_low[512]; // 1GB - 2GB
__attribute__((aligned(4096))) uint64_t el1_l2_ram[512]; // 1GB - 2GB
__attribute__((aligned(4096))) uint64_t el1_l3[512];     // 1GB - 2GB

extern char __end[];
extern void el1_entry();

void map_uart_virtual() {
  uint64_t l2_idx = (UART_VA >> 21) & 0x1FF;
  uint64_t l3_idx = (UART_VA >> 12) & 0x1FF;

  el1_l2_low[l2_idx] = (uint64_t)el1_l3 | PTE_TYPE_TABLE;

  el1_l3[l3_idx] = UART_BASE | PTE_AF | PTE_ATTR_INDX(0) | PTE_TYPE_PAGE;
}

void setup_mmu(void) {
  // 1. MAIR: Index 0=Device, Index 1=Normal
  write_sysreg(MAIR_EL3, (0x00ULL << 0) | (0xFFULL << 8));
  write_sysreg(MAIR_EL1, (0x00ULL << 0) | (0xFFULL << 8));

  // 2. TCR: 32-bit VA space, 4KB granules
  uint64_t tcr = (32ULL << 0) | (3ULL << 12) | (1ULL << 10) | (1ULL << 8);
  write_sysreg(TCR_EL3, tcr);
  write_sysreg(TCR_EL1, tcr);

  // 3. Clear Tables
  for (int i = 0; i < 512; i++) {
    l1_table[i] = 0;
    l2_low[i] = 0;
    l2_ram[i] = 0;
    el1_l1[i] = 0;
    el1_l2_low[i] = 0;
    el1_l2_ram[i] = 0;
  }

  // 4. Link Tables (Descriptor 0x3 is a Table link)
  l1_table[0] = ((uintptr_t)l2_low | PTE_TYPE_TABLE);
  l1_table[1] = ((uintptr_t)l2_ram | PTE_TYPE_TABLE);

  el1_l1[0] = ((uintptr_t)el1_l2_low | PTE_TYPE_TABLE);
  el1_l1[1] = ((uintptr_t)el1_l2_ram | PTE_TYPE_TABLE);

  // 5. Map RAM (Identity map 256MB)
  for (int i = 0; i < 128; i++) {
    uint64_t addr = RAM_BASE + (i * 0x200000);
    l2_ram[i] = addr | PROT_NORMAL_MEM;
  }

  // 5. Map RAM (Identity map 256MB)
  for (int i = 0; i < 128; i++) {
    uint64_t addr = RAM_BASE + (i * 0x200000);
    el1_l2_ram[i] = addr | PROT_NORMAL_MEM;
  }

  // 6. Map Device I/O (UART and Bluetooth)
  l2_low[UART_BASE >> 21] = UART_BASE | PROT_DEVICE;
  l2_low[BT_BASE >> 21] = BT_BASE | PROT_DEVICE;

  map_uart_virtual();

  // 7. Commit and Enable
  write_sysreg(TTBR0_EL3, (uintptr_t)l1_table);
  write_sysreg(TTBR0_EL1, (uintptr_t)el1_l1);
  asm volatile("tlbi alle3; dsb sy; isb");
  asm volatile("tlbi alle1; dsb sy; isb");

  uint64_t sctlr = read_sysreg(SCTLR_EL3);
  uint64_t sctlr_el1 = read_sysreg(SCTLR_EL1);
  sctlr |= (1 << 0) | (1 << 2) | (1 << 12);     // M, C, I bits
  sctlr_el1 |= (1 << 0) | (1 << 2) | (1 << 12); // M, C, I bits
  write_sysreg(SCTLR_EL3, sctlr);
  write_sysreg(SCTLR_EL1, sctlr_el1);
  asm volatile("isb");
}

// ------------------------
// UART functions
// ------------------------
static void uart_putc(char c) {
  volatile uint32_t *dr = (volatile uint32_t *)UART_BASE;
  volatile uint32_t *fr = (volatile uint32_t *)(UART_BASE + 0x18);
  while (*fr & (1 << 5))
    ;
  *dr = c;
}

void uart_puts(const char *s) {
  while (*s)
    uart_putc(*s++);
}

static void uart_virt_putc(char c) {
  volatile uint32_t *dr = (volatile uint32_t *)UART_VA;
  volatile uint32_t *fr = (volatile uint32_t *)(UART_VA + 0x18);
  while (*fr & (1 << 5))
    ;
  *dr = c;
}

void uart_virt_puts(const char *s) {
  while (*s)
    uart_virt_putc(*s++);
}

void jump_to_el1() {
  uint64_t el1_stack_top = (uint64_t)__end + 0x8000;
  el1_stack_top &= ~0xF;

  asm volatile("msr sp_el1, %0" ::"r"(el1_stack_top));

  uint64_t scr = (1 << 0) | (1 << 10);
  asm volatile("msr scr_el3, %0" ::"r"(scr));

  asm volatile("msr spsr_el3, %0" ::"r"(0x3c5));
  asm volatile("msr elr_el3, %0" ::"r"(el1_entry));
  asm volatile("eret");
}

// ------------------------
// Main
// ------------------------
void main() {
  uart_puts("Booting Monitor...\n");
  setup_mmu();
  uart_puts("MMU enabled successfully!\n");

  jump_to_el1();
}

void el1_entry() {
  uart_virt_puts("Booted into EL1\n");

  while (1) {
    asm volatile("wfi");
  }
}
