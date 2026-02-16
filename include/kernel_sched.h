#ifndef __KERNEL_SCHED__
#define __KERNEL_SCHED__

#include "gic-v3.h"
#include "registers.h"
#include "trap_frame.h"
#include <stdint.h>

#define ZERO ((void *)0)
#define MAX_TASKS 64
#define TASK_STACK_SIZE (12 * 1024) // 12KB
#define SCHEDULER_YIELD 8           // SGI ID 8
#define SCHEDULER_GC 9 // SGI ID 9 - Garbage collection of zombie tasks

typedef enum {
  TASK_EMPTY = 0,
  TASK_READY,
  TASK_RUNNING,
  TASK_ZOMBIE
} task_state_t;

typedef struct task {
  trap_frame_t *frame;
  uint32_t task_id;
  uint32_t core_id;
  task_state_t state;
  uint64_t stack_base;
  struct task *next;
} task_t;

typedef struct {
  task_t *head;
  task_t *tail;
} queue_t;

static inline task_t *get_current_task() {
  return (task_t *)read_sysreg(tpidr_el1);
}

static inline void set_current_task(task_t *t) { write_sysreg(tpidr_el1, t); }

static void yield() { send_sgi1_to_core(get_core_id(), SCHEDULER_YIELD); }

void init_sched();
void init_sched_core();
void reaper_task();
void task_exit();
void create_task(void (*entry)(void));
task_t *schedule_task(trap_frame_t *frame);
void sched_enable();
void sched_disable();

// scheduler services
void reaper_service();

#endif
