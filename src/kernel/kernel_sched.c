#include "../../include/kernel_sched.h"
#include "../../include/generic_timer.h"
#include "../../include/kernel_gicv3.h"
#include "../../include/kernel_spinlock.h"
#include "../../include/kernel_stdlib.h"
#include "../../include/mm.h"
#include "../../include/uart.h"

static queue_t ready_q = {ZERO, ZERO};
static queue_t zombie_q = {ZERO, ZERO};

static task_t task_pool[MAX_TASKS];
static task_t idle_tasks[2];
static uint32_t next_task_id = 0;
static uint32_t sched_lock = 0;

void task_enqueue(queue_t *q, task_t *t) {
  t->next = ZERO;
  if (q->tail == ZERO) {
    q->head = q->tail = t;
  } else {
    q->tail->next = t;
    q->tail = t;
  }
}

task_t *task_dequeue(queue_t *q) {
  if (q->head == ZERO)
    return ZERO;
  task_t *t = q->head;
  q->head = q->head->next;
  if (q->head == ZERO)
    q->tail = ZERO;
  t->next = ZERO;
  return t;
}

void reaper_task() {
  while (1) {
    spin_lock(&sched_lock);
    task_t *z = task_dequeue(&zombie_q);

    if (z == ZERO) {
      spin_unlock(&sched_lock);
      wait_for_interrupt();
      continue;
    }

    // We found a zombie!
    if (z->core_id == 0xFF) {
      kernel_printf("[REAPER] Freeing task %d\n", z->task_id);
      mm_free_pages((void *)z->stack_base, TASK_STACK_SIZE / PAGE_SIZE);
      mem_zero(z, sizeof(task_t));
      spin_unlock(&sched_lock);

      // We did work, now we can yield to let others run
      yield();
    } else {
      // Core is still returning from this task, put it back
      task_enqueue(&zombie_q, z);
      spin_unlock(&sched_lock);

      // Mandatory yield so the other core has time to finish the switch
      yield();
    }
  }
}

void init_sched() {
  mem_zero(task_pool, sizeof(task_pool));
  mem_zero(idle_tasks, sizeof(idle_tasks));

  uint32_t core_id = get_core_id();
  idle_tasks[core_id].task_id = 0;
  idle_tasks[core_id].state = TASK_RUNNING;
  idle_tasks[core_id].core_id = core_id;
  set_current_task(&idle_tasks[core_id]);
}

void init_sched_core() {
  uint32_t core_id = get_core_id();
  idle_tasks[core_id].task_id = 0;
  idle_tasks[core_id].state = TASK_RUNNING;
  idle_tasks[core_id].core_id = core_id;
  set_current_task(&idle_tasks[core_id]);
}

void create_task(void (*entry)(void)) {
  spin_lock(&sched_lock);

  task_t *t = ZERO;
  for (int i = 1; i < MAX_TASKS; i++) {
    if (task_pool[i].state == TASK_EMPTY) {
      t = &task_pool[i];
      break;
    }
  }

  if (!t) {
    kernel_printf("[SCHED] Error. Maximum tasks reached\n");
    spin_unlock(&sched_lock);
    return;
  }

  uint64_t stack = (uint64_t)mm_allocate_pages(TASK_STACK_SIZE / PAGE_SIZE);
  mem_zero((void *)stack, TASK_STACK_SIZE);

  t->task_id = ++next_task_id;
  t->stack_base = stack;
  t->state = TASK_READY;
  t->core_id = 0xFF;

  t->frame = (trap_frame_t *)(stack + TASK_STACK_SIZE - sizeof(trap_frame_t));
  t->frame->elr = (uint64_t)entry;
  t->frame->regs[30] = (uint64_t)task_exit;
  t->frame->spsr = SPSR_M_EL1H;

  task_enqueue(&ready_q, t);

  kernel_printf("[SCHED] Created task %d\n", t->task_id);
  spin_unlock(&sched_lock);
}

void task_exit() {
  mask_interrupts(1);
  spin_lock(&sched_lock);

  task_t *current = get_current_task();
  current->state = TASK_ZOMBIE;

  spin_unlock(&sched_lock);
  mask_interrupts(0);

  yield();

  while (1) {
    wait_for_interrupt();
  }
}

task_t *schedule_task(trap_frame_t *frame) {
  spin_lock(&sched_lock);

  task_t *current = get_current_task();
  if (current == ZERO) {
    kernel_printf("No tasks to schedule\n");
    spin_unlock(&sched_lock);
    return ZERO;
  }

  current->frame = frame;

  uint32_t core_id = get_core_id();
  task_t *idle = &idle_tasks[core_id];

  if (current->state == TASK_ZOMBIE) {
    task_enqueue(&zombie_q, current);
  } else if (current != idle) {
    current->state = TASK_READY;
    task_enqueue(&ready_q, current);
  }

  task_t *next = task_dequeue(&ready_q);

  if (!next) {
    next = idle;
  }

  if (current != next) {
    current->core_id = 0xFF;
  }

  if (current->state == TASK_ZOMBIE)
    send_sgi1_to_core(core_id, SCHEDULER_GC);

  next->state = TASK_RUNNING;
  next->core_id = core_id;
  set_current_task(next);

  spin_unlock(&sched_lock);
  return next;
}

void sched_enable() {
  /* Setup PPI and SGI
   * PPI ID 30 - Timer
   * SGI ID 8 - Yield
   * EL3 reserved us SGI's in range 8-15
   */
  gic_conf_sgi(SCHEDULER_YIELD, 0x20, 1);
  gic_conf_sgi(SCHEDULER_GC, 0x10, 1);
  gic_conf_ppi(CNTP_INTID, 0x0, 1);

  /* Enable the timer
   * 1 second interval for testing purpose
   * Later, we will change it to a reasonable value
   * for the scheduler
   */
  timer_countdown(1000);
  timer_enable_interrupts();
  timer_enable();
}

void sched_disable() {
  /* Disable the PPI/SGI interupts
   * Finally, disable the timer
   */
  gic_disable_sgi(SCHEDULER_YIELD);
  gic_disable_sgi(SCHEDULER_GC);
  gic_disable_ppi(CNTP_INTID);

  timer_disable_interrupts();
  timer_disable();
}

void reaper_service() {
  // make sure we dont
  // stall the cpu too much
  spin_lock(&sched_lock);

  task_t *z = task_dequeue(&zombie_q);

  while (z != ZERO && z->core_id == 0xFF) {
    kernel_printf("[REAPER] Cleaning task %d\n", z->task_id);
    mm_free_pages((void *)z->stack_base, TASK_STACK_SIZE / PAGE_SIZE);
    mem_zero(z, sizeof(task_t));
    z = task_dequeue(&zombie_q);
  }

  spin_unlock(&sched_lock);
}
