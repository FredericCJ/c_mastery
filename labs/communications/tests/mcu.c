#include "com_mcu.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  uint8_t memory[4096], last[6];
  size_t n;
  unsigned transfers, resets, mode_stuck, fail;
  uint64_t now;
} Model;
static int transfer(void *v, const uint8_t *tx, uint8_t *rx, size_t n) {
  Model *m = v;
  m->transfers++;
  m->n = n;
  memcpy(m->last, tx, n);
  memset(rx, 0, n);
  if (m->fail)
    return -1;
  unsigned command = tx[0] >> 4, address = ((tx[0] & 15u) << 8) | tx[1];
  if (command == 0) {
    if (n != 2 || address || ((m->memory[2] >> 5) & 7) != 4)
      return -1;
    m->resets++;
    return 0;
  }
  if (n != 6 || address + 4 > 4096)
    return -1;
  if (command == 3)
    memcpy(rx + 2, m->memory + address, 4);
  else if (command == 2) {
    memcpy(m->memory + address, tx + 2, 4);
    if (!address && !m->mode_stuck)
      m->memory[2] = (uint8_t)((m->memory[2] & 31) | ((m->memory[3] & 7) << 5));
  } else
    return -1;
  return 0;
}
static uint64_t clock_ms(void *v) { return ((Model *)v)->now++; }
static int spi(void) {
  Model m = {0};
  McpBus b = {&m, transfer, clock_ms};
  CHECK(!mcp_write32(&b, 0x123, 0xdeadbeef));
  CHECK(m.last[0] == 0x21 && m.last[1] == 0x23 && m.last[2] == 0xef &&
        m.last[5] == 0xde);
  uint32_t n = 0;
  CHECK(!mcp_read32(&b, 0x123, &n) && n == 0xdeadbeef && m.last[0] == 0x31);
  unsigned before = m.transfers;
  CHECK(mcp_write32(&b, 0xfff, 0) && mcp_write32(&b, 0xe04, 0) &&
        m.transfers == before);
  CHECK(!mcp_configuration_reset(&b, 10) && m.resets == 1);
  memset(&m, 0, sizeof m);
  m.mode_stuck = 1;
  CHECK(mcp_configuration_reset(&b, 5) && m.resets == 0 && m.transfers < 10);
  m.fail = 1;
  n = 77;
  CHECK(mcp_read32(&b, 0, &n) && n == 77);
  unsigned size;
  CHECK(!can_fd_length(15, &size) && size == 64);
  CHECK(can_fd_length(16, &size));
  return 0;
}
static void barrier(void *v) { (*(unsigned *)v)++; }
static int dma(void) {
  DmaSlot s = {0};
  uint8_t p[64] = {0};
  unsigned barriers = 0;
  CHECK(!dma_tx_prepare(&s, p, 64));
  CHECK(!dma_tx_publish(&s, barrier, &barriers) && barriers == 1 && s.owner);
  CHECK(dma_tx_prepare(&s, p, 64));
  CHECK(!dma_tx_complete(&s, 0) && s.complete);
  CHECK(!dma_tx_prepare(&s, p, 64));
  CHECK(!dma_tx_publish(&s, barrier, &barriers) && barriers == 2);
  CHECK(dma_tx_complete(&s, 1) && !s.complete && !s.owner);
  size_t exposed = 99;
  CHECK(dma_rx_acquire(&s, 1, 1, 1, 0, 64, &exposed) && exposed == 99);
  CHECK(!dma_rx_acquire(&s, 0, 1, 1, 0, 64, &exposed) && exposed == 60);
  CHECK(dma_rx_acquire(&s, 0, 1, 0, 0, 64, &exposed));
  CHECK(dma_rx_acquire(&s, 0, 1, 1, 0, 63, &exposed));
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  int r = !strcmp(argv[1], "spi") ? spi() : !strcmp(argv[1], "dma") ? dma() : 2;
  if (!r)
    printf("PASS mcu_%s (model only)\n", argv[1]);
  return r;
}
