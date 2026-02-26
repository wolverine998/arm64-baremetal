#ifndef __SEC_CRYPTO__
#define __SEC_CRYPTO__

#include <stdint.h>

typedef struct {
  uint64_t wrapped_key1;
  uint64_t wrapped_key2;
} wrapped_key_t;

void see_init_crypto(const uint8_t *master_key);
wrapped_key_t see_wrap_key(uint64_t plain_key);
uint64_t see_unwrap_key(wrapped_key_t wrapped_key);

#endif
