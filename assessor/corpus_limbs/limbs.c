#include "limbs.h"
#ifndef BL_MUTANT
#define BL_MUTANT 0
#endif
static int view(BlView a) {
  if (a.used && (!a.digits || a.digits[a.used - 1] == 0))
    return 0;
#if BL_BITS == 8
  for (size_t i = 0; i < a.used; i++)
    if (a.digits[i] > BL_MASK)
      return 0;
#endif
  return 1;
}
static int output(const BlOutput *o) {
  return o && o->used <= o->capacity && (!o->capacity || o->digits);
}
static size_t trim(const uint16_t *a, size_t n) {
  while (n && !a[n - 1])
    n--;
  return n;
}
static int compare(const uint16_t *a, size_t an, const uint16_t *b, size_t bn) {
  if (an != bn)
    return an > bn ? 1 : -1;
  while (an) {
    an--;
    if (a[an] != b[an])
      return a[an] > b[an] ? 1 : -1;
  }
  return 0;
}
static void publish(BlOutput *o, const uint16_t *s, size_t n) {
  for (size_t i = 0; i < n; i++)
    o->digits[i] = s[i];
  o->used = n;
}
static BlStatus stage(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t sn,
                      size_t need) {
  if (!view(a) || !view(b) || !output(o))
    return BL_INVALID;
  if (sn < need)
    return BL_FULL;
  if (need && !s)
    return BL_INVALID;
  return BL_OK;
}
BlStatus bl_add(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t sn) {
  size_t n = a.used > b.used ? a.used : b.used;
  if (n == SIZE_MAX)
    return BL_FULL;
  BlStatus r = stage(a, b, o, s, sn, n + 1);
  if (r != BL_OK)
    return r;
  uint32_t carry = 0;
  for (size_t i = 0; i < n; i++) {
    uint32_t t = (i < a.used ? a.digits[i] : 0) +
                 (uint32_t)(i < b.used ? b.digits[i] : 0) + carry;
    s[i] = (uint16_t)(t & BL_MASK);
    carry = BL_MUTANT == 1 ? 0 : t >> BL_BITS;
  }
  s[n] = (uint16_t)carry;
  n = trim(s, n + 1);
  if (n > o->capacity)
    return BL_FULL;
  publish(o, s, n);
  return BL_OK;
}
static void subtract(uint16_t *out, const uint16_t *a, size_t an,
                     const uint16_t *b, size_t bn) {
  uint32_t borrow = 0;
  for (size_t i = 0; i < an; i++) {
    uint32_t x = a[i], y = (i < bn ? b[i] : 0) + (uint32_t)borrow;
    out[i] = (uint16_t)((x - y) & BL_MASK);
    borrow = BL_MUTANT == 2 ? 0 : (x < y);
  }
}
BlStatus bl_sub(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t sn) {
  BlStatus r = stage(a, b, o, s, sn, a.used);
  if (r != BL_OK)
    return r;
  if (compare(a.digits, a.used, b.digits, b.used) < 0)
    return BL_NEGATIVE;
  subtract(s, a.digits, a.used, b.digits, b.used);
  size_t n = trim(s, a.used);
  if (n > o->capacity)
    return BL_FULL;
  publish(o, s, n);
  return BL_OK;
}
BlStatus bl_mul(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t sn) {
  if (a.used > SIZE_MAX - b.used)
    return BL_FULL;
  size_t n = a.used + b.used;
  BlStatus r = stage(a, b, o, s, sn, n);
  if (r != BL_OK)
    return r;
  for (size_t i = 0; i < n; i++)
    s[i] = 0;
  for (size_t i = 0; i < a.used; i++) {
    uint32_t carry = 0;
    for (size_t j = 0; j < b.used; j++) {
      uint32_t t = (uint32_t)a.digits[i] * b.digits[j] + s[i + j] + carry;
      s[i + j] = (uint16_t)(t & BL_MASK);
      carry = t >> BL_BITS;
    }
    if (BL_MUTANT != 3)
      s[i + b.used] = (uint16_t)carry;
  }
  n = trim(s, n);
  if (n > o->capacity)
    return BL_FULL;
  publish(o, s, n);
  return BL_OK;
}
BlStatus bl_div(BlView a, BlView b, BlOutput *q, BlOutput *r, uint16_t *s,
                size_t sn) {
  if (!view(a) || !view(b) || !output(q) || !output(r) || q == r)
    return BL_INVALID;
  if (!b.used)
    return BL_ZERO_DIVISOR;
  if (a.used > SIZE_MAX - b.used || a.used + b.used == SIZE_MAX)
    return BL_FULL;
  size_t need = a.used + b.used + 1;
  if (sn < need)
    return BL_FULL;
  if (!s)
    return BL_INVALID;
  uint16_t *rem = s + a.used;
  size_t rn = 0;
  for (size_t i = 0; i < need; i++)
    s[i] = 0;
  size_t limb = a.used;
  while (limb) {
    limb--;
    unsigned bit = BL_BITS;
    while (bit) {
      bit--;
      uint32_t carry = (a.digits[limb] >> bit) & 1u;
      for (size_t j = 0; j < rn; j++) {
        uint32_t t = ((uint32_t)rem[j] << 1) | carry;
        rem[j] = (uint16_t)(t & BL_MASK);
        carry = t >> BL_BITS;
      }
      if (carry)
        rem[rn++] = (uint16_t)carry;
      if (compare(rem, rn, b.digits, b.used) >= (BL_MUTANT == 4 ? 1 : 0)) {
        subtract(rem, rem, rn, b.digits, b.used);
        rn = trim(rem, rn);
        s[limb] = (uint16_t)(s[limb] | (uint16_t)(UINT32_C(1) << bit));
      }
    }
  }
  size_t qn = trim(s, a.used);
  if (qn > q->capacity || rn > r->capacity)
    return BL_FULL;
  publish(q, s, qn);
  publish(r, rem, rn);
  return BL_OK;
}
static int signed_view(BlSigned a) {
  return view(a.magnitude) &&
         (a.magnitude.used ? (a.sign == 1 || a.sign == -1) : a.sign == 0);
}
BlStatus bl_signed_add(BlSigned a, BlSigned b, BlSignedOutput *o, uint16_t *s,
                       size_t sn) {
  if (!o || !signed_view(a) || !signed_view(b))
    return BL_INVALID;
  int sign;
  BlStatus status;
  if (a.sign == b.sign || !a.sign || !b.sign) {
    sign = a.sign ? a.sign : b.sign;
    status = bl_add(a.magnitude, b.magnitude, &o->magnitude, s, sn);
  } else if (compare(a.magnitude.digits, a.magnitude.used, b.magnitude.digits,
                     b.magnitude.used) >= 0) {
    sign = a.sign;
    status = bl_sub(a.magnitude, b.magnitude, &o->magnitude, s, sn);
  } else {
    sign = b.sign;
    status = bl_sub(b.magnitude, a.magnitude, &o->magnitude, s, sn);
  }
  if (status == BL_OK)
    o->sign = o->magnitude.used ? sign : 0;
  return status;
}
BlStatus bl_signed_div(BlSigned a, BlSigned b, BlSignedOutput *q,
                       BlSignedOutput *r, uint16_t *s, size_t sn) {
  if (!q || !r || q == r || !signed_view(a) || !signed_view(b))
    return BL_INVALID;
  BlStatus status =
      bl_div(a.magnitude, b.magnitude, &q->magnitude, &r->magnitude, s, sn);
  if (status == BL_OK) {
    q->sign = q->magnitude.used ? a.sign * b.sign : 0;
    r->sign = r->magnitude.used ? (BL_MUTANT == 5 ? b.sign : a.sign) : 0;
  }
  return status;
}
BlStatus bl_owner_init(BlOwner *o, void *ctx, BlAllocate a, BlFree f) {
  if (!o || !a || !f)
    return BL_INVALID;
  o->digits = NULL;
  o->used = 0;
  o->capacity = 0;
  o->context = ctx;
  o->allocate = a;
  o->release = f;
  return BL_OK;
}
BlStatus bl_owner_set(BlOwner *o, BlView v, size_t reserve) {
  if (!o || !o->allocate || !o->release || !view(v) || reserve < v.used)
    return BL_INVALID;
  if (reserve > SIZE_MAX / sizeof(uint16_t))
    return BL_FULL;
  uint16_t *next = NULL;
  if (reserve) {
    next = o->allocate(o->context, reserve);
    if (!next)
      return BL_ALLOCATION;
  }
  for (size_t i = 0; i < v.used; i++)
    next[i] = v.digits[i];
  uint16_t *old = o->digits;
  size_t capacity = o->capacity;
  o->digits = next;
  o->used = v.used;
  o->capacity = reserve;
  if (old && BL_MUTANT != 6)
    o->release(o->context, old, capacity);
  return BL_OK;
}
void bl_owner_drop(BlOwner *o) {
  if (!o)
    return;
  uint16_t *old = o->digits;
  size_t capacity = o->capacity;
  o->digits = NULL;
  o->used = 0;
  o->capacity = 0;
  if (old)
    o->release(o->context, old, capacity);
}
