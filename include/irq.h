#ifndef __IRQ__

#define __IRQ__

// SGI Group 0
#define SGI_CORE_WAKE 0
#define SGI_CORE_SLEEP 1

// SGI Group 1 Non-Secure
#define SGI_RES8_IGROUP1 8
#define SGI_RES9_IGROUP1 9
#define SGI_RES10_IGROUP1 10
#define SGI_RES11_IGROUP1 11
#define SGI_RES12_IGROUP1 12
#define SGI_RES13_IGROUP1 13
#define SGI_RES14_IGROUP1 14
#define SGI_RES15_IGROUP1 15

// SPI (to be defined)
#define SPI_RESERVED_1 100
#define SPI_RESERVED_2 101
#define SPI_RESERVED_3 102

void gic_el1_init_spi();

#endif
