#ifndef __KERNEL_SCHED__
#define __KERNEL_SCHED__

#include "trap_frame.h"
#include <stdint.h>

#define MAX_TASKS 10

typedef enum {
  TASK_EMPTY = 0,
  TASK_READY,
  TASK_RUNNING,
  TASK_SLEEPING,
  TASK_ZOMBIE
} task_state_t;

typedef struct {
  uint32_t task_id;
  uint32_t core_id;
  uint64_t stack_base;
  task_state_t state;
  void (*handler)(void);
  trap_frame_t *frame;
} task_t;

extern uint32_t total_tasks;
extern uint32_t running_task_id;
extern task_t tasks[MAX_TASKS];

void init_sched();
void idle_task();
void create_task(void (*handler)(void));
task_t *schedule_task(trap_frame_t *frame);

#endif
