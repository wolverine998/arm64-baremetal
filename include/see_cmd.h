// Secure Execution OS communication driver
#ifndef __SEE_CMD__
#define __SEE_CMD__

#include "smc.h"
#include <stdint.h>

// Shared buffer base address (non-secure RAM)
#define SMEM_BUFFER 0x40FFFFFF

typedef enum {
  CMD_EMPTY = 0,
  CMD_BAD_ID,
  CMD_WRONG_ARGS,
  CMD_UNDEFINED,
  CMD_ERROR,
  CMD_SUCCESS
} cmd_response_t;

/* SEEOS command package, passed by the kernel
 * when invoking seeos cmd API.
 * Current size is 72 bytes
 */
typedef struct {
  uint32_t command;        // 4 byte command id
  uint64_t data[4];        // 32 bytes data
  cmd_response_t response; // 4 byte response value
  uint64_t result[4];      // 32 bytes result
} see_cmd_t;

/* void see_call();
 * Invokes the SEEOS command handler.
 * Before invoking the method, SMEM_BUFFER must contain
 * the data in format exactly as struct see_cmd_t.
 */
static inline void see_call() {
  smc_res_t res;
  smc_call(SEEOS_COMMAND, &res, 0, 0, 0, 0, 0);
}
#endif
