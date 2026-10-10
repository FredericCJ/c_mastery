#include "firmware_io.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "%d: %s\n", __LINE__, #x);                               \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  uint32_t reg[16], writes[64][2];
  unsigned count, reads, scl, release_after;
} Model;
static uint32_t read_reg(void *p, unsigned r) {
  Model *m = p;
  m->reads++;
  if (r == FW_SDA && m->release_after)
    return m->scl >= m->release_after;
  return m->reg[r];
}
static void write_reg(void *p, unsigned r, uint32_t v) {
  Model *m = p;
  if (m->count < 64) {
    m->writes[m->count][0] = r;
    m->writes[m->count++][1] = v;
  }
  m->reg[r] = v;
  if (r == FW_SCL && v)
    m->scl++;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  Model m = {0};
  FwIo io = {&m, read_reg, write_reg};
  if (!strcmp(argv[1], "startup")) {
    uint8_t d[5] = {9, 9, 9, 9, 9}, b[5] = {8, 8, 8, 8, 8}, l[] = {1, 2, 3};
    CHECK(fw_startup(d + 1, l, 3, b + 1, 3) == 0);
    CHECK(d[0] == 9 && d[4] == 9 && d[1] == 1 && d[3] == 3 && b[0] == 8 &&
          b[4] == 8 && b[1] == 0 && b[3] == 0);
    CHECK(fw_startup(0, 0, 0, 0, 0) == 0);
    CHECK(fw_startup(0, l, 1, b, 1) != 0);
  } else if (!strcmp(argv[1], "vectors")) {
    CHECK(fw_vector(0x20001000, 0x08000101, 0x20000000, 0x20001000, 0x08000000,
                    0x08001000) == 0);
    CHECK(fw_vector(0x20001004, 0x08000101, 0x20000000, 0x20001000, 0x08000000,
                    0x08001000) != 0);
    CHECK(fw_vector(0x20000ffc, 0x08000101, 0x20000000, 0x20001000, 0x08000000,
                    0x08001000) != 0);
    CHECK(fw_vector(0x20001000, 0x08000100, 0x20000000, 0x20001000, 0x08000000,
                    0x08001000) != 0);
  } else if (!strcmp(argv[1], "gpio")) {
    CHECK(fw_gpio(io, 15, 0) == 0 && m.reg[FW_GPIO] == 0x80000000u &&
          m.reads == 0);
    CHECK(fw_gpio(io, 16, 1) != 0 && m.count == 1);
    CHECK(fw_gpio(io, 2, 1) == 0 && m.reg[FW_GPIO] == 4);
  } else if (!strcmp(argv[1], "uart")) {
    CHECK(fw_uart(io, 42, 3) != 0 && m.reads == 3 && m.count == 0);
    m.reg[FW_STATUS] = FW_TXE;
    CHECK(fw_uart(io, 42, 3) == 0 && m.reg[FW_DATA] == 42);
  } else if (!strcmp(argv[1], "spi")) {
    uint8_t out = 99;
    CHECK(fw_spi(io, 7, &out, 2) != 0 && m.reg[FW_CONTROL] == 0 && out == 99);
    m.reg[FW_STATUS] = FW_TXE | FW_RXNE;
    CHECK(fw_spi(io, 7, &out, 2) == 0 && out == 7 && m.reg[FW_CONTROL] == 0);
    m.reg[FW_STATUS] |= FW_BUSY;
    out = 99;
    CHECK(fw_spi(io, 1, &out, 2) != 0 && out == 99);
  } else if (!strcmp(argv[1], "i2c")) {
    m.release_after = 3;
    CHECK(fw_i2c_recover(io, 9) == 0 && m.scl == 4 && m.reg[FW_SDA] == 1);
    m = (Model){.release_after = 10};
    CHECK(fw_i2c_recover(io, 9) != 0 && m.scl == 9);
  } else if (!strcmp(argv[1], "timer")) {
    CHECK(fw_timer(io, 16000000, 1000, 250) == 0 && m.reg[FW_PERIOD] == 15999 &&
          m.reg[FW_COMPARE] == 4000 && m.writes[0][1] == 0);
    unsigned n = m.count;
    CHECK(fw_timer(io, 16000001, 1000, 250) != 0 && m.count == n);
    CHECK(fw_timer(io, 1, 0, 0) != 0);
  } else if (!strcmp(argv[1], "adc")) {
    uint16_t x = 99;
    CHECK(fw_adc(io, &x, 2) != 0 && x == 99);
    m.reg[FW_STATUS] = FW_EOC;
    m.reg[FW_ADC] = 0xfabc;
    CHECK(fw_adc(io, &x, 2) == 0 && x == 0xabc);
  } else if (!strcmp(argv[1], "dma")) {
    FwDma d = {0};
    CHECK(fw_dma(&d, io, 0, 0) == 0 && d.state == 1 && d.generation == 1);
    CHECK(fw_dma(&d, io, 0, 0) != 0);
    CHECK(fw_dma(&d, io, 1, 0) != 0 && d.state == 1 &&
          m.reg[FW_DMA] == 1);
    CHECK(fw_dma(&d, io, 2, 0) != 0);
    CHECK(fw_dma(&d, io, 2, 1) != 0);
    CHECK(fw_dma(&d, io, 1, 1) == 0 && d.state == 2);
    /* Disable was requested, but the adapter still observes activity. */
    m.reg[FW_DMA] = 1;
    CHECK(fw_dma(&d, io, 2, 1) != 0 && d.state == 2);
    m.reg[FW_DMA] = 0;
    CHECK(fw_dma(&d, io, 2, 1) == 0 && d.state == 0);
    CHECK(fw_dma(&d, io, 2, 1) != 0);
  } else if (!strcmp(argv[1], "ethernet")) {
    FwDescriptor d = {0};
    m.reg[FW_MDIO] = 1;
    CHECK(fw_ethernet(&d, io, 64, 7, 3) != 0 && !d.own);
    m.reg[FW_MDIO] = 0;
    CHECK(fw_ethernet(&d, io, 64, 7, 3) == 0 && d.own && d.length == 64 &&
          d.generation == 7 && m.reg[FW_DMA] == 2);
    CHECK(fw_ethernet(&d, io, 80, 8, 3) != 0 && d.length == 64);
  } else if (!strcmp(argv[1], "rs485")) {
    m.reg[FW_STATUS] = FW_TXE;
    CHECK(fw_rs485(io, 42, 3) != 0 && m.reg[FW_DE] == 1);
    m.reg[FW_STATUS] |= FW_TC;
    CHECK(fw_rs485(io, 42, 3) == 0 && m.reg[FW_DE] == 0);
  } else if (!strcmp(argv[1], "watchdog")) {
    FwWatchdog w = {.required = 7};
    CHECK(fw_watchdog(&w, io, 1, 0) == 0);
    CHECK(fw_watchdog(&w, io, 2, 0) == 0);
    CHECK(fw_watchdog(&w, io, 4, 0) == 1 && w.feeds == 1 && w.seen == 0);
    CHECK(fw_watchdog(&w, io, 7, 12) < 0 && w.last_fault == 12 && w.feeds == 1);
  } else
    return 2;
  return 0;
}
