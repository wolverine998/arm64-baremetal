// ARM Generic Timer
#ifndef __GENERIC_TIMER__
#define __GENERIC_TIMER__

#include "registers.h"

// EL1 Physical Timer Interrupt
#define CNTP_INTID 30

// generic timer bits
#define CNTP_CTL_EL0_ENABLE (1 << 0)
#define CNTP_CTL_EL0_IMASK (1 << 1)
#define CNTP_CTL_EL0_ISTATUS (1 << 2)

static inline void timer_enable() {
  uint32_t val = read_sysreg(cntp_ctl_el0);
  val |= CNTP_CTL_EL0_ENABLE;

  write_sysreg(cntp_ctl_el0, val);
  asm volatile("isb");
}

static inline void timer_disable() {
  uint32_t val = read_sysreg(cntp_ctl_el0);
  val &= ~CNTP_CTL_EL0_ENABLE;

  write_sysreg(cntp_ctl_el0, val);
  asm volatile("isb");
}

static inline void timer_enable_interrupts() {
  uint32_t val = read_sysreg(cntp_ctl_el0);
  val &= ~CNTP_CTL_EL0_IMASK;

  write_sysreg(cntp_ctl_el0, val);
  asm volatile("isb");
}

static inline void timer_disable_interrupts() {
  uint32_t val = read_sysreg(cntp_ctl_el0);
  val |= CNTP_CTL_EL0_IMASK;

  write_sysreg(cntp_ctl_el0, val);
  asm volatile("isb");
}

static inline void timer_set_cval(uint64_t ticks) {
  // first read the current value of timer
  uint64_t ct = read_sysreg(cntpct_el0);
  uint64_t cval = ct + ticks;

  write_sysreg(cntp_cval_el0, cval);
  asm volatile("isb");
}

static inline void timer_countdown(uint32_t ms) {
  uint64_t freq = read_sysreg(cntfrq_el0);
  write_sysreg(cntp_tval_el0, freq * (ms / 1000));
}

#endif
