#include "../../include/irq.h"
#include "../../include/kernel_gicv3.h"

void gic_el1_init_spi() {
  gic_conf_spi(SPI_RESERVED_1, 0x40, 1);
  gic_conf_spi(SPI_RESERVED_2, 0x50, 1);
  gic_conf_spi(SPI_RESERVED_3, 0x60, 1);

  gic_route_spi(SPI_RESERVED_1, 0);
  gic_route_spi(SPI_RESERVED_2, 1);
  gic_route_spi(SPI_RESERVED_3, 1);
}
