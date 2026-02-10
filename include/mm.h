#ifndef __MM__
#define __MM__

#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 131072

void *mm_allocate_pages(uint32_t num_pages);
void mm_free_pages(void *ptr, uint32_t num_pages);

extern uint8_t pages[MAX_PAGES];

#endif
