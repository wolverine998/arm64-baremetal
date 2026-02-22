#ifndef __PSCI__
#define __PSCI__

#include <stdint.h>

#define PSCI_CPU_ON 0x200000A0
#define PSCI_CPU_OFF 0x200000B0
#define PSCI_CPU_SUSPEND 0x200000C0
#define PSCI_SYSTEM_RESET 0x200000D0

#define PSCI_SUCCESS 0
#define PSCI_ERR_BADDRESS 1
#define PSCI_ERR_INVALID_CPU 2
#define PSCI_ERR_NOT_SUPPORTED 3
#define PSCI_ERR_ALREADY_ONLINE 4
#define PSCI_ERR_CORE_BOOTING 5

// gpio pins for system reset/power off
#define GPIO_POWEROFF 0
#define GPIO_RESTART 1

int psci_fn_cpu_on(uint32_t target_core, uint64_t entry_point);
int psci_fn_cpu_off(uint32_t target_core);
int psci_fn_system_reset();

#endif //
