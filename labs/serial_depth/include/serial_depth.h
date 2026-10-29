#ifndef SERIAL_DEPTH_H
#define SERIAL_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum
{
    SD_OK = 0,
    SD_INVALID = 1,
    SD_FULL = 2,
    SD_BUSY = 3,
    SD_STALE = 4,
    SD_IO = 5,
    SD_RESET = 6,
    SD_UNSUPPORTED = 7,
    SD_DONE = 8,
    SD_UNIMPLEMENTED = 9
};

struct sd_uart
{
    uint8_t bits[12], length;
    uint16_t brr;
    uint32_t divisor;
};

int sd_uart(uint32_t clock, uint32_t baud, unsigned over8, unsigned width,
            unsigned parity, unsigned stop, uint16_t value,
            struct sd_uart *out);

enum
{
    SD_RXNE = 1,
    SD_ORE = 2,
    SD_FE = 4,
    SD_PE = 8,
    SD_TXE = 16,
    SD_TC = 32
};

struct sd_receive
{
    uint8_t value, valid, error;
};

int sd_uart_receive(unsigned status, uint16_t data, struct sd_receive *out);

struct sd_sync
{
    uint8_t level[16], sample[16], data[8], edges;
};

int sd_sync(unsigned cpol, unsigned cpha, unsigned lbcl, unsigned tx_enabled,
            unsigned rx_enabled, unsigned changing, uint8_t value,
            struct sd_sync *out);

struct sd_sdi2c
{
    uint8_t phase, active, value;
    uint32_t token;
};

enum
{
    SD_I2C_ACK = 1,
    SD_I2C_NACK = 2,
    SD_I2C_BYTE = 3,
    SD_I2C_ARB = 4,
    SD_I2C_TIMEOUT = 5
};

enum
{
    SD_START_WRITE = 1,
    SD_WRITE_REGISTER = 2,
    SD_RESTART_READ = 3,
    SD_READ_NACK_STOP = 4,
    SD_RELEASE = 5,
    SD_STOP = 6
};

int sd_i2c_begin(struct sd_sdi2c *state, uint32_t token, uint8_t address,
                 uint8_t reg, uint8_t *action, uint8_t *wire);
int sd_i2c_event(struct sd_sdi2c *state, uint32_t token, unsigned event,
                 uint8_t value, uint8_t *action, uint8_t *out);
int sd_i2c_clear(unsigned scl_high, unsigned sda_high, unsigned release_after,
                 unsigned *pulses);

struct sd_spi
{
    uint8_t state, count, position, cpol, cpha, rx[16];
    uint32_t token;
};

int sd_spi_begin(struct sd_spi *state, uint32_t token, unsigned mode,
                 size_t count);
int sd_spi_edge(unsigned mode, uint8_t tx, uint8_t peer, uint8_t levels[16],
                uint8_t samples[16], uint8_t mosi[16], uint8_t *rx);
int sd_spi_poll(struct sd_spi *state, uint32_t token, unsigned txe,
                unsigned rxne, unsigned bsy, uint8_t value);
int sd_spi_cancel(struct sd_spi *state, uint32_t token, unsigned stopped);
int sd_electrical(unsigned kind, int32_t millivolts, unsigned drivers,
                  unsigned receivers, unsigned swapped, int *decision);

struct sd_rs485
{
    uint32_t token;
    uint32_t due, last;
    uint8_t phase, count, position, rxcount, event_count, pending_event, tx[16],
        rx[16], events[32];
};

enum
{
    SD_485_RX = 1,
    SD_485_FRAMING = 2,
    SD_485_OVERRUN = 3,
    SD_485_TXDONE = 4,
    SD_485_ABORTED = 5
};

int sd_485_begin(struct sd_rs485 *state, uint32_t token, const uint8_t *data,
                 size_t count, uint32_t now);
int sd_485_poll(struct sd_rs485 *state, uint32_t token, uint32_t now,
                unsigned txe, unsigned tc);
int sd_485_receive(struct sd_rs485 *state, unsigned event, uint8_t value);
int sd_485_cancel(struct sd_rs485 *state, uint32_t token, uint32_t now,
                  unsigned stopped);

struct sd_can
{
    uint32_t identifier;
    uint8_t extended, remote, length, data[8];
};

int sd_can_arbitration(const struct sd_can *a, const struct sd_can *b,
                       int *winner);
int sd_can_encode(const struct sd_can *frame, uint8_t *bits, size_t capacity,
                  size_t *count);
int sd_can_decode(const uint8_t *bits, size_t count, struct sd_can *frame);

struct sd_timing
{
    uint16_t prescaler;
    uint8_t bs1, bs2, sjw;
    uint32_t btr, rate;
};

int sd_can_timing(uint32_t clock, unsigned prescaler, unsigned bs1,
                  unsigned bs2, unsigned sjw, struct sd_timing *out);
int sd_can_filter(uint32_t identifier, uint32_t filter, uint32_t mask,
                  unsigned *accepted);

struct sd_fifo
{
    uint32_t items[3];
    uint8_t count, locked, overrun;
};

int sd_can_push(struct sd_fifo *fifo, uint32_t value);
int sd_can_pop(struct sd_fifo *fifo, uint32_t *value);

struct sd_fd
{
    uint8_t length, crc_width, brs, esi;
    uint16_t spi_bytes;
};

int sd_fd(unsigned dlc, unsigned fdf, unsigned brs, unsigned esi,
          unsigned remote, unsigned fd_capable, struct sd_fd *out);
int sd_uart_decode(const uint8_t *bits, size_t count, unsigned width,
                   unsigned parity, unsigned stop, uint8_t *value);

enum
{
    SD_ADDRESS_NACK = 10,
    SD_DATA_NACK = 11,
    SD_ARBITRATION_LOST = 12,
    SD_TIMEOUT = 13
};

int sd_485_take(struct sd_rs485 *state, uint8_t *event, uint8_t *value);

struct sd_mailbox
{
    uint32_t token;
    uint8_t state, result;
};

int sd_can_submit(struct sd_mailbox *mailbox, uint32_t token);
int sd_can_complete(struct sd_mailbox *mailbox, uint32_t token,
                    unsigned outcome);
int sd_can_take(struct sd_mailbox *mailbox, uint32_t token, uint8_t *outcome);
#endif
