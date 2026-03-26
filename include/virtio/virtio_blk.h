#ifndef __VIRTIO_BLK__
#define __VIRTIO_BLK__

#include <stdint.h>

#include "../mmu.h"

#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_ADDRESS 0x0A000000
#define VIRTIO_REG_MAGIC 0x00
#define VIRTIO_REG_VERSION 0x04
#define VIRTIO_REG_DEVICE_ID 0x08
#define VIRTIO_REG_PAGE_SIZE 0x28
#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM 0x38
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
#define VIRTIO_REG_QUEUE_NOTIFY 0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_DESC_F_NEXT 1
#define VIRTIO_DESC_F_WRITE 2
#define VIRTIO_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

// Result codes
#define VIRTIO_OK 0
#define VIRTIO_BAD_MAGIC 1
#define VIRTIO_UNKNOWN_DEVICE 2
#define VIRTIO_READ_ERROR 3

struct virtq_desc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
} __attribute__((packed));

struct virtq_avail {
  uint16_t flags;
  uint16_t index;
  uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

struct virtq_used_elem {
  uint32_t id;
  uint32_t len;
} __attribute__((packed));

struct virtq_used {
  uint16_t flags;
  uint16_t index;
  struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

struct virtio_virtq {
  struct virtq_desc desc[VIRTQ_ENTRY_NUM];
  struct virtq_avail avail;
  struct virtq_used used __attribute__((aligned(4096)));
  int queue_index;
  volatile uint16_t *used_index;
  uint16_t last_used_index;
} __attribute__((packed));

struct virtio_blk_req {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
  uint8_t data[512];
  uint8_t status;
} __attribute__((packed));

typedef struct {
  uint64_t base;
  struct virtio_virtq *vring;
  uint32_t actual_q_size;
  uint16_t last_used_idx;
} virtio_blk_dev_t;

// MMIO read/write helpers
static inline uint32_t virtio_read32(uint32_t offset) {
  return *((volatile uint32_t *)(PA_TO_VA(VIRTIO_BLK_ADDRESS) + offset));
}

static inline uint64_t virtio_read64(uint32_t offset) {
  return *((volatile uint64_t *)(PA_TO_VA(VIRTIO_BLK_ADDRESS) + offset));
}

static inline void virtio_write32(uint32_t offset, uint32_t value) {
  *((volatile uint32_t *)(PA_TO_VA(VIRTIO_BLK_ADDRESS) + offset)) = value;
}

static inline void virtio_write64(uint64_t offset, uint64_t value) {
  *((volatile uint64_t *)(PA_TO_VA(VIRTIO_BLK_ADDRESS) + offset)) = value;
}

int virtio_blk_init(virtio_blk_dev_t *dev);
int virtio_blk_read_sector(virtio_blk_dev_t *dev, uint64_t sector, void *buf);

#endif
