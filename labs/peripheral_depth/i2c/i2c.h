#ifndef PERIPHERAL_I2C_DEPTH_H
#define PERIPHERAL_I2C_DEPTH_H
#include "uart.h"
#include <stddef.h>
#include <stdint.h>
enum {
  PI_OK,
  PI_INVALID,
  PI_BUSY,
  PI_STALE,
  PI_TIMEOUT,
  PI_ADDRESS_NACK,
  PI_DATA_NACK,
  PI_ARBITRATION,
  PI_IO,
  PI_UNCERTAIN,
  PI_FORBIDDEN,
  PI_UNIMPLEMENTED
};
enum {
  PI_CR1,
  PI_CR2,
  PI_OAR1,
  PI_OAR2,
  PI_DR,
  PI_SR1,
  PI_SR2,
  PI_CCR,
  PI_TRISE,
  PI_FLTR
};
enum {
  PI_PE = 1,
  PI_START = 256,
  PI_STOP = 512,
  PI_ACK = 1024,
  PI_POS = 2048,
  PI_PEC = 4096
};
enum {
  PI_SB = 1,
  PI_ADDR = 2,
  PI_BTF = 4,
  PI_RXNE = 64,
  PI_TXE = 128,
  PI_BERR = 256,
  PI_ARLO = 512,
  PI_AF = 1024,
  PI_OVR = 2048
};
typedef struct {
  void *context;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uint32_t);
} PiIo;
typedef struct {
  uint32_t generation, last_token, token, last, deadline, clock;
  unsigned ready, phase, address, command, count, received, command_sent,
      master, stop_requested, result, berr;
  uint8_t partial[2], *output;
} PiI2c;
int pi_open(PiI2c *, PiIo, const PuClock *);
int pi_begin(PiI2c *, PiIo, uint32_t generation, uint32_t token,
             unsigned address, unsigned command, unsigned count,
             uint8_t *output, uint32_t now, unsigned budget);
int pi_step(PiI2c *, PiIo, uint32_t token, uint32_t now);
int pi_cancel(PiI2c *, PiIo, uint32_t token);
/* Open-drain recovery requires an externally established exclusive lease. */
typedef struct {
  void *context;
  unsigned (*read)(void *, unsigned);
  void (*release)(void *, unsigned, unsigned);
} PiLines;
enum { PI_SCL, PI_SDA };
typedef struct {
  unsigned active, phase, pulses, result;
  uint32_t last, due, deadline;
  unsigned half;
} PiRecovery;
int pi_recover_begin(PiRecovery *, PiLines, unsigned exclusive,
                     unsigned other_masters, uint32_t now, unsigned budget,
                     unsigned half);
int pi_recover_step(PiRecovery *, PiLines, uint32_t now);
/* Original local partner. Commands0x10 read,0x11 increment; no real device. */
int pi_device(unsigned command, unsigned lose_ack, uint8_t *device,
              uint8_t *reply);
int pi_retry_policy(unsigned command, unsigned may_have_applied,
                    unsigned bus_status);
PiIo pi_f439_io(void);
void pi_f439_pins(void);
int pi_f439_step(PiI2c *, uint32_t token, uint32_t now);
int pi_f439_open(PiI2c *, const PuClock *);
int pi_f439_begin(PiI2c *, uint32_t generation, uint32_t token,
                  unsigned address, unsigned command, unsigned count, uint8_t *,
                  uint32_t now, unsigned budget);
int pi_f439_cancel(PiI2c *, uint32_t token);
PiLines pi_f439_lines(void);
int pi_f439_recovery_prepare(PiI2c *, unsigned exclusive,
                             unsigned other_masters);
#endif
