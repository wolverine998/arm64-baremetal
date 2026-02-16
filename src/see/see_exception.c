#include "../../include/trap_frame.h"
#include "../../include/uart.h"

void seeos_sync_handler(trap_frame_t *frame) {
  seeos_puts_atomic("[SEEOS] Sync exception occured\n");
  // just do loop
  while (1)
    ;
}
