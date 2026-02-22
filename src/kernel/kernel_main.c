#include "../../include/cpu_state.h"
#include "../../include/generic_timer.h"
#include "../../include/gic-v3.h"
#include "../../include/irq.h"
#include "../../include/kernel_mmu.h"
#include "../../include/kernel_sched.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern char _kernel_start[], _kernel_end[], _kernel_stack[];
extern char _app_start[], _app_end[], _app_stack[];
extern void app_entry();

volatile power_state_t k_cpus[MAX_CPUS] = {0};
meminfo_t meminfo = {0, 0, 0};

void initialize_memory_info(void) {
  uint64_t kernel_end_pa = VA_TO_PA((uint64_t)_kernel_end);

  uint64_t used_bytes = kernel_end_pa - RAM_BASE;
  uint64_t free_bytes = RAM_END - kernel_end_pa;

  meminfo.total_ram = RAM_SIZE_BYTES / (1024 * 1024);
  meminfo.used_ram = used_bytes / (1024 * 1024);
  meminfo.free_ram = free_bytes / (1024 * 1024);
  kernel_printf("[KERNEL] Memory initalized.\n");
  kernel_printf("[RAM] Total: %dMB Used: %dMB\n", meminfo.total_ram,
                meminfo.used_ram);
  kernel_printf("[RAM] Free: %dMB\n", meminfo.free_ram);
}

void test_stack_execution() {
  // 0xd65f03c0 is the opcode for 'ret' in ARM64
  uint32_t code = 0xd65f03c0;

  // Create a function pointer pointing to your local (stack) variable
  void (*stack_code)(void) = (void (*)(void))&code;

  kernel_puts("[TEST] Attempting to execute from stack...\n");

  // This should trigger a Synchronous Exception if PXN is working
  stack_code();

  kernel_puts("[ERROR] Stack execution allowed! Security check failed.\n");
}

void task1() {
  task_t *task = get_current_task();
  kernel_printf("Task %d finished\n", task->task_id);
}

void task2() {
  task_t *task = get_current_task();
  kernel_printf("Task %d finished\n", task->task_id);
}

void c_entry() {
  k_cpus[0] = ON;
  kernel_puts("Kernel booted successfully\n");
  initialize_memory_info();
  gic_enable_sre_el1();
  gic_el1_init_spi();
  cpu_enable_group1_interrupts();
  cpu_set_priority_mask(255);

  init_sched();
  sched_enable();

  mask_interrupts(0);

  smc_res_t res;
  smc_call(SEEOS_VERSION, &res, 0, 0, 0, 0, 0);
  smc_call(PSCI_CPU_ON, &res, 1, VA_TO_PA(_kernel_entry), 0, 0, 0);

  create_task(task1);
  create_task(task2);

  uint64_t *l1 = (uint64_t *)user_l1;
  uint64_t k_start = (uint64_t)_kernel_start;
  uint64_t k_end = (uint64_t)_kernel_stack;
  uint64_t start, end;
  asm volatile("ldr %0, =app_entry" : "=r"(start));
  asm volatile("ldr %0, =_app_stack" : "=r"(end));

  while (k_cpus[1] != ON) {
    wait_for_event();
  }

  unmap_region_virtual(l1, k_start, k_end - k_start);
  tlb_flush_all_e1();
  map_region_virtual(l1, start, start, end - start,
                     PROT_NORMAL_MEM | AP_EL0_RW_ELX_RW | PTE_PXN);

  uint64_t spsr = SPSR_M_EL0;
  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, end);
  write_sysreg(elr_el1, start);

  asm volatile("isb; eret");
}

void sec_entry() {
  gic_enable_sre_el1();
  cpu_enable_group1_interrupts();
  uint32_t core_id = get_core_id();
  cpu_set_priority_mask(255);

  mask_interrupts(0);

  kernel_printf("Core %d booted successfully\n", core_id);

  k_cpus[core_id] = ON;

  while (1) {
    wait_for_interrupt();
  }
}
