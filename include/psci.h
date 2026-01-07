#ifndef __PSCI__
#define __PSCI__

#include "trap_frame.h"
#include <stdint.h>

#define PSCI_CPU_ON 1
#define PSCI_CPU_OFF 2
#define PSCI_CPU_SUSPEND 3

#define PSCI_SUCCESS 0
#define PSCI_ERR_BADDRESS -1
#define PSCI_ERR_INVALID_CPU -2
#define PSCI_ERR_NOT_SUPPORTED -3
#define PSCI_ERR_ALREADY_ONLINE -4
#define PSCI_ERR_CORE_BOOTING -5

int psci_fn_cpu_on(uint32_t target_core, uint64_t entry_point);
int psci_fn_cpu_off(uint32_t target_core);

#endif //
