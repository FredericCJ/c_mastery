#include "firmware_io.h"
int fw_startup(uint8_t *a, const uint8_t *b, size_t n, uint8_t *c, size_t m) {
  (void)a;
  (void)b;
  (void)n;
  (void)c;
  (void)m;
  return -1;
}
int fw_vector(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e,
              uint32_t f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return -1;
}
int fw_gpio(FwIo a, unsigned b, int c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_uart(FwIo a, uint8_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_spi(FwIo a, uint8_t b, uint8_t *c, unsigned d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_i2c_recover(FwIo a, unsigned b) {
  (void)a;
  (void)b;
  return -1;
}
int fw_timer(FwIo a, uint32_t b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_adc(FwIo a, uint16_t *b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_dma(FwDma *a, FwIo b, unsigned c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_ethernet(FwDescriptor *a, FwIo b, uint32_t c, uint32_t d, unsigned e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return -1;
}
int fw_rs485(FwIo a, uint8_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_watchdog(FwWatchdog *a, FwIo b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
