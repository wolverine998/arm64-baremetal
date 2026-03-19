#ifndef __VIRTIO_BLK__
#define __VIRTIO_BLK__

#include <stdint.h>

// Legacy MMIO Offsets
#define VIRTIO_REG_MAGIC 0x000
#define VIRTIO_REG_VERSION 0x004
#define VIRTIO_REG_DEVICE_ID 0x008
#define VIRTIO_REG_QUEUE_SEL 0x030
#define VIRTIO_REG_QUEUE_NUM_MAX 0x034
#define VIRTIO_REG_QUEUE_NUM 0x038
#define VIRTIO_REG_QUEUE_ALIGN 0x03c
#define VIRTIO_REG_QUEUE_PFN 0x040
#define VIRTIO_REG_QUEUE_NOTIFY 0x050
#define VIRTIO_REG_STATUS 0x070

#define MAX_QUEUE_SIZE 128
#define PAGE_SIZE 4096

struct virtq_desc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
};

struct virtq_avail {
  uint16_t flags;
  uint16_t idx;
  uint16_t ring[MAX_QUEUE_SIZE];
};

struct virtq_used_item {
  uint32_t id;
  uint32_t len;
};

struct virtq_used {
  uint16_t flags;
  uint16_t idx;
  struct virtq_used_item ring[MAX_QUEUE_SIZE];
};

typedef struct {
  struct virtq_desc desc[MAX_QUEUE_SIZE];
  struct virtq_avail avail;
  struct virtq_used used __attribute__((aligned(4096)));
} virtq_t __attribute__((aligned(4096)));

typedef struct {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
} virtio_blk_req_t;

typedef struct {
  uintptr_t base;
  virtq_t *vring;
  uint32_t actual_q_size;
  uint16_t last_used_idx;
} virtio_blk_dev_t;

#endif
