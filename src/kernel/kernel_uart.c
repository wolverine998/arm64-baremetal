#include "../../include/uart.h"
#include <stdbool.h>
#include <stdint.h>
// ------------------------
// UART functions
// ------------------------
void kernel_putc(char c) {
  volatile uint32_t *dr = (volatile uint32_t *)UART_BASE;
  volatile uint32_t *fr = (volatile uint32_t *)(UART_BASE + 0x18);
  while (*fr & (1 << 5))
    ;
  *dr = c;
}

void kernel_puts_old(const char *s) {
  while (*s)
    kernel_putc(*s++);
}

void kernel_hex(uint64_t value) {
  for (int i = 60; i >= 0; i -= 4) {
    uint8_t nibble = (value >> i) & 0xF;
    // This uses registers only—no memory loads from .rodata
    char c = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
    kernel_putc(c);
  }
  kernel_puts("\n");
}

// Use a simple volatile int as the lock
static volatile int kernel_lock = 0;

void kernel_puts(const char *s) {
  // 1. Acquire Lock
  // __ATOMIC_ACQUIRE ensures no later UART writes happen before this
  int expected = 0;
  while (!__atomic_compare_exchange_n(&kernel_lock, &expected, 1, false,
                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
    expected =
        0; // Reset expected because the intrinsic overwrites it on failure
    asm volatile("yield"); // Optimization: tell CPU we are spinning
  }

  // 2. Critical Section
  while (*s) {
    kernel_putc(*s++);
  }

  // 3. Release Lock
  // __ATOMIC_RELEASE ensures all UART writes are finished before clearing the
  // lock
  __atomic_store_n(&kernel_lock, 0, __ATOMIC_RELEASE);
}
