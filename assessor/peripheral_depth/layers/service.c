#include "service.h"
#ifndef PL_MUTANT
#define PL_MUTANT 0
#endif
static uint8_t crc(uint8_t a, uint8_t b) {
  uint8_t c = 0;
  for (unsigned j = 0; j < 2; j++) {
    c ^= j ? b : a;
    for (unsigned k = 0; k < 8; k++)
      c = (uint8_t)((c << 1) ^ ((c & 128) ? 7 : 0));
  }
  return c;
}
int pl_encode(uint8_t seq, uint8_t value, uint8_t out[8]) {
  if (!out)
    return PL_INVALID;
  uint8_t raw[3] = {seq, value, crc(seq, value)};
  out[0] = 0xf0;
  out[7] = 0xf1;
  for (unsigned k = 0; k < 3; k++) {
    out[2 * k + 1] = (uint8_t)(0x10 | (raw[k] >> 4));
    out[2 * k + 2] = (uint8_t)(0x10 | (raw[k] & 15));
  }
  return PL_OK;
}
static int valid(struct pl_transport t) { return t.submit && t.poll && t.cancel; }
int pl_send(struct pl_service *s, struct pl_transport t, uint8_t seq, uint8_t value,
            uint32_t token, uint32_t now, uint32_t budget) {
  if (!s || !valid(t) || !token || !budget || budget >= 0x80000000u)
    return PL_INVALID;
  if (s->active && PL_MUTANT != 1)
    return PL_BUSY;
  if (token <= s->token)
    return PL_STALE;
  uint8_t old[8];
  for (unsigned i = 0; i < 8; i++)
    old[i] = s->bytes[i];
  (void)pl_encode(seq, value, s->bytes);
  int r = t.submit(t.context, s->bytes, 8, token, now, budget);
  if (r != PL_OK) {
    for (unsigned i = 0; i < 8; i++)
      s->bytes[i] = old[i];
    return r;
  }
  s->token = token;
  s->active = 1;
  s->result = PL_BUSY;
  return PL_OK;
}
int pl_cancel(struct pl_service *s, struct pl_transport t, uint32_t token) {
  if (!s || !valid(t))
    return PL_INVALID;
  if (!s->active || token != s->token)
    return PL_STALE;
  int r = t.cancel(t.context, token);
  if (r == PL_OK && PL_MUTANT == 2)
    s->active = 0;
  return r;
}
int pl_poll(struct pl_service *s, struct pl_transport t, uint32_t token, uint32_t now,
            struct pl_result *out) {
  if (!s || !out || !valid(t))
    return PL_INVALID;
  if (!s->active || token != s->token)
    return PL_STALE;
  int r = t.poll(t.context, token, now);
  if (r == PL_BUSY)
    return r;
  if (r != PL_OK && r != PL_CANCELLED && r != PL_IO && r != PL_TIMEOUT)
    return r;
  struct pl_result result = {token, (uint32_t)r, 1};
  *out = result;
  s->result = (uint32_t)r;
  s->active = 0;
  return PL_OK;
}
int pl_feed(struct pl_parser *p, uint8_t b, struct pl_command *out) {
  if (!p || !out)
    return PL_INVALID;
  if (b == 0xf0) {
    p->active = 1;
    p->nibbles = 0;
    return PL_BUSY;
  }
  if (!p->active)
    return PL_BUSY;
  if (b == 0xf1) {
    p->active = 0;
    if (p->nibbles != 6 ||
        (PL_MUTANT != 3 && p->raw[2] != crc(p->raw[0], p->raw[1])))
      return PL_FRAME;
    *out = (struct pl_command){p->raw[0], p->raw[1]};
    return PL_OK;
  }
  if (b < 0x10 || b > 0x1f || p->nibbles >= 6) {
    p->active = 0;
    return PL_FRAME;
  }
  unsigned k = p->nibbles++;
  if (!(k & 1))
    p->raw[k / 2] = (uint8_t)((b & 15) << 4);
  else
    p->raw[k / 2] |= b & 15;
  return PL_BUSY;
}
