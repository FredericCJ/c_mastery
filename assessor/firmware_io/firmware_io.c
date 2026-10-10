#include "firmware_io.h"
static int valid(FwIo io) { return io.read && io.write; }
static int await(FwIo io, uint32_t mask, uint32_t value, unsigned polls) {
  while (polls--)
    if ((io.read(io.context, FW_STATUS) & mask) == value)
      return 0;
  return -1;
}
int fw_startup(uint8_t *data, const uint8_t *load, size_t n, uint8_t *bss,
               size_t m) {
  if ((n && (!data || !load)) || (m && !bss))
    return -1;
  for (size_t i = 0; i < n; i++)
    data[i] = load[i];
  for (size_t i = 0; i < m; i++)
    bss[i] = 0;
  return 0;
}
int fw_vector(uint32_t sp, uint32_t pc, uint32_t lo, uint32_t hi, uint32_t flo,
              uint32_t fhi) {
  return lo < hi && flo < fhi && sp > lo && sp <= hi && !(sp & 7u) &&
                 (pc & 1u) && (pc & ~1u) >= flo && (pc & ~1u) < fhi
             ? 0
             : -1;
}
int fw_gpio(FwIo io, unsigned pin, int high) {
  if (!valid(io) || pin > 15 || (high != 0 && high != 1))
    return -1;
  io.write(io.context, FW_GPIO, 1u << (pin + (high ? 0 : 16)));
  return 0;
}
int fw_uart(FwIo io, uint8_t b, unsigned polls) {
  if (!valid(io) || await(io, FW_TXE, FW_TXE, polls))
    return -1;
  io.write(io.context, FW_DATA, b);
  return 0;
}
int fw_spi(FwIo io, uint8_t b, uint8_t *out, unsigned polls) {
  if (!valid(io) || !out || !polls)
    return -1;
  io.write(io.context, FW_CONTROL, 1);
  int ok = 0;
  if (!await(io, FW_TXE, FW_TXE, polls)) {
    io.write(io.context, FW_DATA, b);
    if (!await(io, FW_RXNE, FW_RXNE, polls)) {
      uint8_t x = (uint8_t)io.read(io.context, FW_DATA);
      if (!await(io, FW_BUSY, 0, polls)) {
        *out = x;
        ok = 1;
      }
    }
  }
  io.write(io.context, FW_CONTROL, 0);
  return ok ? 0 : -1;
}
int fw_i2c_recover(FwIo io, unsigned pulses) {
  if (!valid(io) || pulses > 9)
    return -1;
  io.write(io.context, FW_SDA, 1);
  for (unsigned i = 0; !io.read(io.context, FW_SDA) && i < pulses; i++) {
    io.write(io.context, FW_SCL, 0);
    io.write(io.context, FW_SCL, 1);
    if (!io.read(io.context, FW_SCL))
      return -1;
  }
  if (!io.read(io.context, FW_SDA))
    return -1;
  io.write(io.context, FW_SDA, 0);
  io.write(io.context, FW_SCL, 1);
  if (!io.read(io.context, FW_SCL))
    return -1;
  io.write(io.context, FW_SDA, 1);
  return 0;
}
int fw_timer(FwIo io, uint32_t clock, uint32_t hz, uint32_t duty) {
  if (!valid(io) || !hz || clock < hz || clock % hz || duty > 1000)
    return -1;
  uint32_t ticks = clock / hz;
  if (ticks > 65536)
    return -1;
  io.write(io.context, FW_CONTROL, 0);
  io.write(io.context, FW_PERIOD, ticks - 1);
  io.write(io.context, FW_COMPARE, (ticks * duty) / 1000);
  io.write(io.context, FW_CONTROL, 2);
  return 0;
}
int fw_adc(FwIo io, uint16_t *out, unsigned polls) {
  if (!valid(io) || !out)
    return -1;
  io.write(io.context, FW_CONTROL, 4);
  if (await(io, FW_EOC, FW_EOC, polls))
    return -1;
  *out = (uint16_t)(io.read(io.context, FW_ADC) & 4095);
  return 0;
}
int fw_dma(FwDma *d, FwIo io, unsigned action, uint32_t ticket) {
  if (!d || !valid(io))
    return -1;
  if (action == 0) {
    if (d->state || d->generation == UINT32_MAX)
      return -1;
    d->generation++;
    d->state = 1;
    io.write(io.context, FW_DMA, 1);
    return 0;
  }
  if (ticket != d->generation || !d->state)
    return -1;
  if (action == 1) {
    d->state = 2;
    io.write(io.context, FW_DMA, 0);
    return 0;
  }
  if (action == 2 && !io.read(io.context, FW_DMA)) {
    d->state = 0;
    return 0;
  }
  return -1;
}
int fw_ethernet(FwDescriptor *d, FwIo io, uint32_t n, uint32_t gen,
                unsigned polls) {
  if (!d || !valid(io) || d->own || n < 14 || n > 1518)
    return -1;
  while (polls--) {
    if (!io.read(io.context, FW_MDIO)) {
      d->length = n;
      d->generation = gen;
      d->own = 1;
      io.write(io.context, FW_DMA, 2);
      return 0;
    }
  }
  return -1;
}
int fw_rs485(FwIo io, uint8_t b, unsigned polls) {
  if (!valid(io) || !polls)
    return -1;
  io.write(io.context, FW_DE, 1);
  if (fw_uart(io, b, polls)) {
    io.write(io.context, FW_DE, 0);
    return -1;
  }
  if (await(io, FW_TC, FW_TC, polls))
    return -1;
  io.write(io.context, FW_DE, 0);
  return 0;
}
int fw_watchdog(FwWatchdog *w, FwIo io, uint32_t progress, uint32_t fault) {
  if (!w || !valid(io) || !w->required)
    return -1;
  if (fault) {
    if (!w->last_fault)
      w->last_fault = fault;
    return -1;
  }
  w->seen |= progress & w->required;
  if (w->seen == w->required) {
    io.write(io.context, FW_WATCHDOG, 0xaaaa);
    w->feeds++;
    w->seen = 0;
    return 1;
  }
  return 0;
}
