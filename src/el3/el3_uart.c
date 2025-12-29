#include "../include/uart.h"
#include <stdint.h>
// ------------------------
// UART functions
// ------------------------
void uart_putc(char c) {
  volatile uint32_t *dr = (volatile uint32_t *)UART_VA;
  volatile uint32_t *fr = (volatile uint32_t *)(UART_VA + 0x18);
  while (*fr & (1 << 5))
    ;
  *dr = c;
}

void uart_puts(const char *s) {
  while (*s)
    uart_putc(*s++);
}
