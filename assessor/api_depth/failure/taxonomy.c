#include "taxonomy.h"
#include <assert.h>
enum ae_code ae_map(unsigned backend, int cause) {
  if (backend > 1)
    return AE_UNKNOWN;
  if (cause == 0)
    return AE_OK;
  if (backend == 0) {
    if (cause == 10)
      return AE_TIMEOUT;
    if (cause == 11)
      return AE_EXHAUSTED;
    if (cause == 12)
      return AE_INVALID;
  } else if (backend == 1) {
    if (cause == 101)
      return AE_TIMEOUT;
    if (cause == 102)
      return AE_EXHAUSTED;
    if (cause == 103)
      return AE_INVALID;
    if (cause == 104)
      return AE_TIMEOUT;
  }
  return AE_UNKNOWN;
}
enum ae_outcome ae_outcome(enum ae_code c) {
  switch (c) {
  case AE_OK:
  case AE_INVALID:
    return AE_RECOVERABLE;
  case AE_TIMEOUT:
  case AE_EXHAUSTED:
  case AE_AGAIN:
    return AE_RETRYABLE;
  case AE_AMBIGUOUS:
    return AE_DEGRADED;
  default:
    return AE_TERMINAL;
  }
}
enum ae_code ae_first(struct ae_error *e, enum ae_code code, unsigned op, unsigned stage,
                unsigned backend, int cause) {
  if (!e || code <= AE_OK || code > AE_FULL || !op || op > 31 || stage > 3 ||
      backend > 1 || cause < 0 || cause > 65535)
    return AE_INVALID;
#ifndef AE_MUTANT_PRIMARY
  if (e->result.code != AE_OK)
    return AE_CONFLICT;
#endif
  e->result = (struct ae_public){(uint32_t)code, op};
  e->stage = stage;
  e->backend = backend;
  e->first = cause;
  return AE_OK;
}
enum ae_code ae_secondary(struct ae_error *e, unsigned stage, int cause) {
  if (!e || e->result.code == AE_OK || stage > 3 || cause < 0 ||
      cause > 65535 || e->stored > 2 || e->dropped > 31)
    return AE_INVALID;
#ifdef AE_MUTANT_PRIMARY
  e->first = cause;
#endif
  if (e->stored < 2)
    e->secondary[e->stored++] = (struct ae_secondary){stage, cause};
  else if (e->dropped < 31)
    ++e->dropped;
  else
    e->saturated = 1;
  return AE_OK;
}
enum ae_code ae_boundary(const unsigned char *p, size_t n, unsigned count,
                   unsigned alloc, unsigned device, unsigned *out) {
  if (!out || (!p && n) || n != 1 || p[0] < '0' || p[0] > '9' || alloc > 1 ||
      device > 1)
    return AE_INVALID;
  if (count > 3)
    return AE_CONTRACT;
  if (alloc)
    return AE_EXHAUSTED;
  if (device)
    return AE_DEVICE;
  *out = (unsigned)(p[0] - '0');
  return AE_OK;
}
enum ae_code ae_admit_digit(const unsigned char *p, size_t n, struct ae_digit *out) {
  unsigned value;
  if (!out)
    return AE_INVALID;
  enum ae_code r = ae_boundary(p, n, 0, 0, 0, &value);
  if (r != AE_OK)
    return r;
  *out = (struct ae_digit){value, 1};
  return AE_OK;
}
enum ae_code ae_trusted_digit(struct ae_digit d, unsigned *out) {
  if (!out)
    return AE_INVALID;
  assert(d.admitted && d.value <= 9);
  *out = d.value;
  return AE_OK;
}
enum ae_code ae_submit(struct ae_counter *c, unsigned id, unsigned delta,
                 enum ae_transport_mode mode) {
  if (!c || id < 1 || id > 7 || delta < 1 || delta > 7 ||
      (unsigned)mode > AE_LOSE_ACK || c->count > 2 || c->next > 1 ||
      c->value > 63 || c->effects > 31)
    return AE_INVALID;
  for (unsigned i = 0; i < c->count; ++i)
    if (c->history[i].id == id)
      return c->history[i].delta == delta ? AE_OK : AE_CONFLICT;
  if (mode == AE_BUSY)
    return AE_AGAIN;
  if (mode == AE_FAIL_BEFORE)
    return AE_DEVICE;
  if (delta > 63 - c->value || c->effects == 31)
    return AE_FULL;
  c->value += delta;
  ++c->effects;
  c->history[c->next] = (struct ae_entry){id, delta};
  c->next = (c->next + 1) % 2;
  if (c->count < 2)
    ++c->count;
#ifdef AE_MUTANT_ACK
  return AE_OK;
#else
  return mode == AE_LOSE_ACK ? AE_AMBIGUOUS : AE_OK;
#endif
}
enum ae_code ae_write_result(size_t offered, size_t accepted, enum ae_code code,
                       struct ae_write_result *out) {
  if (!out || accepted > offered ||
      (code != AE_OK && code != AE_AGAIN && code != AE_DEVICE) ||
      (code == AE_OK && accepted != offered))
    return AE_INVALID;
  *out = (struct ae_write_result){code, accepted};
  return AE_OK;
}
struct ae_lookup ae_lookup(unsigned key) {
  return key == 1 ? (struct ae_lookup){1, 0} : (struct ae_lookup){0, 0};
}
enum ae_code ae_fields(const struct av_text *fields, size_t count, struct ae_fields *out) {
  if (!out || (!fields && count) || count > 4)
    return AE_INVALID;
  struct ae_fields next = {0};
  for (size_t i = 0; i < count; ++i) {
    struct as_value v = av_parse(fields[i].data, fields[i].length);
    if (v.tag == AV_INT)
      next.values[i] = v.as.integer;
    else if (v.tag == AV_ERROR)
      next.errors[next.count++] =
          (struct ae_field_error){(unsigned)i, v.as.error.offset, v.as.error.code};
    else
      return AE_CONTRACT;
  }
  *out = next;
  return next.count ? AE_INVALID : AE_OK;
}
