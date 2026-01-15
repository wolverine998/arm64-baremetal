#ifndef __CPU_STATE__
#define __CPU_STATE__

#include "registers.h"
#include "trap_frame.h"
#include <stdint.h>

#define MAX_CPUS 8

typedef enum { OFF = 0, ON, IDLE, BOOTING } power_state_t;

typedef struct {
  uint64_t regs[31];
  uint64_t sp;
  uint64_t vbar;
  uint64_t ttbr0;
  uint64_t ttbr1;
  uint64_t tcr;
  uint64_t sctlr;
  uint64_t elr;
  uint8_t initialized;
} context_t;

typedef struct {
  volatile power_state_t state;
  volatile uint64_t entry_point;
  volatile uint8_t context_id;
  context_t ns_context;
  context_t s_context;
} cpu_state_t;

extern cpu_state_t cpus[MAX_CPUS];

static inline void save_context(context_t *ctx) {
  ctx->sp = read_sysreg(sp_el1);
  ctx->ttbr0 = read_sysreg(ttbr0_el1);
  ctx->ttbr1 = read_sysreg(ttbr1_el1);
  ctx->tcr = read_sysreg(tcr_el1);
  ctx->vbar = read_sysreg(vbar_el1);
  ctx->sctlr = read_sysreg(sctlr_el1);
  ctx->initialized = 1;
}

static inline void restore_context(context_t *ctx) {
  if (!ctx->initialized)
    return;

  write_sysreg(sp_el1, ctx->sp);
  write_sysreg(sctlr_el1, ctx->sctlr);
  write_sysreg(ttbr0_el1, ctx->ttbr0);
  write_sysreg(ttbr1_el1, ctx->ttbr1);
  write_sysreg(tcr_el1, ctx->tcr);
  write_sysreg(vbar_el1, ctx->vbar);

  asm volatile("isb; tlbi vmalle1; dsb sy; isb");
}

#endif
