#include "../../include/mmu.h"
#include "../../include/syscall.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

void kernel_sync(trap_frame_t *frame) {
  kernel_puts("FATAL EXCEPTIONNN\n");
  kernel_puts("FAR: ");
  kernel_hex(frame->far);
  kernel_puts("ESR: ");
  kernel_hex(frame->esr);
  kernel_puts("ELR: ");
  kernel_hex(frame->elr);
  while (1)
    ; // just loop
}

void kernel_sync_el0(trap_frame_t *frame) {
  uint64_t esr = frame->esr;
  uint32_t ec = (esr >> 26) & 0x3F;

  if (ec == 0x15) {
    if (frame->regs[8] == SVC_CONSOLE_PRINT) {
      kernel_puts((const char *)frame->regs[0]);

      kernel_puts("Returning to EL0. ELR: ");
      kernel_hex(frame->elr);
    } else {
      kernel_puts("Unknown system call\n");
    }
  }
}
