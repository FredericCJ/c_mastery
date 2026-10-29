#include "unicode_stream.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8,
               "UTF-8 laboratory octets require eight-bit bytes");
#ifndef ADU_MUTATION
#define ADU_MUTATION 0
#endif
void adu_init(struct adu_stream *s) { *s = (struct adu_stream){{0, 0, 0}, 0, 0, 0, ADU_OPEN}; }
static unsigned width(unsigned char first) {
  if (first < 0x80)
    return 1;
  if (first >= 0xc2 && first <= 0xdf)
    return 2;
  if (first >= 0xe0 && first <= 0xef)
    return 3;
  if (first >= 0xf0 && first <= 0xf4)
    return 4;
  return 0;
}
static int byte(struct adu_stream *s, unsigned char c) {
  if (!s->used) {
    unsigned need = width(c);
    if (!need)
      return 0;
    if (need == 1) {
      ++s->scalars;
      return 1;
    }
    s->pending[0] = c;
    s->used = 1;
    return 1;
  }
  if (c < 0x80 || c > 0xbf)
    return 0;
  unsigned char lead = s->pending[0];
  if (s->used == 1 &&
      ((lead == 0xe0 && c < 0xa0) || (lead == 0xed && c > 0x9f) ||
       (lead == 0xf0 && c < 0x90) || (lead == 0xf4 && c > 0x8f)))
    return 0;
  unsigned need = width(lead);
  if (s->used + 1 == need) {
    unsigned char complete[4];
    struct ad_text admitted;
    for (unsigned i = 0; i < s->used; ++i)
      complete[i] = s->pending[i];
    complete[s->used] = c;
    if (!ad_text(complete, need, &admitted))
      return 0;
    ++s->scalars;
    s->used = 0;
    for (unsigned i = 0; i < 3; ++i)
      s->pending[i] = 0;
  } else
    s->pending[s->used++] = c;
  return 1;
}
enum adu_status adu_feed(struct adu_stream *s, const unsigned char *p, size_t n) {
  if (!s || (n && !p))
    return ADU_INVALID;
  if (s->phase != ADU_OPEN)
    return ADU_STATE;
  if (n > 65535 - s->bytes)
    return ADU_RANGE;
  struct adu_stream next = *s;
  if (ADU_MUTATION == 1)
    next.used = 0;
  for (size_t i = 0; i < n; ++i) {
    if (!byte(&next, p[i]))
      return ADU_UTF8;
    ++next.bytes;
    if (ADU_MUTATION == 4)
      *s = next;
  }
  *s = next;
  return ADU_OK;
}
enum adu_status adu_finish(struct adu_stream *s, struct adu_summary *out) {
  if (!s || !out)
    return ADU_INVALID;
  if (s->phase == ADU_CANCELLED)
    return ADU_STATE;
  if (s->used && ADU_MUTATION != 2)
    return ADU_UTF8;
  *out = (struct adu_summary){s->bytes, s->scalars};
  s->phase = ADU_FINISHED;
  return ADU_OK;
}
enum adu_status adu_cancel(struct adu_stream *s) {
  if (!s)
    return ADU_INVALID;
  if (ADU_MUTATION != 3)
    adu_init(s);
  s->phase = ADU_CANCELLED;
  return ADU_OK;
}
void adu_owner_init(struct adu_owner *o) {
  ado_init(&o->value);
  o->scalars = 0;
}
enum adu_status adu_owner_set(struct adu_owner *o, const unsigned char *p, size_t n,
                        struct ado_ops ops) {
  if (!o || (n && !p) || !ops.clone || !ops.drop)
    return ADU_INVALID;
  if (n > 65535)
    return ADU_RANGE;
  struct ad_text admitted = {p, n, n};
  if (ADU_MUTATION != 6 && !ad_text(p, n, &admitted))
    return ADU_UTF8;
  enum ado_status status = ado_text(&o->value, p, n, ops);
  if (status != ADO_OK)
    return status == ADO_NOMEM ? ADU_NOMEM : ADU_INVALID;
  o->scalars = ADU_MUTATION == 5 ? n : admitted.scalars;
  return ADU_OK;
}
enum adu_status adu_owner_view(const struct adu_owner *o, struct ad_text *out) {
  if (!o || !out)
    return ADU_INVALID;
  if (o->value.tag == ADO_EMPTY)
    *out = (struct ad_text){NULL, 0, 0};
  else
    *out = (struct ad_text){o->value.data.text.bytes, o->value.data.text.length,
                    o->scalars};
  return ADU_OK;
}
void adu_owner_drop(struct adu_owner *o) {
  ado_drop(&o->value);
  o->scalars = 0;
}
