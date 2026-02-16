#ifndef __SPINLOCK__
#define __SPINLOCK__

#include <stdint.h>

void spin_lock(uint32_t *lock);
void spin_unlock(uint32_t *lock);

#endif
