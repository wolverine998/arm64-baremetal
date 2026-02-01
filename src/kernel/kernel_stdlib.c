#include "../../include/kernel_stdlib.h"

void *mem_copy(void *dest, const void *src, size_t size) {
  unsigned char *d = (unsigned char *)dest;
  const unsigned char *s = (const unsigned char *)src;

  while (size > 0) {
    *d = *s;
    s++;
    d++;
    size--;
  }

  return dest;
}

void mem_zero(void *ptr, size_t size) {
  unsigned char *d = (unsigned char *)ptr;

  while (size > 0) {
    *d = 0;
    d++;
    size--;
  }
}
