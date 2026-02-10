#include "../../include/kernel_sched.h"
#include "../../include/mm.h"
#include "../../include/registers.h"
#include "../../include/uart.h"

// Define the stack size in bytes (12 KB)
#define TASK_STACK_SIZE (12 * 1024)

task_t tasks[MAX_TASKS];
uint32_t total_tasks = 1;
uint32_t running_task_id = 0; // Starting at 0 for the main kernel thread

void idle_task() {
  // just loop.and wait for scheduler to loop through other tasks
  kernel_printf("No more tasks. Going to sleep\n");

  while (1) {
    wait_for_interrupt();
  }
}

void init_sched() {
  tasks[0].task_id = 0;
  tasks[0].core_id = get_core_id();
  tasks[0].state = TASK_READY;
  tasks[0].handler = idle_task;

  uint32_t num_pages = TASK_STACK_SIZE / PAGE_SIZE;
  uint64_t stack_base = (uint64_t)mm_allocate_pages(num_pages);
  uint64_t stack_top = stack_base + (num_pages * PAGE_SIZE);
  trap_frame_t *frame = (trap_frame_t *)(stack_top - sizeof(trap_frame_t));

  for (int i = 0; i < 31; i++)
    frame->regs[i] = 0;

  frame->spsr = SPSR_M_EL1H;
  frame->elr = (uint64_t)idle_task;

  tasks[0].frame = frame;
  tasks[0].stack_base = stack_base;

  total_tasks = 1;
}

void task_exit() {
  tasks[running_task_id].state = TASK_ZOMBIE;
  kernel_printf("[SCHED] Task %d exited. Waiting for reschedule!\n",
                running_task_id);

  while (1) {
    wait_for_interrupt();
  }
}

void create_task(void (*handler)()) {
  if (total_tasks >= MAX_TASKS) {
    kernel_printf("[SCHED] ERROR: Maximum tasks reached!\n");
    return;
  }

  // Use a local ID to prevent the scheduler from seeing a partial task
  uint32_t id = total_tasks;
  uint32_t num_pages = TASK_STACK_SIZE / PAGE_SIZE;
  uint64_t stack_base = (uint64_t)mm_allocate_pages(num_pages);
  uint64_t stack_top = stack_base + TASK_STACK_SIZE;
  trap_frame_t *frame = (trap_frame_t *)(stack_top - sizeof(trap_frame_t));

  if (stack_base == 0) {
    kernel_printf("[SCHED] ERROR: Failed to allocate stack for task %d\n", id);
    return;
  }

  // Initialize registers to 0
  for (int i = 0; i < 31; i++) {
    frame->regs[i] = 0;
  }

  frame->regs[30] = (uint64_t)task_exit;
  frame->spsr = SPSR_M_EL1H; // EL1H DAIF=0
  frame->elr = (uint64_t)handler;

  tasks[id].task_id = id;
  tasks[id].frame = frame;
  tasks[id].stack_base = stack_base;
  tasks[id].state = TASK_READY;
  tasks[id].core_id = 0xFF;

  total_tasks++;
}

task_t *schedule_task(trap_frame_t *frame) {
  if (total_tasks < 2)
    return &tasks[0];

  tasks[running_task_id].frame = frame;

  if (tasks[running_task_id].state == TASK_RUNNING) {
    tasks[running_task_id].state = TASK_READY;
  }

  // Search for next worker (Tasks 1 to total_tasks-1)
  for (uint32_t i = 1; i < total_tasks; i++) {
    // Increment first to move past the current task
    uint32_t next_id = (running_task_id + i) % total_tasks;

    // Skip Task 0 in the search loop
    if (next_id == 0)
      continue;

    if (tasks[next_id].state == TASK_READY) {
      running_task_id = next_id;
      tasks[running_task_id].state = TASK_RUNNING;
      tasks[running_task_id].core_id = get_core_id();
      return &tasks[running_task_id];
    }
  }

  // Fallback: No workers found, return Idle Task 0
  running_task_id = 0;
  tasks[0].state = TASK_RUNNING;
  tasks[0].core_id = get_core_id();
  return &tasks[0];
}
