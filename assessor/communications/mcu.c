#include "com_mcu.h"
#include <string.h>
static int address32(uint16_t a) {
  return a <= 0x2ec || (a >= 0xe00 && a <= 0xe14) ||
         (a >= 0x400 && a <= 0xbfc && !(a & 3));
}
int mcp_read32(const McpBus *b, uint16_t address, uint32_t *value) {
  if (!b || !b->transfer || !value || !address32(address))
    return -1;
  uint8_t tx[6] =
      {(uint8_t)(0x30 | (address >> 8)), (uint8_t)address, 0, 0, 0, 0},
          rx[6] = {0};
  if (b->transfer(b->ctx, tx, rx, sizeof tx))
    return -1;
  *value = (uint32_t)rx[2] | (uint32_t)rx[3] << 8 | (uint32_t)rx[4] << 16 |
           (uint32_t)rx[5] << 24;
  return 0;
}
int mcp_write32(const McpBus *b, uint16_t address, uint32_t value) {
  /* IOCON requires single-byte writes; exclude that register from this helper.
   */
  if (!b || !b->transfer || !address32(address) ||
      (address < 0xe08 && address + 4 > 0xe04))
    return -1;
  uint8_t tx[6] = {(uint8_t)(0x20 | (address >> 8)),
                   (uint8_t)address,
                   (uint8_t)value,
                   (uint8_t)(value >> 8),
                   (uint8_t)(value >> 16),
                   (uint8_t)(value >> 24)},
          rx[6] = {0};
  return b->transfer(b->ctx, tx, rx, sizeof tx) ? -1 : 0;
}
int mcp_configuration_reset(const McpBus *b, uint64_t timeout) {
  if (!b || !b->milliseconds || !timeout)
    return -1;
  uint64_t start = b->milliseconds(b->ctx);
  uint32_t con;
  if (mcp_read32(b, 0, &con))
    return -1;
  if (mcp_write32(b, 0, (con & ~UINT32_C(0x07000000)) | UINT32_C(0x04000000)))
    return -1;
  /* Finite transaction budget also prevents a stopped injected clock hanging.
   */
  for (unsigned i = 0; i < 1024; i++) {
    uint64_t now = b->milliseconds(b->ctx);
    if (now < start || now - start >= timeout)
      return -1;
    if (mcp_read32(b, 0, &con))
      return -1;
    if (((con >> 21) & 7) == 4) {
      uint8_t tx[2] = {0, 0}, rx[2] = {0, 0};
      return b->transfer(b->ctx, tx, rx, 2) ? -1 : 0;
    }
  }
  return -1;
}
int can_fd_length(unsigned dlc, unsigned *length) {
  static const uint8_t lengths[16] = {0, 1,  2,  3,  4,  5,  6,  7,
                                      8, 12, 16, 20, 24, 32, 48, 64};
  if (dlc > 15 || !length)
    return -1;
  *length = lengths[dlc];
  return 0;
}
int dma_tx_prepare(DmaSlot *s, const void *v, size_t n) {
  if (!s || s->owner || s->published || n < 14 || n > 1514)
    return -1;
  memcpy(s->data, v, n);
  s->length = n;
  s->complete = 0;
  return 0;
}
int dma_tx_publish(DmaSlot *s, void (*barrier)(void *), void *ctx) {
  if (!s || !barrier || s->owner || s->published || s->length < 14)
    return -1;
  barrier(ctx);
  s->published = 1;
  s->owner = 1;
  return 0;
}
int dma_tx_complete(DmaSlot *s, int error) {
  if (!s || !s->published || !s->owner)
    return -1;
  s->owner = s->published = 0;
  s->length = 0;
  s->complete = !error;
  return error ? -1 : 0;
}
int dma_rx_acquire(DmaSlot *s, unsigned own, unsigned fs, unsigned ls,
                   unsigned error, unsigned fl, size_t *exposed) {
  if (!s || !exposed || own || !fs || !ls || error || fl < 64 || fl > 1518)
    return -1;
  *exposed = fl - 4;
  s->length = *exposed;
  return 0;
}
