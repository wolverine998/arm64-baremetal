#include "../../include/uart.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
// ------------------------
// UART functions
// ------------------------

void seeos_putc(char c) {
  volatile uint32_t *dr = (volatile uint32_t *)SECURE_UART1;
  volatile uint32_t *fr = (volatile uint32_t *)(SECURE_UART1 + 0x18);
  while (*fr & (1 << 5))
    ;
  *dr = c;
}

void seeos_puts(const char *s) {
  while (*s)
    seeos_putc(*s++);
}

void seeos_hex(uint64_t value) {
  for (int i = 60; i >= 0; i -= 4) {
    uint8_t nibble = (value >> i) & 0xF;
    // This uses registers only—no memory loads from .rodata
    char c = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
    seeos_putc(c);
  }
  seeos_puts_atomic("\n");
}

// Use a simple volatile int as the lock
static volatile int _lock = 0;

void seeos_puts_atomic(const char *s) {
  // 1. Acquire Lock
  // __ATOMIC_ACQUIRE ensures no later UART writes happen before this
  int expected = 0;
  while (!__atomic_compare_exchange_n(&_lock, &expected, 1, false,
                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
    expected =
        0; // Reset expected because the intrinsic overwrites it on failure
    asm volatile("yield"); // Optimization: tell CPU we are spinning
  }

  // 2. Critical Section
  while (*s) {
    seeos_putc(*s++);
  }

  // 3. Release Lock
  // __ATOMIC_RELEASE ensures all UART writes are finished before clearing the
  // lock
  __atomic_store_n(&_lock, 0, __ATOMIC_RELEASE);
}

void seeos_printf(const char *fmt, ...) {
  // 1. Acquire Lock
  // Prevents characters from different cores from interleaving
  int expected = 0;
  while (!__atomic_compare_exchange_n(&_lock, &expected, 1, false,
                                      __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)) {
    expected = 0;
    asm volatile("yield");
  }

  __builtin_va_list args;
  __builtin_va_start(args, fmt);

  for (const char *p = fmt; *p != '\0'; p++) {
    if (*p == '%') {
      p++; // Move to specifier

      switch (*p) {
      case 's': {
        char *s = __builtin_va_arg(args, char *);
        if (!s)
          s = 0;
        while (*s)
          seeos_putc(*s++);
        break;
      }

      case 'x': {
        uint64_t val = __builtin_va_arg(args, uint64_t);
        seeos_putc('0');
        seeos_putc('x');
        if (val == 0) {
          seeos_putc('0');
        } else {
          int started = 0;
          for (int i = 60; i >= 0; i -= 4) {
            uint8_t nibble = (val >> i) & 0xF;
            if (nibble != 0 || started) {
              seeos_putc((nibble < 10) ? ('0' + nibble)
                                       : ('A' + (nibble - 10)));
              started = 1;
            }
          }
        }
        break;
      }
      }
    }
  }
}
