#include "../../include/syscall.h"

void svc_print(const char *s) {
  register const char *x0 asm("x0") = s;
  register uint64_t x8 asm("x8") = SVC_CONSOLE_PRINT;

  asm volatile("svc #0" : : "r"(x0), "r"(x8) : "memory");
}
