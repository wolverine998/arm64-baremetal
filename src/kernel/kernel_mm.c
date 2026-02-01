#include "../../include/kernel_mmu.h"
#include "../../include/kernel_stdlib.h"
#include "../../include/mm.h"
#include "../../include/mmu.h"

uint8_t pages[MAX_PAGES];
extern char _heap_begin[];

void *mm_allocate_page() {
  for (int i = 0; i < MAX_PAGES; i++) {
    if (pages[i] == 0) {
      pages[i] = 1;
      uint64_t ptr = ((uint64_t)_heap_begin + (i * PAGE_SIZE));
      map_page_virtual(ptr, VA_TO_PA(ptr),
                       PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW | PTE_PXN | PTE_UXN);
      return (void *)ptr;
    }
  }
  return 0;
}

void mm_free_page(void *ptr) {
  mem_zero(ptr, PAGE_SIZE);
  uint64_t index = ((uint64_t)ptr - (uint64_t)_heap_begin) / PAGE_SIZE;
  pages[index] = 0;
  unmap_region(kernel_l1, (uint64_t)ptr, 1);
  tlb_flush_page_el1((uint64_t)ptr);
}
