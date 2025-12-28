#include "../include/mmu.h"
#include "../include/uart.h"
#include <stdint.h>

void main() {
  setup_mmu();

  uart_puts("Booted into EL3\n");
  uart_puts("Welcome to EL3...\n");
}
