#include "../../include/kernel_mmu.h"
#include "../../include/stdlib.h"
#include "../../include/virtio/virtio_blk.h"

static struct virtio_virtq blk_ring;

int virtio_blk_init(virtio_blk_dev_t *dev) {
  if (virtio_read32(VIRTIO_REG_MAGIC) != 0x74726976)
    return VIRTIO_BAD_MAGIC;
  if (virtio_read32(VIRTIO_REG_DEVICE_ID) != VIRTIO_DEVICE_BLK)
    return VIRTIO_UNKNOWN_DEVICE;

  virtio_write32(VIRTIO_REG_DEVICE_STATUS, 0);
  virtio_write32(VIRTIO_REG_DEVICE_STATUS,
                 VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER);

  // set page size
  virtio_write32(VIRTIO_REG_PAGE_SIZE, 4096);

  // Configure queue
  virtio_write32(VIRTIO_REG_QUEUE_SEL, 0);
  uint32_t q_max = virtio_read32(VIRTIO_REG_QUEUE_NUM_MAX);

  dev->actual_q_size = (q_max < VIRTQ_ENTRY_NUM) ? q_max : VIRTQ_ENTRY_NUM;
  virtio_write32(VIRTIO_REG_QUEUE_NUM, dev->actual_q_size);

  uint64_t ring_pa = VA_TO_PA(&blk_ring);
  virtio_write32(VIRTIO_REG_QUEUE_PFN, (uint32_t)(ring_pa >> 12));

  // Set driver ok
  uint32_t status = virtio_read32(VIRTIO_REG_DEVICE_STATUS);
  virtio_write32(VIRTIO_REG_DEVICE_STATUS, status | VIRTIO_STATUS_DRIVER_OK);

  dev->base = PA_TO_VA(VIRTIO_BLK_ADDRESS);
  dev->vring = &blk_ring;
  dev->last_used_idx = 0;

  return VIRTIO_OK;
}

int virtio_blk_read_sector(virtio_blk_dev_t *dev, uint64_t sector, void *buf) {
  static struct virtio_blk_req req;

  req.type = VIRTIO_BLK_T_IN;
  req.sector = sector;
  req.status = 0xFF;

  // Request header
  dev->vring->desc[0].addr = VA_TO_PA(&req);
  dev->vring->desc[0].len = 16;
  dev->vring->desc[0].flags = VIRTIO_DESC_F_NEXT;
  dev->vring->desc[0].next = 1;

  // Data + Status
  dev->vring->desc[1].addr = VA_TO_PA(&req.data);
  dev->vring->desc[1].len = SECTOR_SIZE + 1;
  dev->vring->desc[1].flags = VIRTIO_DESC_F_WRITE;
  dev->vring->desc[1].next = 0;

  uint16_t avail_idx = dev->vring->avail.index % dev->actual_q_size;
  dev->vring->avail.ring[avail_idx] = 0;

  asm volatile("dsb ishst" ::: "memory");
  dev->vring->avail.index++;
  asm volatile("dsb ishst" ::: "memory");

  virtio_write32(VIRTIO_REG_QUEUE_NOTIFY, 0);

  while (dev->vring->used.index == dev->last_used_idx) {
    asm volatile("wfe");
  }

  mem_copy(buf, req.data, SECTOR_SIZE);
  dev->last_used_idx = dev->vring->used.index;

  return (req.status == 0) ? VIRTIO_OK : VIRTIO_READ_ERROR;
}
