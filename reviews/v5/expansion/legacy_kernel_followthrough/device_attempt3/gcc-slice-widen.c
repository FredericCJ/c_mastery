#include "device.h"
#include <string.h>
static uint32_t le32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
         (uint32_t)p[3] << 24;
}
int kf_pci(const uint8_t c[256], const KfBar bars[6], KfPci *out) {
  if (!c || !bars || !out || !(c[6] & 16))
    return 0;
  KfPci p = {0};
  uint8_t visited[256] = {0};
  unsigned kinds = 0, cap = c[0x34];
  while (cap) {
    if (cap < 64 || cap > 252 || (cap & 3) || visited[cap])
      return 0;
    visited[cap] = 1;
    ++p.visited;
    unsigned next = c[cap + 1];
    if (c[cap] == 9) {
      unsigned len = c[cap + 2], kind = c[cap + 3];
      if (len < 4 || len > 256 - cap)
        return 0;
      if (kind >= 1 && kind <= 4) {
        if ((kinds & (1u << kind)) || len < 16)
          return 0;
        kinds |= 1u << kind;
        unsigned bar = c[cap + 4];
        if (bar >= 6 || !bars[bar].memory)
          return 0;
        uint64_t off = le32(c + cap + 8), size = le32(c + cap + 12),
                 base = bars[bar].base, extent = bars[bar].size;
        if (!size || off > extent || size > extent - off ||
            base > UINT64_MAX - off || base + off > UINT64_MAX - size)
          return 0;
        uint64_t address = base + off;
        if (kind == 1) {
          if (size < 56 || address % 4)
            return 0;
          p.common = address;
        }
        if (kind == 2) {
          if (len < 20 || address % 2)
            return 0;
          p.notify = address;
          p.notify_size = (uint32_t)size;
          p.multiplier = le32(c + cap + 16);
        }
        if (kind == 3 && size < 1)
          return 0;
        if (kind == 4) {
          if (size < 8 || address % 4)
            return 0;
          p.device = address;
        }
      }
    }
    cap = next;
  }
  if ((kinds & 22u) != 22u)
    return 0;
  *out = p;
  return 1;
}
int kf_negotiate(uint64_t features, unsigned queue_size, uint16_t offset,
                 const KfPci *p, uint64_t *out) {
  if (!p || !out ||
      (features & ((UINT64_C(1) << 32) | (1u << 9))) !=
          ((UINT64_C(1) << 32) | (1u << 9)) ||
      queue_size < 8)
    return 0;
  uint64_t relative = (uint64_t)offset * p->multiplier;
  if (p->notify_size < 2 || relative > p->notify_size - 2 ||
      p->notify > UINT64_MAX - relative || (p->notify + relative) % 2)
    return 0;
  *out = p->notify + relative;
  return 1;
}
void kf_queue_init(KfQueue *q, uint16_t index) {
  memset(q, 0, sizeof *q);
  q->available = q->used = index;
  for (unsigned i = 0; i < 2; ++i)
    q->requests[i].generation = 1;
}
int kf_submit(KfQueue *q, unsigned type, uint64_t sector, const uint8_t *bytes,
              KfTicket *out) {
  if (!q || !out || q->poisoned || (type != 0 && type != 1 && type != 4) ||
      (type == 1 && !bytes))
    return 0;
  /* A flush is serialized after all writes complete, never races a new write.
   */
  for (unsigned i = 0; i < 2; ++i)
    if (q->requests[i].state == KF_SUBMITTED &&
        (type == 4 || q->requests[i].header.type == 4))
      return 0;
  unsigned i;
  for (i = 0; i < 2 && q->requests[i].state != KF_FREE; ++i) {
  }
  if (i == 2)
    return 0;
  KfRequest *r = &q->requests[i];
  r->header = (KfHeader){type, 0, type == 4 ? 0 : sector};
  r->status = 255;
  if (type == 1)
    memcpy(r->bytes, bytes, 4096);
  else
    memset(r->bytes, 0, 4096);
  unsigned head = i * 3;
  q->descriptors[head] = (KfDescriptor){(uintptr_t)&r->header, 16, 1,
                                        (uint16_t)(head + (type == 4 ? 2 : 1))};
  q->descriptors[head + 1] =
      (KfDescriptor){(uintptr_t)r->bytes, 4096,
                     (uint16_t)(1 | (type == 0 ? 2 : 0)), (uint16_t)(head + 2)};
  q->descriptors[head + 2] = (KfDescriptor){(uintptr_t)&r->status, 1, 2, 0};
  r->state = KF_SUBMITTED;
  q->ring[q->available % 8] = (uint16_t)head;
  q->available = (uint16_t)(q->available + 1);
  *out = (KfTicket){i, r->generation};
  return 1;
}
int kf_complete(KfQueue *q, uint16_t index, uint32_t head, uint8_t status) {
  if (!q || index != (uint16_t)(q->used + 1) || (head != 0 && head != 3))
    return 0;
  KfRequest *r = &q->requests[head / 3];
  if (r->state != KF_SUBMITTED && r->state != KF_UNKNOWN)
    return 0;
  q->used = index;
  r->status = status;
  /* Late completion is observed, but timeout poisoning and buffer retention
   * remain until a separately acknowledged reset. */
  if (r->state == KF_UNKNOWN)
    return 1;
  r->state = status == 0 ? KF_SUCCESS : KF_DEVICE_ERROR;
  if (!status && r->header.type == 1)
    ++q->completed_writes;
  if (!status && r->header.type == 4)
    q->stable_writes = q->completed_writes;
  return 1;
}
static KfRequest *ticket(KfQueue *q, KfTicket t) {
  return q && t.slot < 2 && t.generation == q->requests[t.slot].generation
             ? &q->requests[t.slot]
             : 0;
}
int kf_timeout(KfQueue *q, KfTicket t) {
  KfRequest *r = ticket(q, t);
  if (!r || r->state != KF_SUBMITTED)
    return 0;
  r->state = KF_UNKNOWN;
  q->poisoned = 1;
  return 1;
}
int kf_release(KfQueue *q, KfTicket t) {
  KfRequest *r = ticket(q, t);
  if (!r || q->poisoned ||
      (r->state != KF_SUCCESS && r->state != KF_DEVICE_ERROR) ||
      r->generation == UINT32_MAX)
    return 0;
  ++r->generation;
  r->state = KF_FREE;
  return 1;
}
int kf_reset(KfQueue *q, int ack) {
  if (!q || ack != 1)
    return 0;
  for (unsigned i = 0; i < 2; ++i)
    if (q->requests[i].generation == UINT32_MAX)
      return 0;
  uint32_t a = q->requests[0].generation + 1, b = q->requests[1].generation + 1;
  kf_queue_init(q, 0);
  q->requests[0].generation = a;
  q->requests[1].generation = b;
  return 1;
}
int kf_page_sector(uint64_t raw, unsigned bytes, uint32_t page, uint64_t *out) {
  if (!out || bytes != 512 || raw < 8 || page >= raw / 8)
    return 0;
  *out = (uint64_t)page * 8;
  return 1;
}
int kf_slice(uint64_t raw, uint32_t base, uint32_t count, uint32_t logical,
             uint32_t *out) {
  if (!out || !count || base > raw || count > raw - base || 0 ||
      base > UINT32_MAX - logical)
    return 0;
  *out = base + logical;
  return 1;
}
