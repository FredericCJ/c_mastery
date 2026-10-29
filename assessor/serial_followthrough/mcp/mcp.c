#include "mcp.h"
#include <string.h>
#ifndef MC_MUTANT
#define MC_MUTANT 0
#endif
static const unsigned sizes[8] = {8, 12, 16, 20, 24, 32, 48, 64};
static const unsigned lengths[16] = {0, 1,  2,  3,  4,  5,  6,  7,
                                     8, 12, 16, 20, 24, 32, 48, 64};
static uint32_t get(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
         (uint32_t)p[3] << 24;
}
static void put(uint8_t *p, uint32_t n) {
  for (unsigned i = 0; i < 4; i++)
    p[i] = (uint8_t)(n >> (8 * i));
}
uint16_t mc_crc(const uint8_t *p, size_t n) {
  uint16_t c = 65535;
  for (size_t i = 0; i < n; i++) {
    c ^= (uint16_t)((unsigned)p[i] << 8);
    for (unsigned j = 0; j < 8; j++)
      c = (uint16_t)((unsigned)c << 1) ^ ((c & 32768) ? 32773 : 0);
  }
  return c;
}
static int range(uint16_t a, unsigned n) {
  if (!n || n > 76)
    return 0;
  if (a >= 0x400 && a < 0xc00)
    return !(a & 3) && !(n & 3) && n <= 0xc00u - a;
  return (a <= 0x2ec && n <= 0x2f0u - a) ||
         (a >= 0xe00 && a <= 0xe17 && n <= 0xe18u - a);
}
int mc_read(struct mc_bus b, uint16_t a, uint8_t *out, unsigned n) {
  if (!b.transfer || !out || !range(a, n))
    return MC_INVALID;
  uint8_t staged[76];
  for (unsigned off = 0; off < n;) {
    unsigned count = n - off;
    if (count > 8)
      count = 8;
    unsigned addr = a + off;
    uint8_t tx[13] = {0}, rx[13], check[11];
    tx[0] = (uint8_t)(0xb0 | (addr >> 8));
    tx[1] = (uint8_t)addr;
    tx[2] = (uint8_t)(addr >= 0x400 && addr < 0xc00 ? count / 4 : count);
    unsigned attempt;
    for (attempt = 0; attempt < 3; attempt++) {
      if (b.transfer(b.context, tx, rx, count + 5))
        return MC_IO;
      memcpy(check, tx, 3);
      memcpy(check + 3, rx + 3, count);
      if (MC_MUTANT == 1 ||
          mc_crc(check, count + 3) ==
              ((unsigned)rx[count + 3] << 8 | rx[count + 4])) {
        memcpy(staged + off, rx + 3, count);
        break;
      }
    }
    if (attempt == 3)
      return MC_CRC;
    off += count;
  }
  memcpy(out, staged, n);
  return MC_OK;
}
int mc_write(struct mc_bus b, uint16_t a, const uint8_t *p, unsigned n) {
  if (!b.transfer || !p || !range(a, n) ||
      (a < 0xe08 && a + n > 0xe04 && n != 1))
    return MC_INVALID;
  for (unsigned off = 0; off < n;) {
    unsigned count = n - off, addr = a + off;
    if (count > 12)
      count = 12;
    uint8_t tx[14], rx[14];
    tx[0] = (uint8_t)(0x20 | (addr >> 8));
    tx[1] = (uint8_t)addr;
    memcpy(tx + 2, p + off, count);
    if (b.transfer(b.context, tx, rx, count + 2))
      return MC_IO;
    off += count;
  }
  return MC_OK;
}
static int rd(struct mc_bus b, uint16_t a, uint32_t *out) {
  uint8_t p[4];
  int r = mc_read(b, a, p, 4);
  if (!r)
    *out = get(p);
  return r;
}
static int wr(struct mc_bus b, uint16_t a, uint32_t v) {
  uint8_t p[4];
  put(p, v);
  return mc_write(b, a, p, 4);
}
static int byte(struct mc_bus b, uint16_t a, uint8_t v) {
  return mc_write(b, a, &v, 1);
}
static int verify(struct mc_bus b, uint16_t a, uint32_t v, uint32_t mask) {
  uint32_t got;
  int r = wr(b, a, v);
  if (r)
    return r;
  r = rd(b, a, &got);
  return r ? r : ((got & mask) == (v & mask) ? MC_OK : MC_DEVICE);
}
static int wait_bits(struct mc_bus b, uint16_t a, uint32_t mask, uint32_t value,
                     unsigned budget) {
  uint32_t start = b.now(b.context), last = start;
  for (unsigned i = 0; i < 1024; i++) {
    uint32_t now = b.now(b.context), got;
    if (now - last >= UINT32_C(0x80000000) || now - start >= budget)
      return MC_TIMEOUT;
    last = now;
    int r = rd(b, a, &got);
    if (r)
      return r;
    if ((got & mask) == value)
      return MC_OK;
  }
  return MC_TIMEOUT;
}
int mc_layout(unsigned code, unsigned depth, struct mc_layout *out) {
  if (code > 7 || !depth || depth > 16 || !out)
    return MC_INVALID;
  struct mc_layout l = {8 + sizes[code],
                depth * (8 + sizes[code]),
                MC_MUTANT == 5 ? 0 : 29 * 16,
                0,
                0,
                0};
  l.total = l.tx_bytes + l.rx_bytes + l.unused_bytes;
  if (l.total > 2048)
    return MC_INVALID;
  l.txcon =
      (uint32_t)code << 29 |
      0x90; /* one TX object, no automatic retransmission, attempt interrupt */
  l.rxcon = (uint32_t)code << 29 | (uint32_t)(depth - 1) << 24 | 9;
  *out = l;
  return MC_OK;
}
static int poison(struct mc_driver *d, int r) {
  if (r)
    d->state = MC_POISONED;
  return r;
}
int mc_init(struct mc_driver *d, struct mc_bus b, const struct mc_config *c, unsigned budget) {
  struct mc_layout l;
  if (!d || d->state != MC_NEW || !b.transfer || !b.now || !c || !budget ||
      budget >= 0x80000000u || c->loopback > 1 || c->expected_devid > 255 ||
      c->accepted_id > 2047 || c->clock_hz != 40000000 || !c->spi_hz ||
      c->spi_hz > 17000000 ||
      (c->data_rate != 1000000 && c->data_rate != 2000000) ||
      mc_layout(c->payload_code, c->rx_depth, &l))
    return MC_INVALID;
  d->state = MC_MUTANT == 7 ? MC_READY : MC_POISONED;
  uint32_t got;
  int r = rd(b, 0, &got);
  if (r)
    return r;
  if (got == UINT32_MAX)
    return MC_DEVICE;
  if ((r = byte(b, 3, 4)) || (r = wait_bits(b, 0, 7u << 21, 4u << 21, budget)))
    return r;
  {
    uint8_t tx[2] = {0}, rx[2];
    if (b.transfer(b.context, tx, rx, 2))
      return MC_IO;
  }
  if ((r = wait_bits(b, 0, 7u << 21, 4u << 21, budget)) ||
      (r = rd(b, 0xe14, &got)))
    return r;
  if (got != c->expected_devid)
    return MC_DEVICE;
  /* Awake/cold-start only: no OSCDIS sleep/wake operation. */
  if ((r = verify(b, 0xe00, 0, 0x7b)) ||
      (r = wait_bits(b, 0xe00, 1u << 10, 1u << 10, budget)) ||
      (r = wait_bits(b, 0xe00, 1u << 10, 1u << 10, budget)))
    return r;
  for (unsigned i = 0; i < 4; i++)
    if ((r = byte(b, (uint16_t)(0xe04 + i), i == 0 ? 3 : 0)))
      return r;
  if ((r = verify(b, 0xe0c, MC_MUTANT == 6 ? 1 : 7, 7)))
    return r;
  uint8_t zeros[64] = {0}, readback[64];
  for (unsigned a = 0x400; a < 0xc00; a += 64) {
    if ((r = mc_write(b, (uint16_t)a, zeros, 64)) ||
        (r = mc_read(b, (uint16_t)a, readback, 64)))
      return r;
    if (memcmp(zeros, readback, 64))
      return MC_DEVICE;
  }
  /* ISO CRC; protocol exceptions enabled; gateway ESI; restricted retries;
     no TEF/TXQ. 40MHz: nominal500k at75%, data2M/1M at75%. */
  uint32_t base = 0x04030020, nb = (58u << 16) | (19u << 8) | 3u,
           db = c->data_rate == 2000000 ? (13u << 16) | (4u << 8) | 3u
                                        : (28u << 16) | (9u << 8) | 3u;
  if ((r = verify(b, 0, base, 0x071f17ff)) ||
      (r = verify(b, 4, nb, UINT32_MAX)) ||
      (r = verify(b, 8, db, UINT32_MAX)) ||
      (r = verify(b, 12, 0x20000u | (c->data_rate == 2000000 ? 15u : 30u) << 8,
                  0x3037f3f)))
    return r;
  if ((r = verify(b, 0x5c, l.txcon, 0xff7f00ff)) ||
      (r = verify(b, 0x68, l.rxcon, 0xff7f00ff)))
    return r;
  if ((r = wr(b, 0x1d0, 0)) ||
      (r = verify(b, 0x1f0, c->accepted_id, UINT32_MAX)) ||
      (r = verify(b, 0x1f4, 0x400007ff, UINT32_MAX)) ||
      (r = verify(b, 0x1d0, 0x82, UINT32_MAX)))
    return r;
  /* Clear software-clearable startup flags, then enable RX/overflow/TXAT,
     CAN/system/ECC/SPI-CRC sources. Status is cleared at the source. */
  if ((r = wr(b, 0x1c, 0x3f020000)) ||
      (r = byte(b, 3, (uint8_t)(c->loopback ? 2 : 0))) ||
      (r = wait_bits(b, 0, 7u << 21, (c->loopback ? 2u : 0u) << 21, budget)) ||
      (r = wait_bits(b, 0x5c, 0x400, 0, budget)) ||
      (r = wait_bits(b, 0x68, 0x400, 0, budget)))
    return r;
  if ((r = rd(b, 0x64, &got)))
    return r;
  if (got != 0)
    return MC_DEVICE;
  if ((r = rd(b, 0x70, &got)))
    return r;
  if (got != l.tx_bytes)
    return MC_DEVICE;
  d->config = *c;
  d->payload = sizes[c->payload_code];
  d->rx_depth = c->rx_depth;
  d->state = MC_READY;
  return MC_OK;
}
static int frame(const struct mc_frame *f) {
  return f && f->id <= 2047 && f->dlc <= 15 && f->brs <= 1 && f->esi <= 1;
}
int mc_send(struct mc_driver *d, struct mc_bus b, uint32_t token, const struct mc_frame *f) {
  if (!d || d->state != MC_READY || !b.transfer || !frame(f))
    return MC_INVALID;
  if (d->pending)
    return MC_BUSY;
  if (!token || token <= d->last_token)
    return MC_STALE;
  unsigned n = lengths[f->dlc];
  if (n > d->payload || (MC_MUTANT == 2 && !n))
    return MC_INVALID;
  uint32_t status, ua;
  int r = rd(b, 0x60, &status);
  if (r)
    return poison(d, r);
  if (!(status & 1))
    return MC_BUSY;
  if ((r = rd(b, 0x64, &ua)))
    return poison(d, r);
  if (ua != 0)
    return poison(d, MC_DEVICE);
  uint8_t p[72] = {0}, back[72];
  put(p, f->id);
  put(p + 4,
      (uint32_t)f->dlc | 0x80u | (unsigned)f->brs << 6 | (unsigned)f->esi << 8);
  memcpy(p + 8, f->data, n);
  unsigned bytes = (8 + n + 3) & ~3u;
  if ((r = mc_write(b, (uint16_t)(0x400 + ua), p, bytes)) ||
      (r = mc_read(b, (uint16_t)(0x400 + ua), back, bytes)))
    return poison(d, r);
  if (memcmp(p, back, bytes))
    return poison(d, MC_DEVICE);
  /* Reserve the token BEFORE an indeterminate publication. No blind retry. */
  d->pending = 1;
  d->token = d->last_token = token;
  d->terminal = 0;
  d->cancel_requested = 0;
  if ((r = byte(b, 0x5d, MC_MUTANT == 3 ? 2 : 3)))
    return poison(d, r);
  return MC_OK;
}
int mc_poll(struct mc_driver *d, struct mc_bus b) {
  if (!d || d->state != MC_READY || !d->pending)
    return MC_INVALID;
  if (d->terminal)
    return MC_OK;
  uint32_t con, status, trec;
  int r = rd(b, 0x5c, &con);
  if (r)
    return poison(d, r);
  if ((r = rd(b, 0x60, &status)) || (r = rd(b, 0x34, &trec)))
    return poison(d, r);
  if (trec & (1u << 21))
    return poison(d, MC_DEVICE);
  if (con & 0x200)
    return MC_BUSY;
  if (status & 0xb0) {
    d->result = MC_FAILED;
    d->terminal = 1;
    return MC_OK;
  }
  if (!(status & 4))
    return poison(d, MC_DEVICE);
  d->result = MC_OK;
  d->terminal = 1;
  return MC_OK;
}
int mc_cancel(struct mc_driver *d, struct mc_bus b) {
  if (!d || d->state != MC_READY || !d->pending)
    return MC_INVALID;
  if (d->terminal || d->cancel_requested)
    return MC_OK;
  int r = byte(b, 0x5d, 0);
  if (r)
    return poison(d, r);
  d->cancel_requested = 1;
  return MC_OK;
}
int mc_take(struct mc_driver *d, struct mc_bus b, uint32_t token, unsigned budget,
            unsigned *out) {
  if (!d || d->state != MC_READY || !out || !b.now || !budget ||
      budget >= 0x80000000u)
    return MC_INVALID;
  if (!d->pending || token != d->token)
    return MC_STALE;
  if (!d->terminal)
    return MC_BUSY;
  int r = byte(b, 0x5d, 4);
  if (r)
    return poison(d, r);
  r = wait_bits(b, 0x5c, 0x400, 0, budget);
  if (r)
    return poison(d, r);
  *out = d->result;
  d->pending = 0;
  d->terminal = 0;
  return MC_OK;
}
int mc_receive(struct mc_driver *d, struct mc_bus b, struct mc_frame *out) {
  if (!d || d->state != MC_READY || !out)
    return MC_INVALID;
  uint32_t status, ua;
  int r = rd(b, 0x6c, &status);
  if (r)
    return poison(d, r);
  if (!(status & 1))
    return MC_EMPTY;
  if ((r = rd(b, 0x70, &ua)))
    return poison(d, r);
  unsigned stride = 8 + d->payload;
  if (ua < stride || ua >= stride * (1 + d->rx_depth) || (ua - stride) % stride)
    return poison(d, MC_DEVICE);
  uint8_t p[72];
  if ((r = mc_read(b, (uint16_t)(0x400 + ua), p, stride)))
    return poison(d, r);
  uint32_t id = get(p) & 2047u, ctrl = get(p + 4);
  unsigned n = lengths[ctrl & 15];
  if (!(ctrl & 0x80) || (ctrl & 0x30) || n > d->payload)
    return poison(d, MC_DEVICE);
  struct mc_frame f = {0};
  f.id = (uint16_t)id;
  f.dlc = (uint8_t)(ctrl & 15);
  f.brs = (uint8_t)((ctrl >> 6) & 1);
  f.esi = (uint8_t)((ctrl >> 8) & 1);
  memcpy(f.data, p + 8, n);
  if (MC_MUTANT == 10)
    *out = f;
  if ((r = byte(b, 0x69, MC_MUTANT == 4 ? 0 : 1)))
    return poison(d, r);
  *out = f;
  return MC_OK;
}
int mc_interrupt(struct mc_driver *d, struct mc_bus b, uint32_t *out) {
  if (!d || d->state != MC_READY || !out)
    return MC_INVALID;
  uint32_t flags, ecc;
  int r = rd(b, 0x1c, &flags);
  if (r)
    return poison(d, r);
  if ((r = rd(b, 0xe10, &ecc)))
    return poison(d, r);
  *out = flags & 65535;
  /* Neither SEC nor DED means trustworthy corrected data (erratum2). */
  if ((ecc & 6) || (flags & 0x1200))
    return poison(d, MC_DEVICE);
  if (flags & 0x800) {
    if ((r = byte(b, 0x6c, 0xf7)))
      return poison(d, r);
  }
  return MC_OK;
}
int mc_bench_frame(unsigned index, unsigned brs, struct mc_frame *out) {
  static const unsigned dlc[] = {0, 8, 9, 10, 11, 15};
  if (index >= 6 || brs > 1 || !out)
    return MC_INVALID;
  struct mc_frame f = {0};
  f.id = (uint16_t)(0x300 + index);
  f.dlc = (uint8_t)dlc[index];
  f.brs = (uint8_t)brs;
  unsigned n = lengths[f.dlc];
  for (unsigned j = 0; j < n; j++)
    f.data[j] = (uint8_t)((index * 37 + j * 13) ^ 0xa5);
  if (n) {
    f.data[0] = (uint8_t)index;
    uint8_t sum = 0;
    for (unsigned j = 0; j + 1 < n; j++)
      sum ^= f.data[j];
    f.data[n - 1] = sum;
  }
  *out = f;
  return MC_OK;
}
int mc_bench_match(unsigned index, unsigned brs, const struct mc_frame *got) {
  if (MC_MUTANT == 8)
    return MC_OK;
  struct mc_frame f;
  if (!frame(got) || mc_bench_frame(index, brs, &f))
    return MC_INVALID;
  return f.id == got->id && f.dlc == got->dlc && f.brs == got->brs &&
                 !memcmp(f.data, got->data, lengths[f.dlc])
             ? MC_OK
             : MC_FAILED;
}
