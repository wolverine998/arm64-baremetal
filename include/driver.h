#ifndef __DRIVER__
#define __DRIVER__

#include <stdint.h>

typedef struct {
  uint32_t irq_id;
  void (*handler)(void *dev);
  void *device;
  const char *name;
} driver_t;

void driver_manager_init();
int register_driver(uint32_t irq, void (*handler)(void *), void *device,
                    const char *name);
void dispatch_interrupt(uint32_t irq);

#endif
