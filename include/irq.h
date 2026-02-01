#ifndef __IRQ__

#define __IRQ__

// SGI Group 0
#define SGI_CORE_WAKE 0
#define SGI_CORE_SLEEP 1

// SGI Group 1 Non-Secure
#define SGI_RESERVED_1 5
#define SGI_RESERVED_2 6
#define SGI_RESERVED_3 7
#define SGI_RESERVED_4 8
#define SGI_RESERVED_5 9

// SPI (to be defined)
#define SPI_RESERVED_1 100
#define SPI_RESERVED_2 101
#define SPI_RESERVED_3 102

void gic_el1_init_spi();

#endif
