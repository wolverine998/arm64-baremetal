#include "../../include/driver.h"
#include "../../include/kernel_gicv3.h"
#include "../../include/uart.h"

#define MAX_DRIVERS 32
#define DEFAULT_IRQ_PRIORITY 0x80

static driver_t driver_registry[MAX_DRIVERS];
static int driver_count = 0;

void driver_manager_init() {
  for (int i = 0; i < MAX_DRIVERS; i++) {
    driver_registry[i].irq_id = 0;
    driver_registry[i].handler = 0;
  }
  driver_count = 0;
}

int register_driver(uint32_t irq, void (*handler)(void *), void *device,
                    const char *name) {
  if (driver_count >= MAX_DRIVERS) {
    kernel_printf("[DRV] Error: Registry full!\n");
    return -1;
  }

  driver_registry[driver_count].irq_id = irq;
  driver_registry[driver_count].handler = handler;
  driver_registry[driver_count].device = device;
  driver_registry[driver_count].name = name;

  driver_count++;

  gic_conf_spi(irq, DEFAULT_IRQ_PRIORITY, 1);
  gic_route_spi(irq, 0);
  gic_enable_spi(irq);

  kernel_printf("[DRV] Registered %s on IRQ %d\n", name, irq);
  return 0;
}

void dispatch_interrupt(uint32_t irq) {
  for (int i = 0; i < driver_count; i++) {
    if (driver_registry[i].irq_id == irq) {
      if (driver_registry[i].handler) {
        driver_registry[i].handler(driver_registry[i].device);
        return;
      }
    }
  }

  kernel_printf("[DRV] Warning: No handler for IRQ %d\n", irq);
}
