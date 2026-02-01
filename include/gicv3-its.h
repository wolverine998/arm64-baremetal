#ifndef __GICV3_ITS__
#define __GICV3_ITS__

#include <stdint.h>

#define PROPBASER_IDBITS(n) (n << 0)
#define PROPBASER_INNER_CACHE(n) (n << 7)
#define PROPBASER_OUTER_CACHE(n) (n << 56)
#define PROPBASER_SHAREABILITY(n) (n << 10)
#define PENDBASER_PTZ (1ULL << 62)

#define BASER_SIZE(n) (((n - 1) << 0) & 0xFF)
#define BASER_PAGE_SIZE(n) ((n << 8) & 0x3)
#define BASER_INNER_CACHE(n) (n << 59)
#define BASER_OUTER_CACHE(n) (n << 53)
#define BASER_SHAREABILITY(n) (n << 10)
#define BASER_ENTRY_SIZE(n) ((n >> 48) & 0x1F)
#define BASER_TYPE(n) ((n >> 56) & 0x7)
#define BASER_VALID (1ULL << 63)

#define INNER_DEVICE_NGNRNE (0x0ULL)
#define INNER_NORMAL_NON_CACHEABLE (0x1ULL)
#define INNER_NORMAL_CACHEABLE_RAWT (0x2ULL)
#define INNER_NORMAL_CACHEABLE_RAWB (0x3ULL)
#define INNER_NORMAL_CACHEABLE_WAWT (0x4ULL)
#define INNER_NORMAL_CACHEABLE_WAWB (0x5ULL)
#define INNER_NORMAL_CACHEABLE_RAWAWT (0x6ULL)
#define INNER_NORMAL_CACHEABLE_RAWAWB (0x7ULL)

#define SH_NONE (0x0ULL)
#define SH_INNER (0x1ULL)
#define SH_OUTER (0x2ULL)

#define BASER_PAGESZ_4KB (0x0ULL)
#define BASER_PAGESZ_16KB (0x1ULL)
#define BASER_PAGESZ_64KB (0x2ULL)

#define LPI_ENABLE (1 << 0)
#define LPI_PRIORITY(n) (n << 2)

// GITS_CBASER bits
#define GITS_CBASER_SIZE(n) ((n - 1) << 0)
#define GITS_CBASER_SHAREABILITY(n) (n << 10)
#define GITS_CBASER_OUTER_CACHE(n) (n << 53)
#define GITS_CBASER_INNER_CACHE(n) (n << 59)
#define GITS_CBASER_VALID (1ULL << 63)

// GITS_BASER
#define GITS_BASER_SIZE(n) (n << 0)
#define GITS_BASER_PAGESIZE(n) (n << 8)
#define GITS_BASER_SHAREABILITY(n) (n << 10)
#define GITS_BASER_ENTRY_SIZE(n) ((n >> 48) & 0x1F)
#define GITS_BASER_OUTER_CACHE(n) (n << 53)
#define GITS_BASER_TYPE(n) ((n >> 56) & 0x7)
#define GITS_BASER_INNER_CACHE(n) (n << 59)
#define GITS_BASER_VALID (1ULL << 63)

// GITS_CWRITER bits
#define GITS_CWRITER_RETRY (1 << 0)
#define GITS_CWRITER_OFFSET(n) (n << 5)

// GITS_CREADR
#define GITS_CREADR_STALLED(n) (n & 0x1)
#define GITS_CREADR_OFFSET(n) ((n >> 5) & 0x7FFF)

extern uint32_t cmd_offset;

void gic_redistributor_init_lpi();
void gic_its_enable();
void gic_its_disable();
void gic_its_prepare();
void gic_its_configure_lpi(uint32_t interrupt_id, uint8_t priority,
                           uint8_t enabled);
void its_mapd(uint64_t device_id, uint64_t itt_addr, uint32_t size);
void its_mapc(uint32_t collection_id, uint32_t rd_base);
void its_mapti(uint64_t device_id, uint32_t event_id, uint32_t interrupt_id,
               uint32_t collection_id);
void its_sync(uint32_t rd_base);
void its_int(uint64_t device_id, uint32_t event_id);
void its_inv(uint64_t device_id, uint32_t event_id);

#endif
