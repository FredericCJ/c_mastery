#define main contract_main
#include "../../../labs/peripheral_depth/uart/tests.c"
#undef main
#include "firmware_io.h"
typedef struct {
  Device device;
  unsigned de;
} Adapter;
static uint32_t read_fw(void *p, unsigned r) {
  Adapter *a = p;
  if (r == FW_DMA)
    return a->device.dma_busy;
  if (r == FW_STATUS) {
    uint32_t sr = rd(&a->device, PU_SR);
    return ((sr & PU_TXE) ? FW_TXE : 0) | ((sr & PU_TC) ? FW_TC : 0);
  }
  return 0;
}
static void write_fw(void *p, unsigned r, uint32_t v) {
  Adapter *a = p;
  if (r == FW_DE)
    a->de = v;
  else if (r == FW_DMA)
    wr(&a->device, PU_DMA_CONTROL, v);
  else if (r == FW_DATA)
    wr(&a->device, PU_DR, v);
}
int main(void) {
  for (unsigned i = 0; i < 256; i++) {
    Adapter a = {0};
    a.device.gate = 1;
    a.device.synced = 1;
    a.device.sr = PU_TXE | PU_TC;
    FwIo io = {&a, read_fw, write_fw};
    CHECK(fw_uart(io, (uint8_t)i, 2) == 0 && a.device.writes == 1 &&
          a.device.wire[0] == i && !(a.device.sr & PU_TC));
    a.device.writes = 0;
    CHECK(fw_rs485(io, (uint8_t)i, 2) == -1 && a.de == 1 &&
          a.device.writes == 1);
    FwDma d = {0};
    CHECK(fw_dma(&d, io, 0, 0) == 0 && d.state == 1 && a.device.dma_busy);
    CHECK(fw_dma(&d, io, 1, d.generation) == 0 && d.state == 2);
    CHECK(fw_dma(&d, io, 2, d.generation) == -1 && d.state == 2);
    a.device.dma_busy = 0;
    CHECK(fw_dma(&d, io, 2, d.generation) == 0 && !d.state);
    CHECK(!a.device.bad);
  }
  puts("PU_COMPOSE_PASS 256 byte cases: actual FW-04/FW-09/FW-11 contracts");
  return 0;
}
