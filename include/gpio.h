// ARM Primecell PL061 GPIO Controller
#ifndef __GPIO__
#define __GPIO__

#include <stdint.h>

// MMIO registers
#define GPIO_BASE 0x090B0000
#define GPIO_DATA 0x0000
#define GPIO_DIR 0x0400
#define GPIO_IS 0x0404
#define GPIO_IBE 0x0408
#define GPIO_IEV 0x040C
#define GPIO_IE 0x0410
#define GPIO_RIS 0x0414
#define GPIO_MIS 0x0418
#define GPIO_IC 0x041C
#define GPIO_AFSEL 0x0420
#define GPIO_PERIPHID0 0x0FE0
#define GPIO_PERIPHID1 0x0FE4
#define GPIO_PERIPHID2 0x0FE8
#define GPIO_PERIPHID3 0x0FEC
#define GPIO_CELLID0 0x0FF0
#define GPIO_CELLID1 0x0FF4
#define GPIO_CELLID2 0x0FF8
#define GPIO_CELLID3 0x0FFC

static inline void gpio_write(uint64_t offset, uint32_t val) {
  *(volatile uint32_t *)(GPIO_BASE + offset) = val;
}

static inline uint32_t gpio_read(uint64_t offset) {
  return *(volatile uint32_t *)(GPIO_BASE + offset);
}

static inline void gpio_set_low(uint8_t pin) {
  uint32_t mask = (1 << pin) << 2;

  gpio_write(GPIO_DATA + mask, 0);
}

static inline void gpio_set_high(uint8_t pin) {
  uint32_t mask = (1 << pin) << 2;

  gpio_write(GPIO_DATA + mask, 1 << pin);
}

static inline void gpio_set_direction(uint8_t pin, uint8_t output) {
  // 0-input 1-output
  uint8_t val = 0;
  if (output)
    val |= (1 << pin);
  else
    val &= ~(1 << pin);

  gpio_write(GPIO_DIR, val);
}

#endif
