#ifndef __STDLIB_IMPL__
#define __STDLIB_IMPL__

#include <stddef.h>

void *mem_copy(void *dest, const void *src, size_t size);
void mem_zero(void *ptr, size_t size);

#endif
