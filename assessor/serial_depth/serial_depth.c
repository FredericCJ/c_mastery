#include "serial_depth.h"
#include <limits.h>
int sd_uart(uint32_t clock, uint32_t baud, unsigned over8, unsigned width,
            unsigned parity, unsigned stop, uint16_t value, struct sd_uart *out) {
  struct sd_uart r = {0};
  uint32_t n, rem;
  unsigned i, p = 0, scale = over8 ? 8u : 16u;
  if (!out || !clock || !baud || over8 > 1 || (width != 7 && width != 8) ||
      parity > 2 || (stop != 1 && stop != 2) || value >= (1u << width))
    return SD_INVALID;
  n = clock / baud;
  rem = clock % baud;
  if (rem >= baud - rem) {
    if (n == UINT32_MAX)
      return SD_INVALID;
    n++;
  }
  if (n < scale || n > scale * 4096u - 1u)
    return SD_INVALID;
  r.divisor = n;
  r.brr = (uint16_t)(((n / scale) << 4) | (n % scale));
  r.bits[r.length++] = 0;
  for (i = 0; i < width; i++) {
    unsigned b = (value >> i) & 1u;
    r.bits[r.length++] = (uint8_t)b;
    p ^= b;
  }
  if (parity)
    r.bits[r.length++] = (uint8_t)(p ^ (parity == 2));
  for (i = 0; i < stop; i++)
    r.bits[r.length++] = 1;
#ifdef SD_MUTATE_UART
  r.bits[1] = (uint8_t)((value >> (width - 1u)) & 1u);
#endif
#ifdef SD_MUTATE_UART2
  r.brr = (uint16_t)n;
#endif
  *out = r;
  return SD_OK;
}
int sd_uart_receive(unsigned status, uint16_t data, struct sd_receive *out) {
  struct sd_receive r = {0};
  if (!out || status > 63u || data > 255u)
    return SD_INVALID;
  r.error = (uint8_t)(status & (SD_ORE | SD_FE | SD_PE));
  r.valid = (uint8_t)((status & SD_RXNE) != 0 && r.error == 0);
  r.value = r.valid ? (uint8_t)data : 0;
  *out = r;
  return SD_OK;
}
int sd_sync(unsigned cpol, unsigned cpha, unsigned lbcl, unsigned tx,
            unsigned rx, unsigned changing, uint8_t value, struct sd_sync *out) {
  struct sd_sync r = {0};
  unsigned i;
  if (!out || cpol > 1 || cpha > 1 || lbcl > 1 || tx > 1 || rx > 1 ||
      changing > 1)
    return SD_INVALID;
  if (changing && (tx || rx))
    return SD_BUSY;
  if (changing) {
    *out = r;
    return SD_OK;
  }
  if (!tx)
    return SD_INVALID;
  r.edges = (uint8_t)(lbcl ? 16 : 14);
  for (i = 0; i < 8; i++)
    r.data[i] = (uint8_t)((value >> i) & 1u);
  for (i = 0; i < r.edges; i++) {
    r.level[i] = (uint8_t)(cpol ^ ((i + 1u) & 1u));
    r.sample[i] = (uint8_t)((i & 1u) == cpha);
  }
#ifdef SD_MUTATE_SYNC
  r.edges = 16;
#endif
#ifdef SD_MUTATE_SYNC2
  for (i = 0; i < r.edges; i++)
    r.sample[i] = (uint8_t)!r.sample[i];
#endif
  *out = r;
  return SD_OK;
}
int sd_i2c_begin(struct sd_sdi2c *s, uint32_t token, uint8_t address, uint8_t reg,
                 uint8_t *action, uint8_t *wire) {
  if (!s || !action || !wire || !token || address > 127)
    return SD_INVALID;
  if (s->active)
    return SD_BUSY;
  if (token <= s->token)
    return SD_STALE;
  s->phase = 1;
  s->active = 1;
  s->value = reg;
  s->token = token;
  *action = SD_START_WRITE;
  *wire = (uint8_t)(address << 1);
  return SD_OK;
}
int sd_i2c_event(struct sd_sdi2c *s, uint32_t token, unsigned event, uint8_t value,
                 uint8_t *action, uint8_t *out) {
  if (!s || !action || !out || event < 1 || event > 5)
    return SD_INVALID;
  if (!s->active || token != s->token)
    return SD_STALE;
  if (s->phase < 1 || s->phase > 4)
    return SD_INVALID;
  if (event == SD_I2C_ARB) {
    s->active = 0;
    *action = SD_RELEASE;
#ifdef SD_MUTATE_I2C
    *action = SD_STOP;
#endif
    return SD_ARBITRATION_LOST;
  }
  if (event == SD_I2C_TIMEOUT || event == SD_I2C_NACK) {
    int result = event == SD_I2C_TIMEOUT ? SD_TIMEOUT
                 : s->phase == 2         ? SD_DATA_NACK
                                         : SD_ADDRESS_NACK;
    if (event == SD_I2C_NACK && s->phase == 4)
      return SD_INVALID;
    s->active = 0;
    *action = SD_STOP;
    return result;
  }
  if (s->phase < 4 && event != SD_I2C_ACK)
    return SD_INVALID;
  if (s->phase == 4 && event != SD_I2C_BYTE)
    return SD_INVALID;
  if (s->phase == 1) {
    s->phase = 2;
    *action = SD_WRITE_REGISTER;
    *out = s->value;
  } else if (s->phase == 2) {
    s->phase = 3;
    *action = SD_RESTART_READ;
  } else if (s->phase == 3) {
    s->phase = 4;
    *action = SD_READ_NACK_STOP;
  } else {
    s->active = 0;
    *action = SD_STOP;
    *out = value;
  }
  return SD_OK;
}
int sd_i2c_clear(unsigned scl, unsigned sda, unsigned after, unsigned *pulses) {
  if (!pulses || scl > 1 || sda > 1 || after > 10)
    return SD_INVALID;
  if (!scl) {
    *pulses = 0;
    return SD_RESET;
  }
  if (sda) {
    *pulses = 0;
    return SD_OK;
  }
  *pulses = after > 0 && after <= 9 ? after : 9;
#ifdef SD_MUTATE_I2C2
  *pulses = 9;
  return SD_OK;
#endif
  return after > 0 && after <= 9 ? SD_OK : SD_RESET;
}
int sd_spi_begin(struct sd_spi *s, uint32_t token, unsigned mode, size_t count) {
  if (!s || !token || mode > 3 || !count || count > 16)
    return SD_INVALID;
  if (s->state)
    return SD_BUSY;
  if (token <= s->token)
    return SD_STALE;
  s->state = 1;
  s->count = (uint8_t)count;
  s->position = 0;
  s->cpol = (uint8_t)(mode >> 1);
  s->cpha = (uint8_t)(mode & 1);
  s->token = token;
  return SD_OK;
}
int sd_spi_edge(unsigned mode, uint8_t tx, uint8_t peer, uint8_t levels[16],
                uint8_t samples[16], uint8_t mosi[16], uint8_t *rx) {
  unsigned i;
  uint8_t r = 0;
  if (mode > 3 || !levels || !samples || !mosi || !rx)
    return SD_INVALID;
  for (i = 0; i < 16; i++) {
    levels[i] = (uint8_t)((mode >> 1) ^ ((i + 1u) & 1u));
    samples[i] = (uint8_t)((i & 1u) == (mode & 1));
    mosi[i] = (uint8_t)((tx >> (7u - i / 2u)) & 1u);
    if (samples[i])
      r = (uint8_t)((r << 1) | ((peer >> (7u - i / 2u)) & 1u));
  }
  *rx = r;
  return SD_OK;
}
int sd_spi_poll(struct sd_spi *s, uint32_t token, unsigned txe, unsigned rxne,
                unsigned bsy, uint8_t value) {
  if (!s || txe > 1 || rxne > 1 || bsy > 1)
    return SD_INVALID;
  if (!s->state || s->token != token)
    return SD_STALE;
  if (s->state == 3)
    return SD_BUSY;
  if (s->count == 0 || s->count > 16 || s->position > s->count || s->state > 3)
    return SD_INVALID;
  if (s->state == 1 && s->position < s->count && txe)
    s->state = 2;
  if (s->state == 2 && rxne) {
    s->rx[s->position++] = value;
    s->state = 1;
  }
  if (s->position == s->count && txe && !bsy) {
    s->state = 0;
    return SD_DONE;
  }
#ifdef SD_MUTATE_SPI
  if (s->position == s->count && txe) {
    s->state = 0;
    return SD_DONE;
  }
#endif
  return SD_BUSY;
}
int sd_spi_cancel(struct sd_spi *s, uint32_t token, unsigned stopped) {
  if (!s || stopped > 1)
    return SD_INVALID;
  if (!s->state || s->token != token)
    return SD_STALE;
#ifdef SD_MUTATE_SPI2
  stopped = 1;
#endif
  if (stopped) {
    s->state = 0;
    return SD_DONE;
  }
  s->state = 3;
  return SD_BUSY;
}
int sd_electrical(unsigned kind, int32_t mv, unsigned drivers,
                  unsigned receivers, unsigned swapped, int *decision) {
  int d;
  if (!decision || kind > 1 || swapped > 1 || drivers != 1 || !receivers ||
      receivers > (kind ? 10u : 1u))
    return SD_INVALID;
  if (kind == 0)
    d = mv <= -3000 ? 1 : mv >= 3000 ? 0 : -1;
  else
    d = mv >= 200 ? 1 : mv <= -200 ? 0 : -1;
  if (d >= 0 && swapped)
    d ^= 1;
#ifdef SD_MUTATE_ELECTRICAL
  if (kind == 1 && mv == 199)
    d = 1;
#endif
#ifdef SD_MUTATE_ELECTRICAL2
  if (kind == 0 && d >= 0)
    d ^= 1;
#endif
  *decision = d;
  return SD_OK;
}
static int sd_time(struct sd_rs485 *s, uint32_t now) { return now >= s->last; }
static int sd_485_event(struct sd_rs485 *s, unsigned event) {
  if (s->event_count >= 32)
    return SD_FULL;
  s->events[s->event_count++] = (uint8_t)event;
  return SD_OK;
}
int sd_485_begin(struct sd_rs485 *s, uint32_t token, const uint8_t *data, size_t n,
                 uint32_t now) {
  size_t i;
  if (!s || !data || !n || n > 16 || !token || now > UINT32_MAX - 600u ||
      now < s->last)
    return SD_INVALID;
  if (s->phase)
    return SD_BUSY;
  if (token <= s->token)
    return SD_STALE;
  s->token = token;
  s->phase = 1;
  s->count = (uint8_t)n;
  s->position = 0;
  s->due = now + 600u;
  s->last = now;
  s->pending_event = SD_485_TXDONE;
  for (i = 0; i < n; i++)
    s->tx[i] = data[i];
  return SD_OK;
}
int sd_485_poll(struct sd_rs485 *s, uint32_t token, uint32_t now, unsigned txe,
                unsigned tc) {
  if (!s || txe > 1 || tc > 1 || !sd_time(s, now) || s->phase > 4)
    return SD_INVALID;
  if (!s->phase || token != s->token)
    return SD_STALE;
  if (s->phase == 4)
    return SD_BUSY;
  if (!s->count || s->count > 16 || s->position > s->count)
    return SD_INVALID;
  if (s->phase == 1 && now >= s->due)
    s->phase = 2;
  if (s->phase == 2) {
    if (s->position < s->count && txe) {
      s->position++;
      s->last = now;
      return SD_BUSY;
    }
    if (s->position == s->count && tc) {
      if (now > UINT32_MAX - 2200u || s->event_count >= 32)
        return SD_FULL;
      s->phase = 3;
      s->due = now + 2200u;
    }
  }
#ifdef SD_MUTATE_RS485
  if (s->phase == 2 && s->position == s->count && txe) {
    s->phase = 3;
    s->due = now + 2200u;
  }
#endif
  s->last = now;
  if (s->phase == 3 && now >= s->due) {
    if (sd_485_event(s, s->pending_event) != SD_OK)
      return SD_FULL;
    s->phase = 0;
    return SD_DONE;
  }
  return SD_BUSY;
}
int sd_485_receive(struct sd_rs485 *s, unsigned event, uint8_t value) {
  if (!s || event < 1 || event > 3)
    return SD_INVALID;
  if (s->phase)
    return SD_BUSY;
  if (s->event_count >= 32 || (event == SD_485_RX && s->rxcount >= 16))
    return SD_FULL;
  if (event == SD_485_RX)
    s->rx[s->rxcount++] = value;
  return sd_485_event(s, event);
}
int sd_485_cancel(struct sd_rs485 *s, uint32_t token, uint32_t now, unsigned stopped) {
  if (!s || stopped > 1 || !sd_time(s, now))
    return SD_INVALID;
  if (!s->phase || s->token != token)
    return SD_STALE;
#ifdef SD_MUTATE_RS4852
  stopped = 1;
#endif
  if (!stopped) {
    s->phase = 4;
    s->last = now;
    return SD_BUSY;
  }
  if (s->event_count >= 32 || now > UINT32_MAX - 2200u)
    return SD_FULL;
  s->phase = 3;
  s->due = now + 2200u;
  s->last = now;
  s->pending_event = SD_485_ABORTED;
  return SD_BUSY;
}
static int can_valid(const struct sd_can *f) {
  return f && f->extended <= 1 && f->remote <= 1 && f->length <= 8 &&
         f->identifier <= (f->extended ? 0x1fffffffu : 0x7ffu);
}
static size_t append(uint8_t *b, size_t n, uint32_t v, unsigned width) {
  while (width)
    b[n++] = (uint8_t)((v >> (--width)) & 1u);
  return n;
}
static size_t arb(const struct sd_can *f, uint8_t *b) {
  size_t n = 0;
  n = append(b, n, f->extended ? f->identifier >> 18 : f->identifier, 11);
  b[n++] = f->extended ? 1 : f->remote;
  b[n++] = f->extended;
  if (f->extended) {
    n = append(b, n, f->identifier & 0x3ffffu, 18);
    b[n++] = f->remote;
  }
  return n;
}
int sd_can_arbitration(const struct sd_can *a, const struct sd_can *b, int *winner) {
  uint8_t x[32], y[32];
  size_t i, n, m;
  if (!winner || !can_valid(a) || !can_valid(b))
    return SD_INVALID;
  n = arb(a, x);
  m = arb(b, y);
  for (i = 0; i < n && i < m; i++)
    if (x[i] != y[i]) {
      *winner = x[i] < y[i] ? -1 : 1;
      return SD_OK;
    }
  *winner = 0;
  return SD_OK;
}
static uint16_t crc15(const uint8_t *b, size_t n) {
  uint16_t c = 0;
  size_t i;
  for (i = 0; i < n; i++) {
    unsigned top = ((c >> 14) ^ b[i]) & 1u;
    c = (uint16_t)((c << 1) & 0x7fffu);
    if (top)
      c ^= 0x4599u;
  }
  return c;
}
int sd_can_encode(const struct sd_can *f, uint8_t *bits, size_t capacity,
                  size_t *count) {
  uint8_t raw[128], stuffed[160];
  size_t n = 0, k = 0, i;
  unsigned run = 0, last = 2;
  if (!can_valid(f) || !bits || !count)
    return SD_INVALID;
  raw[n++] = 0;
  n += arb(f, raw + n);
  if (f->extended)
    raw[n++] = 0;
  raw[n++] = 0;
  n = append(raw, n, f->length, 4);
  if (!f->remote)
    for (i = 0; i < f->length; i++)
      n = append(raw, n, f->data[i], 8);
  n = append(raw, n, crc15(raw, n), 15);
  for (i = 0; i < n; i++) {
    unsigned bit = raw[i];
    stuffed[k++] = (uint8_t)bit;
    run = bit == last ? run + 1u : 1u;
    last = bit;
    if (run == 5) {
      last ^= 1u;
      stuffed[k++] = (uint8_t)last;
      run = 1;
    }
  }
  for (i = 0; i < 10; i++)
    stuffed[k++] = 1;
#ifdef SD_MUTATE_CAN
  stuffed[0] = 1;
#endif
  if (capacity < k)
    return SD_FULL;
  for (i = 0; i < k; i++)
    bits[i] = stuffed[i];
  *count = k;
  return SD_OK;
}
static uint32_t extract(const uint8_t *b, size_t off, unsigned width) {
  uint32_t v = 0;
  while (width--)
    v = (v << 1) | b[off++];
  return v;
}
int sd_can_decode(const uint8_t *bits, size_t count, struct sd_can *out) {
  uint8_t raw[128];
  struct sd_can f = {0};
  size_t n = 0, i = 0, header = 19, needed = 0, j;
  unsigned run = 0, last = 2, expect = 0;
  uint16_t crc;
  if (!bits || !out || count < 44 || count > 160)
    return SD_INVALID;
  while (i < count) {
    unsigned bit = bits[i++];
    if (bit > 1)
      return SD_INVALID;
    if (expect) {
      if (bit == last)
        return SD_IO;
      last = bit;
      run = 1;
      expect = 0;
      if (needed && n == needed)
        break;
      continue;
    }
    if (n >= 128)
      return SD_INVALID;
    raw[n++] = (uint8_t)bit;
    run = bit == last ? run + 1u : 1u;
    last = bit;
    expect = run == 5;
    if (n == 14 && raw[13])
      header = 39;
    if (n == header) {
      f.extended = raw[13];
      f.identifier = extract(raw, 1, 11);
      if (f.extended) {
        if (raw[12] != 1 || raw[33] || raw[34])
          return SD_IO;
        f.identifier = (f.identifier << 18) | extract(raw, 14, 18);
        f.remote = raw[32];
        f.length = (uint8_t)extract(raw, 35, 4);
      } else {
        if (raw[14])
          return SD_IO;
        f.remote = raw[12];
        f.length = (uint8_t)extract(raw, 15, 4);
      }
      if (f.length > 8)
        return SD_IO;
      needed = header + (f.remote ? 0 : 8u * f.length) + 15;
    }
    if (needed && n == needed && !expect)
      break;
  }
  if (!needed || n != needed || count - i != 10 || raw[0])
    return SD_IO;
  for (j = 0; j < 10; j++)
    if (bits[i + j] > 1 || (j != 1 && bits[i + j] != 1))
      return SD_IO;
  crc = (uint16_t)extract(raw, n - 15, 15);
#ifndef SD_MUTATE_CAN2
  if (crc != crc15(raw, n - 15))
    return SD_IO;
#else
  (void)crc;
#endif
  for (j = 0; !f.remote && j < f.length; j++)
    f.data[j] = (uint8_t)extract(raw, header + j * 8, 8);
  *out = f;
  return SD_OK;
}
int sd_can_timing(uint32_t clock, unsigned pre, unsigned bs1, unsigned bs2,
                  unsigned sjw, struct sd_timing *out) {
  struct sd_timing t;
  if (!out || !clock || pre < 1 || pre > 1024 || bs1 < 1 || bs1 > 16 ||
      bs2 < 1 || bs2 > 8 || sjw < 1 || sjw > 4 || sjw > bs2)
    return SD_INVALID;
  t.prescaler = (uint16_t)pre;
  t.bs1 = (uint8_t)bs1;
  t.bs2 = (uint8_t)bs2;
  t.sjw = (uint8_t)sjw;
  t.btr =
      (pre - 1u) | ((bs1 - 1u) << 16) | ((bs2 - 1u) << 20) | ((sjw - 1u) << 24);
  t.rate = clock / (pre * (1u + bs1 + bs2));
#ifdef SD_MUTATE_BXCAN
  t.btr = pre | (bs1 << 16) | (bs2 << 20) | (sjw << 24);
#endif
  *out = t;
  return SD_OK;
}
int sd_can_filter(uint32_t id, uint32_t filter, uint32_t mask,
                  unsigned *accepted) {
  if (!accepted || id > 0x7ffu || filter > 0x7ffu || mask > 0x7ffu)
    return SD_INVALID;
  *accepted = (id & mask) == (filter & mask);
  return SD_OK;
}
int sd_can_push(struct sd_fifo *f, uint32_t value) {
  if (!f || f->count > 3 || f->locked > 1)
    return SD_INVALID;
  if (f->count == 3) {
    f->overrun = 1;
    if (f->locked)
      return SD_FULL;
#ifdef SD_MUTATE_BXCAN2
    f->items[0] = f->items[1];
    f->items[1] = f->items[2];
#endif
    f->items[2] = value;
  } else
    f->items[f->count++] = value;
  return SD_OK;
}
int sd_can_pop(struct sd_fifo *f, uint32_t *value) {
  unsigned i;
  if (!f || !value || f->count > 3 || f->locked > 1)
    return SD_INVALID;
  if (!f->count)
    return SD_FULL;
  *value = f->items[0];
  for (i = 1; i < f->count; i++)
    f->items[i - 1] = f->items[i];
  f->count--;
  return SD_OK;
}
int sd_fd(unsigned dlc, unsigned fdf, unsigned brs, unsigned esi,
          unsigned remote, unsigned capable, struct sd_fd *out) {
  static const uint8_t lengths[16] = {0, 1,  2,  3,  4,  5,  6,  7,
                                      8, 12, 16, 20, 24, 32, 48, 64};
  struct sd_fd r;
  if (!out || dlc > 15 || fdf != 1 || brs > 1 || esi > 1 || remote ||
      capable > 1)
    return SD_INVALID;
  if (!capable)
    return SD_UNSUPPORTED;
  r.length = lengths[dlc];
  r.crc_width = r.length <= 16 ? 17 : 21;
  r.brs = (uint8_t)brs;
  r.esi = (uint8_t)esi;
  r.spi_bytes = (uint16_t)(4u + 4u * ((r.length + 3u) / 4u));
#ifdef SD_MUTATE_FD
  r.length = (uint8_t)dlc;
#endif
#ifdef SD_MUTATE_FD2
  if (r.length == 16)
    r.crc_width = 21;
#endif
  *out = r;
  return SD_OK;
}

int sd_uart_decode(const uint8_t *bits, size_t count, unsigned width,
                   unsigned parity, unsigned stop, uint8_t *value) {
  unsigned i, p = 0;
  uint8_t v = 0;
  if (!bits || !value || (width != 7 && width != 8) || parity > 2 ||
      (stop != 1 && stop != 2) || count != 1 + width + (parity != 0) + stop)
    return SD_INVALID;
  for (i = 0; i < count; i++)
    if (bits[i] > 1)
      return SD_INVALID;
  if (bits[0])
    return SD_IO;
  for (i = 0; i < width; i++) {
    v |= (uint8_t)(bits[i + 1] << i);
    p ^= bits[i + 1];
  }
  if (parity && bits[1 + width] != (p ^ (parity == 2)))
    return SD_IO;
  for (i = 1 + width + (parity != 0); i < count; i++)
    if (!bits[i])
      return SD_IO;
  *value = v;
  return SD_OK;
}
int sd_485_take(struct sd_rs485 *s, uint8_t *event, uint8_t *value) {
  unsigned i;
  uint8_t e, v = 0;
  if (!s || !event || !value || s->event_count > 32 || s->rxcount > 16)
    return SD_INVALID;
  if (!s->event_count)
    return SD_FULL;
  e = s->events[0];
  if (e < 1 || e > 5 || (e == SD_485_RX && !s->rxcount))
    return SD_INVALID;
  if (e == SD_485_RX) {
    v = s->rx[0];
    for (i = 1; i < s->rxcount; i++)
      s->rx[i - 1] = s->rx[i];
    s->rxcount--;
  }
  for (i = 1; i < s->event_count; i++)
    s->events[i - 1] = s->events[i];
  s->event_count--;
  *event = e;
  *value = v;
  return SD_OK;
}
int sd_can_submit(struct sd_mailbox *m, uint32_t token) {
  if (!m || !token)
    return SD_INVALID;
  if (m->state)
    return SD_BUSY;
  if (token <= m->token)
    return SD_STALE;
  m->state = 1;
  m->token = token;
  return SD_OK;
}
int sd_can_complete(struct sd_mailbox *m, uint32_t token, unsigned outcome) {
  if (!m || outcome > 3)
    return SD_INVALID;
  if (m->state != 1 || token != m->token)
    return SD_STALE;
  if (outcome == 1)
    return SD_BUSY;
  m->result = (uint8_t)outcome;
  m->state = 2;
  return SD_OK;
}
int sd_can_take(struct sd_mailbox *m, uint32_t token, uint8_t *outcome) {
  if (!m || !outcome)
    return SD_INVALID;
  if (!m->state || token != m->token)
    return SD_STALE;
  if (m->state == 1)
    return SD_BUSY;
  if (m->state != 2)
    return SD_INVALID;
  *outcome = m->result;
  m->state = 0;
  return SD_OK;
}
