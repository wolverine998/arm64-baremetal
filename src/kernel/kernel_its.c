#include "../../include/gic-v3.h"
#include "../../include/gicv3-its.h"
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
  uint64_t rd_base = GET_GICR_BASE(core_id) + KERNEL_VIRT_BASE;

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

  lpi_prop_table[index] = priority | (enabled << 0);
}

void its_mapd(uint64_t device_id, uint64_t itt_addr, uint32_t size) {
  uint32_t index = cmd_offset / 8;

  // 1. Mandatory Zero-out of the 32-byte block
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  // DW0: [7:0] Opcode 0x08, [63:32] DeviceID
  command_queue[index + 0] = 0x08ULL | (device_id << 32);

  // DW1: [4:0] Size = IDbits - 1. (e.g. use 8 for 256 interrupts)
  command_queue[index + 1] = (uint64_t)((size - 1) & 0x1F);

  // DW2: ITT Address MUST start at Byte 17 (bit 8 of this DW).
  // This is the #1 reason for silent failure on QEMU.
  command_queue[index + 2] = (itt_addr) | (1ULL << 63);

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

  // DW0: Opcode 0x09
  command_queue[index + 0] = 0x09ULL;

  // DW2: [15:0] Collection ID
  //      [51:16] Target ID (Since PTA=0, this is just the core index)
  //      [63] Valid Bit
  command_queue[index + 2] = (uint64_t)(collection_id & 0xFFFF) |
                             ((uint64_t)target_index << 16) | (1ULL << 63);

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

  // DW0: Opcode 0x0A, DeviceID [63:32]
  command_queue[index + 0] = 0x0AULL | (device_id << 32);
  // DW1: EventID [31:0], pINTID [63:32] (Physical LPI ID)
  command_queue[index + 1] = (uint64_t)event_id | ((uint64_t)p_intid << 32);
  // DW2: Collection ID [15:0]
  command_queue[index + 2] = (uint64_t)(coll_id & 0xFFFF);

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
  // DW2: [51:16] Target ID (Since PTA=0)
  command_queue[index + 2] = ((uint64_t)target_index << 16);

  cmd_offset = (cmd_offset + 32) % 4096;
  asm volatile("dsb ish");
  write_gits_64(GITS_CWRITER, cmd_offset);
  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}

void its_int(uint64_t device_id, uint32_t event_id) {
  uint32_t index = cmd_offset / 8;

  // 1. Clear the 32-byte slot
  for (int i = 0; i < 4; i++)
    command_queue[index + i] = 0;

  // DW0: [7:0] Opcode 0x03, [63:32] DeviceID
  command_queue[index + 0] = 0x03ULL | (device_id << 32);

  // DW1: [31:0] EventID
  command_queue[index + 1] = (uint64_t)event_id;

  // Update offset
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

  // DW0: [7:0] Opcode 0x0C, [63:32] DeviceID
  command_queue[index + 0] = 0x0CULL | (device_id << 32);

  // DW1: [31:0] EventID
  command_queue[index + 1] = (uint64_t)event_id;

  cmd_offset = (cmd_offset + 32) % 4096;

  asm volatile("dsb ish");

  write_gits_64(GITS_CWRITER, cmd_offset);

  while (read_gits_64(GITS_CREADR) != cmd_offset)
    ;
}
