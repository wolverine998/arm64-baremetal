#include "../include/uart.h"

void app_entry() {
  uart_puts("App initialized.\n");
  // do nothing, just loop forever
  while (1)
    ;
}
