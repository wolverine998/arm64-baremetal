#include "../../include/cpu_state.h"
#include "../../include/gic-v3.h"
#include "../../include/gicv3-its.h"
#include "../../include/irq.h"
#include "../../include/kernel_gicv3.h"
#include "../../include/kernel_mmu.h"
#include "../../include/mmu.h"
#include "../../include/registers.h"
#include "../../include/smc.h"
#include "../../include/uart.h"
#include <stdint.h>

extern void _kernel_entry();
extern char _kernel_start[], _kernel_end[], _kernel_stack[];
extern void app_entry();
extern char _app_start[], _app_end[], _app_stack[];

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

void c_entry() {
  k_cpus[0] = ON;
  kernel_puts("Kernel booted successfully\n");
  initialize_memory_info();
  gic_its_configure_lpi(9000, 0x80, 1);
  gic_its_configure_lpi(9100, 0x90, 1);
  gic_enable_sre_el1();
  gic_el1_init_spi();
  gic_redistributor_init_lpi();
  gic_its_prepare();
  gic_its_enable();
  cpu_enable_group1_interrupts();
  cpu_set_priority_mask(255);
  mask_interrupts(0);

  smc_res_t res;
  smc_call(SEEOS_VERSION, &res, 0, 0, 0, 0, 0);
  smc_call(PSCI_CPU_ON, &res, 1, VA_TO_PA((uint64_t)_kernel_entry), 0, 0, 0);

  uint64_t *l1 = (uint64_t *)user_l1;
  uint64_t k_start = (uint64_t)_kernel_start;
  uint64_t k_end = (uint64_t)_kernel_stack;
  uint64_t start, end;
  asm volatile("ldr %0, =app_entry" : "=r"(start));
  asm volatile("ldr %0, =_app_stack" : "=r"(end));

  while (k_cpus[1] != ON) {
    wait_for_event();
  }

  unmap_region(l1, k_start, k_end - k_start);
  map_region(l1, start, start, end - start,
             PROT_NORMAL_MEM | AP_EL0_RW_ELX_RW | PTE_PXN);

  uint64_t spsr = SPSR_M_EL0;
  write_sysreg(spsr_el1, spsr);
  write_sysreg(sp_el0, end);
  write_sysreg(elr_el1, start);

  asm volatile("isb; eret");
}

__attribute__((aligned(4096))) uint8_t dev0[192];

void sec_entry() {
  gic_enable_sre_el1();
  gic_redistributor_init_lpi();
  cpu_enable_group1_interrupts();
  uint32_t core_id = get_core_id();
  cpu_set_priority_mask(255);
  mask_interrupts(0);

  kernel_printf("Core %d booted successfully\n", core_id);

  k_cpus[core_id] = ON;

  // try trigger LPI ID 9000
  its_mapc(0x1, 0x0);
  its_mapd(0x2, (uint64_t)VA_TO_PA(dev0), 4);
  its_mapti(0x2, 0x2, 9000, 0x1);
  its_sync(0x0);
  its_int(0x2, 0x2);
}
