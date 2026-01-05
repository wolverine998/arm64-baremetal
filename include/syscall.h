#ifndef __SYSCALL__

#define __SYSCALL__

#include <stdint.h>
#define SVC_CONSOLE_PRINT 1

void svc_print(const char *s);

#endif
