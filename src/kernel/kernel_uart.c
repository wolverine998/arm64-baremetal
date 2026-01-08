#include "../../include/uart.h"
#include <stdarg.h>
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

void kernel_printf(const char *fmt, ...) {
  // 1. Acquire Lock
  // Prevents characters from different cores from interleaving
  int expected = 0;
  while (!__atomic_compare_exchange_n(&kernel_lock, &expected, 1, false,
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
          s = "(null)";
        while (*s)
          kernel_putc(*s++);
        break;
      }

      case 'x': {
        uint64_t val = __builtin_va_arg(args, uint64_t);
        kernel_putc('0');
        kernel_putc('x');
        if (val == 0) {
          kernel_putc('0');
        } else {
          int started = 0;
          for (int i = 60; i >= 0; i -= 4) {
            uint8_t nibble = (val >> i) & 0xF;
            if (nibble != 0 || started) {
              kernel_putc((nibble < 10) ? ('0' + nibble)
                                        : ('A' + (nibble - 10)));
              started = 1;
            }
          }
        }
        break;
      }

      case 'd': {
        int64_t val = __builtin_va_arg(args, int64_t);
        if (val == 0) {
          kernel_putc('0');
        } else {
          if (val < 0) {
            kernel_putc('-');
            val = -val;
          }
          char buf[20]; // Fits max 64-bit decimal
          int i = 0;
          while (val > 0) {
            buf[i++] = (val % 10) + '0';
            val /= 10;
          }
          while (i > 0)
            kernel_putc(buf[--i]);
        }
        break;
      }

      case 'c': {
        // Char is promoted to int in variadic arguments
        char c = (char)__builtin_va_arg(args, int);
        kernel_putc(c);
        break;
      }

      default:
        kernel_putc('%');
        kernel_putc(*p);
        break;
      }
    } else {
      kernel_putc(*p);
    }
  }

  __builtin_va_end(args);

  // 2. Release Lock
  __atomic_store_n(&kernel_lock, 0, __ATOMIC_RELEASE);
}
