#ifndef CMASTERY_FIRMWARE_IO_H
#define CMASTERY_FIRMWARE_IO_H
#include <stddef.h>
#include <stdint.h>
/* Register addresses below belong to an educational adapter, not an STM32
   register layout. Port this interface to inspected F439/H745 registers and
   preserve the register trace contract. Hosted tests are NOT board evidence. */
enum {
  FW_STATUS,
  FW_DATA,
  FW_CONTROL,
  FW_GPIO,
  FW_SCL,
  FW_SDA,
  FW_PERIOD,
  FW_COMPARE,
  FW_ADC,
  FW_DMA,
  FW_MDIO,
  FW_DE,
  FW_WATCHDOG
};
enum { FW_TXE = 1, FW_RXNE = 2, FW_BUSY = 4, FW_TC = 8, FW_EOC = 16 };
struct fw_io {
  void *context;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uint32_t);
};
int fw_startup(uint8_t *, const uint8_t *, size_t, uint8_t *, size_t);
int fw_vector(uint32_t stack, uint32_t reset, uint32_t ram_low,
              uint32_t ram_high, uint32_t flash_low, uint32_t flash_high);
int fw_gpio(struct fw_io, unsigned pin, int high);
int fw_uart(struct fw_io, uint8_t, unsigned polls);
int fw_spi(struct fw_io, uint8_t, uint8_t *, unsigned polls);
/* Single-master recovery only, with all other drivers quiesced. */
int fw_i2c_recover(struct fw_io, unsigned pulses);
int fw_timer(struct fw_io, uint32_t clock, uint32_t hz, uint32_t duty_per_mille);
int fw_adc(struct fw_io, uint16_t *, unsigned polls);
struct fw_dma {
  uint32_t generation;
  unsigned state;
};
/* state:0 CPU-owned,1 device-owned,2 cancelling; no release before idle. */
int fw_dma(struct fw_dma *, struct fw_io, unsigned action, uint32_t ticket);
struct fw_descriptor {
  uint32_t own, length, generation;
};
int fw_ethernet(struct fw_descriptor *, struct fw_io, uint32_t length, uint32_t generation,
                unsigned polls);
int fw_rs485(struct fw_io, uint8_t, unsigned polls);
struct fw_watchdog {
  uint32_t required, seen, feeds, last_fault;
};
int fw_watchdog(struct fw_watchdog *, struct fw_io, uint32_t progress, uint32_t fault);
#endif
