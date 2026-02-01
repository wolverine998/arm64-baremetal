#ifndef __MM__
#define __MM__

#include <stdint.h>

#define PAGE_SIZE 4096
#define MAX_PAGES 131072

void *mm_allocate_page();
void mm_free_page(void *ptr);

extern uint8_t pages[MAX_PAGES];

#endif
