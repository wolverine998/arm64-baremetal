#include "../../include/gic-v3.h"
#include "../../include/gicv3-its.h"
#include "../../include/irq.h"
#include "../../include/registers.h"
#include "../../include/syscall.h"
#include "../../include/trap_frame.h"
#include "../../include/uart.h"
#include <stdint.h>

void kernel_panic(trap_frame_t *frame, const char *msg) {
  kernel_printf("[KERNEL] %s", msg);
  kernel_printf("[KERNEL] Register dump\n");
  kernel_printf("[KERNEL] FAR: %x ELR: %x ESR: %x\n", frame->far, frame->elr,
                frame->esr);

  while (1) {
    wait_for_interrupt();
  }
}

void kernel_sync_elx(trap_frame_t *frame) {
  uint64_t esr = frame->esr;
  uint32_t ec = ESR_EC_MASK(esr);
  uint32_t iss = ESR_ISS_MASK(esr);
  uint8_t dfsc = ISS_DFSC_MASK(iss);

  if (ec == EC_DATA_ABORT || ec == EC_DATA_ABORT_LOWER) {
    uint8_t wnr = ISS_WNR(iss);

    if (dfsc >= DFSC_TRANSLATION_L0 && dfsc <= DFSC_TRANSLATION_L3) {
      kernel_printf("Translation fault (L%d) on %s at %x\n", dfsc - 4,
                    wnr ? "WRITE" : "READ", frame->far);
    } else {
      kernel_printf("Data Abort. DFSC: %x, FAR: %x\n", dfsc, frame->far);
    }

    kernel_panic(frame, "Data abort. Shutting down\n");
  } else {
    kernel_puts("FATAL EXCEPTIONNN\n");
    kernel_puts("FAR: ");
    kernel_hex(frame->far);
    kernel_puts("ESR: ");
    kernel_hex(frame->esr);
    kernel_puts("ELR: ");
    kernel_hex(frame->elr);
    kernel_panic(frame, "Unknown exception. Reseting board!!\n");
  }
}

void kernel_sync_el0(trap_frame_t *frame) {
  uint64_t esr = frame->esr;
  uint32_t ec = (esr >> 26) & 0x3F;

  if (ec == 0x15) {
    if (frame->regs[8] == SVC_CONSOLE_PRINT) {
      kernel_puts((const char *)frame->regs[0]);
    } else {
      kernel_puts("Unknown system call\n");
    }
  } else {
    kernel_sync_elx(frame);
  }
}

void kernel_irq(trap_frame_t *frame) {
  uint32_t core_id = get_core_id();
  uint32_t iar = gic_read_iar1();
  uint32_t interupt_id = INTERRUPT_ID_MASK(iar);

  if (interupt_id != 0 || interupt_id != 1023) {
    kernel_printf("Interrupt ID: %d fired on core %d\n", interupt_id,
                  get_core_id());
  }

  gic_write_eoir1(iar);
}
