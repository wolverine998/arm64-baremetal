#include "../../include/gic-v3.h"
#include "../../include/gicv3-its.h"
#include "../../include/kernel_stdlib.h"
#include "../../include/mmu.h"
#include "../../include/uart.h"
#include <stdint.h>

uint32_t cmd_offset = 0;

// GIC Redistributor tables
__attribute__((aligned(4096))) uint8_t lpi_prop_table[16384];
__attribute__((aligned(65536))) uint8_t pend_tables[2][65536];

// GIC ITS tables
__attribute__((aligned(4096))) uint8_t device_table[4096];
__attribute__((aligned(4096))) uint8_t collection_table[4096];
__attribute__((aligned(65536))) uint64_t command_queue[512];

void gic_redistributor_init_lpi() {
  uint32_t core_id = get_core_id();
  uint64_t rd_base = PA_TO_VA(GET_GICR_BASE(core_id));

  uint64_t prop_pa = (uint64_t)VA_TO_PA(lpi_prop_table);
  uint64_t pend_pa = (uint64_t)VA_TO_PA(pend_tables[core_id]);

  uint64_t flags = PROPBASER_IDBITS(13) | PROPBASER_SHAREABILITY(SH_INNER) |
                   PROPBASER_INNER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB) |
                   PROPBASER_OUTER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB);

  asm volatile("dsb ish");

  write_gicr_64(rd_base, GICR_PROPBASER, prop_pa | flags);
  write_gicr_64(rd_base, GICR_PENDBASER, pend_pa | flags | PENDBASER_PTZ);

  // 5. Enable LPIs in the Redistributor
  uint32_t ctlr = read_gicr(rd_base, GICR_CTLR);

  write_gicr(rd_base, GICR_CTLR, ctlr | GICR_CTLR_ENLPIS);
}

void gic_its_enable() {
  if (read_gits_32(GITS_CTLR) & GITS_CTLR_ENABLED)
    return;

  // pool for quiescent
  // if its 1, we are allowed to disable/enable the ITS

  while (!(read_gits_32(GITS_CTLR) & GITS_CTLR_QUIESCENT))
    ;

  uint32_t ctlr = read_gits_32(GITS_CTLR);
  write_gits_32(GITS_CTLR, ctlr | GITS_CTLR_ENABLED);
  kernel_printf("[GIC]: ITS enabled\n");
}

void gic_its_disable() {
  if (!(read_gits_32(GITS_CTLR) & GITS_CTLR_ENABLED))
    return;

  // pool for quiescent
  // if its 1, we are allowed to disable/enable the ITS

  while (!(read_gits_32(GITS_CTLR) & GITS_CTLR_QUIESCENT))
    ;

  uint32_t ctlr = read_gits_32(GITS_CTLR);
  write_gits_32(GITS_CTLR, ctlr & ~(GITS_CTLR_ENABLED));
  kernel_printf("[GIC]: ITS disabled\n");
}

void gic_its_prepare() {
  uint64_t flags = BASER_SIZE(1) | BASER_PAGE_SIZE(BASER_PAGESZ_4KB) |
                   BASER_SHAREABILITY(SH_INNER) |
                   BASER_OUTER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB) |
                   BASER_INNER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB) |
                   BASER_VALID;
  uint64_t cmd_flags = GITS_CBASER_SIZE(1) |
                       GITS_CBASER_SHAREABILITY(SH_INNER) |
                       GITS_CBASER_OUTER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB) |
                       GITS_CBASER_INNER_CACHE(INNER_NORMAL_CACHEABLE_RAWAWB) |
                       GITS_CBASER_VALID;

  // write the tables
  uint64_t dev_baser = (uint64_t)VA_TO_PA(device_table);
  uint64_t collection_baser = (uint64_t)VA_TO_PA(collection_table);
  uint64_t cmd_baser = (uint64_t)VA_TO_PA(command_queue);

  asm volatile("dsb ish");

  write_gits_64(GITS_BASER0, dev_baser | flags);
  write_gits_64(GITS_BASER1, collection_baser | flags);
  write_gits_64(GITS_CBASER, cmd_baser | cmd_flags);
  write_gits_64(GITS_CWRITER, 0);
}

void gic_its_configure_lpi(uint32_t interrupt_id, uint8_t priority,
                           uint8_t enabled) {
  if (interrupt_id < 8192)
    return;

  uint32_t index = interrupt_id - 8192;
  uint8_t en = 0;

  if (enabled)
    en |= (1 << 0);
  else
    en &= ~(1 << 0);

  lpi_prop_table[index] = priority | en;
}

void its_mapd(uint64_t device_id, uint64_t itt_addr, uint32_t size,
              uint32_t valid) {
  uint32_t index = cmd_offset / 8;

  // 1. Mandatory Zero-out of the 32-byte block
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  uint64_t V = 0;
  if (valid)
    V = (1ULL << 63);
  else
    V &= ~(1ULL << 63);

  command_queue[index + 0] = 0x08ULL | (device_id << 32);
  command_queue[index + 1] = ((size - 1));
  command_queue[index + 2] = (itt_addr) | (V);

  cmd_offset = (cmd_offset + 32) % 4096;

  // 2. CRITICAL: Memory Barrier
  // Ensures the command is in RAM BEFORE we update the GIC's CWRITER
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_mapc(uint32_t collection_id, uint32_t target_index) {
  uint32_t index = cmd_offset / 8;
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x09ULL;

  command_queue[index + 2] =
      (collection_id & 0xFFFF) | (target_index << 16) | (1ULL << 63);

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");
  write_gits_64(GITS_CWRITER, cmd_offset);
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_mapti(uint64_t device_id, uint32_t event_id, uint32_t p_intid,
               uint32_t coll_id) {
  uint32_t index = cmd_offset / 8;
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x0AULL | (device_id << 32);
  command_queue[index + 1] = event_id | ((uint64_t)p_intid << 32);
  command_queue[index + 2] = (coll_id & 0xFFFF);

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");
  write_gits_64(GITS_CWRITER, cmd_offset);
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_sync(uint32_t target_index) {
  uint32_t index = cmd_offset / 8;
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x05ULL;
  command_queue[index + 2] = (target_index << 16);

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");
  write_gits_64(GITS_CWRITER, cmd_offset);
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_int(uint64_t device_id, uint32_t event_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x03ULL | (device_id << 32);
  command_queue[index + 1] = event_id;

  cmd_offset = (cmd_offset + 32) % 4096;

  // 2. The Barrier: Ensure the command is in RAM before the GIC sees CWRITER
  // update
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  // 3. Wait for the ITS to consume the command
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_inv(uint64_t device_id, uint32_t event_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x0CULL | (device_id << 32);

  command_queue[index + 1] = event_id;

  cmd_offset = (cmd_offset + 32) % 4096;

  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_discard(uint64_t device_id, uint32_t event_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x0FULL | (device_id << 32);
  command_queue[index + 1] = event_id;

  cmd_offset = (cmd_offset + 32) % 4096;

  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_clear(uint64_t device_id, uint32_t event_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x04ULL | (device_id << 32);
  command_queue[index + 1] = event_id;

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_invall(uint32_t collection_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  command_queue[index + 0] = 0x0DULL;
  command_queue[index + 2] = collection_id;

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_movall(uint32_t rd_base, uint32_t rd_target) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    mem_zero(&command_queue[index + i], sizeof(command_queue));

  command_queue[index + 0] = 0x0EULL;
  command_queue[index + 2] = (rd_base << 16);
  command_queue[index + 3] = (rd_target << 16);

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_movi(uint64_t device_id, uint32_t event_id, uint32_t collection_id) {
  uint32_t index = cmd_offset / 8;

  for (int i = 0; i < 4; i++)
    mem_zero(&command_queue[index + i], sizeof(command_queue));

  command_queue[index + 0] = 0x01ULL | (device_id << 32);
  command_queue[index + 1] = event_id;
  command_queue[index + 2] = collection_id;

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}
