#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/gpio.h"
#include "../../include/irq.h"
#include "../../include/psci.h"

cpu_state_t cpus[MAX_CPUS];

int psci_fn_cpu_on(uint32_t target_core, uint64_t entry_point) {
  // check if the core is offline
  // if not, just return
  if (target_core >= MAX_CPUS)
    return PSCI_ERR_INVALID_CPU;

  if (entry_point == 0 || (entry_point) & 0x3)
    return PSCI_ERR_BADDRESS;

  if (cpus[target_core].state == ON)
    return PSCI_ERR_ALREADY_ONLINE;

  if (cpus[target_core].state == BOOTING)
    return PSCI_ERR_CORE_BOOTING;

  cpus[target_core].entry_point = entry_point;
  cpus[target_core].state = BOOTING;
  cpus[target_core].context_id = 0;
  asm volatile("dsb sy");

  // wake the core
  send_sgi0_to_core(target_core, SGI_CORE_WAKE);

  return PSCI_SUCCESS;
}

int psci_fn_cpu_off(uint32_t target_core) {
  if (cpus[target_core].state == OFF)
    return PSCI_ERR_NOT_SUPPORTED;

  send_sgi0_to_core(target_core, SGI_CORE_SLEEP);

  return PSCI_SUCCESS;
}

int psci_fn_system_reset() {
  gpio_set_direction(GPIO_RESTART, 1);

  /* Restart the board.
   * Setting the corresponding pin to 1
   * forces the hardware reset.
   * We must disable the GIC from
   * signaling interrupts completelly
   * before reseting the board
   */
  write_gicd(GICD_CTLR, 0);

  /* Wait for distributor to notify us
   * that the changes are propagated upstream
   */
  while (read_gicd(GICD_CTLR) & GICD_CTLR_RWP)
    ;
  gpio_set_high(GPIO_RESTART);

  return PSCI_ERR_BADDRESS; // this should never return
}
