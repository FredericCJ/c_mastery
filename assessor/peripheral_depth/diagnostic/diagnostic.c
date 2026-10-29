#include "diagnostic.h"
#include "foundation.h"
#include <limits.h>
#ifndef DG_MUTANT
#define DG_MUTANT 0
#endif
_Static_assert(CHAR_BIT == 8, "wire octets required");
static uint16_t read16(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] * 256u + p[1]);
}
int dg_encode(const struct dg_message *m, uint8_t *out, size_t cap, size_t *written) {
  if (!m || !out || !written || m->length > 32 || !m->sequence)
    return DG_INVALID;
  size_t n = (size_t)m->length + 4;
  if (cap < n)
    return DG_FULL;
  uint8_t stage[36];
  stage[0] = (uint8_t)(m->sequence >> 8);
  stage[1] = (uint8_t)m->sequence;
  stage[2] = 0;
  stage[3] = (uint8_t)m->length;
  for (size_t i = 0; i < m->length; ++i)
    stage[i + 4] = m->payload[i];
  for (size_t i = 0; i < n; ++i)
    out[i] = stage[i];
  *written = n;
  return DG_OK;
}
int dg_decode(const uint8_t *wire, size_t n, struct dg_message *out) {
  if (!out || !wire)
    return DG_INVALID;
  struct f_cursor c = {wire, n, 0};
  const uint8_t *h, *p;
  if (f_take(&c, 4, &h) != F_OK)
    return DG_FRAME;
  uint16_t seq = read16(h), length = read16(h + 2);
  if (!seq || length > 32 || f_take(&c, length, &p) != F_OK)
    return DG_FRAME;
  if (DG_MUTANT != 1 && c.position != c.length)
    return DG_FRAME;
  struct dg_message m = {seq, length, {0}};
  for (size_t i = 0; i < length; ++i)
    m.payload[i] = p[i];
  *out = m;
  return DG_OK;
}
int dg_accept(struct dg_session *s, const uint8_t *wire, size_t n, struct dg_message *out) {
  if (!s || !out || s->accepted > 65535u)
    return DG_INVALID;
  struct dg_message m;
  int rc = dg_decode(wire, n, &m);
  if (rc)
    return rc;
  if (DG_MUTANT != 2 && m.sequence <= s->last)
    return DG_STALE;
  if (s->accepted == 65535u)
    return DG_FULL;
  s->last = m.sequence;
  ++s->accepted;
  *out = m;
  return DG_OK;
}
int dg_stream_feed(struct dg_stream *s, const uint8_t *p, size_t n, size_t *used) {
  if (!s || !used || (n && !p) || s->used > 38 || s->needed > 38)
    return DG_INVALID;
  *used = 0;
  if (s->poisoned)
    return DG_FRAME;
  if (s->ready)
    return DG_BUSY;
  while (*used < n) {
    s->bytes[s->used++] = p[(*used)++];
    if (s->used == 2) {
      size_t length = read16(s->bytes);
      if (length < 4 || length > 36) {
        s->poisoned = 1;
        return DG_FRAME;
      }
      s->needed = length + 2;
    }
    if (s->needed && s->used == s->needed) {
      s->ready = 1;
      break;
    }
  }
  if (DG_MUTANT == 3 && s->used > 2)
    s->ready = 1;
  return DG_OK;
}
int dg_stream_take(struct dg_stream *s, struct dg_session *session, struct dg_message *out) {
  if (!s || !session || !out)
    return DG_INVALID;
  if (s->poisoned)
    return DG_FRAME;
  if (!s->ready)
    return DG_BUSY;
  int rc = dg_accept(session, s->bytes + 2, s->used - 2, out);
  /* A valid outer length isolates even a rejected inner message. */
  s->used = s->needed = 0;
  s->ready = 0;
  return rc;
}
int dg_stream_end(const struct dg_stream *s) {
  if (!s)
    return DG_INVALID;
  if (s->poisoned || (s->used && !s->ready))
    return DG_FRAME;
  return s->ready ? DG_BUSY : DG_END;
}
