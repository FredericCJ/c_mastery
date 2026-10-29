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
int fw_gpio(struct fw_io a, unsigned b, int c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_uart(struct fw_io a, uint8_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_spi(struct fw_io a, uint8_t b, uint8_t *c, unsigned d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_i2c_recover(struct fw_io a, unsigned b) {
  (void)a;
  (void)b;
  return -1;
}
int fw_timer(struct fw_io a, uint32_t b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_adc(struct fw_io a, uint16_t *b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_dma(struct fw_dma *a, struct fw_io b, unsigned c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
int fw_ethernet(struct fw_descriptor *a, struct fw_io b, uint32_t c, uint32_t d, unsigned e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return -1;
}
int fw_rs485(struct fw_io a, uint8_t b, unsigned c) {
  (void)a;
  (void)b;
  (void)c;
  return -1;
}
int fw_watchdog(struct fw_watchdog *a, struct fw_io b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return -1;
}
