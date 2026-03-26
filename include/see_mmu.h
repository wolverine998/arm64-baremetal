#ifndef __SEE_MMU__
#define __SEE_MMU__

#include <stdint.h>

#define MAX_TABLES 64

uint64_t *see_alloc_table();
void see_map_page(uint64_t *l1, uint64_t va, uint64_t pa, uint64_t flags);
void see_map_region(uint64_t *l1, uint64_t va_start, uint64_t pa_start,
                    uint64_t size, uint64_t flags);

#endif
