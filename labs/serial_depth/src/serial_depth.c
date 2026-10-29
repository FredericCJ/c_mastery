#include "serial_depth.h"
int sd_uart(uint32_t clock, uint32_t baud, unsigned over8, unsigned width,
            unsigned parity, unsigned stop, uint16_t value, struct sd_uart *out) {
  (void)clock;
  (void)baud;
  (void)over8;
  (void)width;
  (void)parity;
  (void)stop;
  (void)value;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_uart_receive(unsigned status, uint16_t data, struct sd_receive *out) {
  (void)status;
  (void)data;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_sync(unsigned cpol, unsigned cpha, unsigned lbcl, unsigned tx_enabled,
            unsigned rx_enabled, unsigned changing, uint8_t value,
            struct sd_sync *out) {
  (void)cpol;
  (void)cpha;
  (void)lbcl;
  (void)tx_enabled;
  (void)rx_enabled;
  (void)changing;
  (void)value;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_i2c_begin(struct sd_sdi2c *state, uint32_t token, uint8_t address, uint8_t reg,
                 uint8_t *action, uint8_t *wire) {
  (void)state;
  (void)token;
  (void)address;
  (void)reg;
  (void)action;
  (void)wire;
  return SD_UNIMPLEMENTED;
}
int sd_i2c_event(struct sd_sdi2c *state, uint32_t token, unsigned event, uint8_t value,
                 uint8_t *action, uint8_t *out) {
  (void)state;
  (void)token;
  (void)event;
  (void)value;
  (void)action;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_i2c_clear(unsigned scl_high, unsigned sda_high, unsigned release_after,
                 unsigned *pulses) {
  (void)scl_high;
  (void)sda_high;
  (void)release_after;
  (void)pulses;
  return SD_UNIMPLEMENTED;
}
int sd_spi_begin(struct sd_spi *state, uint32_t token, unsigned mode, size_t count) {
  (void)state;
  (void)token;
  (void)mode;
  (void)count;
  return SD_UNIMPLEMENTED;
}
int sd_spi_edge(unsigned mode, uint8_t tx, uint8_t peer, uint8_t levels[16],
                uint8_t samples[16], uint8_t mosi[16], uint8_t *rx) {
  (void)mode;
  (void)tx;
  (void)peer;
  (void)levels;
  (void)samples;
  (void)mosi;
  (void)rx;
  return SD_UNIMPLEMENTED;
}
int sd_spi_poll(struct sd_spi *state, uint32_t token, unsigned txe, unsigned rxne,
                unsigned bsy, uint8_t value) {
  (void)state;
  (void)token;
  (void)txe;
  (void)rxne;
  (void)bsy;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_spi_cancel(struct sd_spi *state, uint32_t token, unsigned stopped) {
  (void)state;
  (void)token;
  (void)stopped;
  return SD_UNIMPLEMENTED;
}
int sd_electrical(unsigned kind, int32_t millivolts, unsigned drivers,
                  unsigned receivers, unsigned swapped, int *decision) {
  (void)kind;
  (void)millivolts;
  (void)drivers;
  (void)receivers;
  (void)swapped;
  (void)decision;
  return SD_UNIMPLEMENTED;
}
int sd_485_begin(struct sd_rs485 *state, uint32_t token, const uint8_t *data,
                 size_t count, uint32_t now) {
  (void)state;
  (void)token;
  (void)data;
  (void)count;
  (void)now;
  return SD_UNIMPLEMENTED;
}
int sd_485_poll(struct sd_rs485 *state, uint32_t token, uint32_t now, unsigned txe,
                unsigned tc) {
  (void)state;
  (void)token;
  (void)now;
  (void)txe;
  (void)tc;
  return SD_UNIMPLEMENTED;
}
int sd_485_receive(struct sd_rs485 *state, unsigned event, uint8_t value) {
  (void)state;
  (void)event;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_485_cancel(struct sd_rs485 *state, uint32_t token, uint32_t now,
                  unsigned stopped) {
  (void)state;
  (void)token;
  (void)now;
  (void)stopped;
  return SD_UNIMPLEMENTED;
}
int sd_can_arbitration(const struct sd_can *a, const struct sd_can *b, int *winner) {
  (void)a;
  (void)b;
  (void)winner;
  return SD_UNIMPLEMENTED;
}
int sd_can_encode(const struct sd_can *frame, uint8_t *bits, size_t capacity,
                  size_t *count) {
  (void)frame;
  (void)bits;
  (void)capacity;
  (void)count;
  return SD_UNIMPLEMENTED;
}
int sd_can_decode(const uint8_t *bits, size_t count, struct sd_can *frame) {
  (void)bits;
  (void)count;
  (void)frame;
  return SD_UNIMPLEMENTED;
}
int sd_can_timing(uint32_t clock, unsigned prescaler, unsigned bs1,
                  unsigned bs2, unsigned sjw, struct sd_timing *out) {
  (void)clock;
  (void)prescaler;
  (void)bs1;
  (void)bs2;
  (void)sjw;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_can_filter(uint32_t identifier, uint32_t filter, uint32_t mask,
                  unsigned *accepted) {
  (void)identifier;
  (void)filter;
  (void)mask;
  (void)accepted;
  return SD_UNIMPLEMENTED;
}
int sd_can_push(struct sd_fifo *fifo, uint32_t value) {
  (void)fifo;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_can_pop(struct sd_fifo *fifo, uint32_t *value) {
  (void)fifo;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_fd(unsigned dlc, unsigned fdf, unsigned brs, unsigned esi,
          unsigned remote, unsigned fd_capable, struct sd_fd *out) {
  (void)dlc;
  (void)fdf;
  (void)brs;
  (void)esi;
  (void)remote;
  (void)fd_capable;
  (void)out;
  return SD_UNIMPLEMENTED;
}
int sd_uart_decode(const uint8_t *bits, size_t count, unsigned width,
                   unsigned parity, unsigned stop, uint8_t *value) {
  (void)bits;
  (void)count;
  (void)width;
  (void)parity;
  (void)stop;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_485_take(struct sd_rs485 *state, uint8_t *event, uint8_t *value) {
  (void)state;
  (void)event;
  (void)value;
  return SD_UNIMPLEMENTED;
}
int sd_can_submit(struct sd_mailbox *mailbox, uint32_t token) {
  (void)mailbox;
  (void)token;
  return SD_UNIMPLEMENTED;
}
int sd_can_complete(struct sd_mailbox *mailbox, uint32_t token, unsigned outcome) {
  (void)mailbox;
  (void)token;
  (void)outcome;
  return SD_UNIMPLEMENTED;
}
int sd_can_take(struct sd_mailbox *mailbox, uint32_t token, uint8_t *outcome) {
  (void)mailbox;
  (void)token;
  (void)outcome;
  return SD_UNIMPLEMENTED;
}
