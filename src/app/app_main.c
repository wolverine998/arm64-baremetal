#include "../../include/syscall.h"
#include "../../include/uart.h"
#include <stdint.h>

void app_entry() {
  svc_print("Hello from EL0\n");

  // do nothing, just loop forever
  while (1) {
    asm volatile("wfi");
  }
}
