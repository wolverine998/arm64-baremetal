#include "../../include/kernel_mmu.h"
#include "../../include/kernel_stdlib.h"
#include "../../include/mm.h"
#include "../../include/mmu.h"

uint8_t pages[MAX_PAGES];
extern char _heap_begin[];

#include "../../include/kernel_mmu.h"
#include "../../include/kernel_stdlib.h"
#include "../../include/mm.h"
#include "../../include/mmu.h"
#include "../../include/uart.h"

uint8_t pages[MAX_PAGES];
extern char _heap_begin[];

// New function to allocate multiple contiguous pages
void *mm_allocate_pages(uint32_t num_pages) {
  if (num_pages == 0)
    return 0;

  // Search through the bit-array for a contiguous block of zeros
  for (int i = 0; i <= MAX_PAGES - num_pages; i++) {
    int found = 1;
    for (int j = 0; j < num_pages; j++) {
      if (pages[i + j] != 0) {
        found = 0;
        break;
      }
    }

    if (found) {
      // Mark the block as allocated
      for (int j = 0; j < num_pages; j++) {
        pages[i + j] = 1;
      }

      // Calculate the starting address based on the index
      uint64_t ptr = ((uint64_t)_heap_begin + (i * PAGE_SIZE));
      uint64_t size = num_pages * PAGE_SIZE;

      // Map the entire range into the kernel page table
      // This ensures Virtual Address == Physical Address (Direct Mapping)
      map_region_virtual(ptr, VA_TO_PA(ptr), size,
                         PROT_NORMAL_MEM | AP_EL0_NO_ELX_RW | PTE_PXN |
                             PTE_UXN);

      return (void *)ptr;
    }
  }

  kernel_printf("[MM] ERROR: No contiguous block of %d pages found!\n",
                num_pages);
  return 0;
}

void mm_free_pages(void *ptr, uint32_t num_pages) {
  if (!ptr)
    return;

  uint64_t index = ((uint64_t)ptr - (uint64_t)_heap_begin) / PAGE_SIZE;
  uint64_t size = num_pages * PAGE_SIZE;

  // Clear memory and update bit-array
  mem_zero(ptr, size);
  for (uint32_t i = 0; i < num_pages; i++) {
    pages[index + i] = 0;
  }

  // Unmap from page tables and flush TLB
  unmap_region_virtual(kernel_l1, (uint64_t)ptr, size);
  for (uint32_t i = 0; i < num_pages; i++) {
    tlb_flush_page_el1((uint64_t)ptr + (i * PAGE_SIZE));
  }
}
