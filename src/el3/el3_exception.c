#include "../../include/gic-v3.h"
#include "../../include/registers.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();

void el3_sync(trap_frame_t *frame) {
  uart_puts("Some exception occured\n");

  uart_puts("FAR: ");
  uart_hex(frame->far);
}

void el3_fiq(trap_frame_t *frame) {
  uint32_t iar = gic_read_iar0();

  if ((iar & 0x3FF) == 1) {
    uart_puts("Core received SGI 1\n");

    // prepare drop
    uint64_t scr = RW_AARCH64 | FIQ_ROUTE | NS;
    uint64_t spsr = SPSR_M_EL1H;

    write_sysreg(scr_el3, scr);
    frame->spsr = spsr;
    frame->elr = (uint64_t)_kernel_entry;

    gic_write_eoir0(iar);
  }
}
