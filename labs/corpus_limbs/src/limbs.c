#include "limbs.h"
BlStatus bl_add(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t n) {
  (void)a;
  (void)b;
  (void)o;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_sub(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t n) {
  (void)a;
  (void)b;
  (void)o;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_mul(BlView a, BlView b, BlOutput *o, uint16_t *s, size_t n) {
  (void)a;
  (void)b;
  (void)o;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_div(BlView a, BlView b, BlOutput *q, BlOutput *r, uint16_t *s,
                size_t n) {
  (void)a;
  (void)b;
  (void)q;
  (void)r;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_signed_add(BlSigned a, BlSigned b, BlSignedOutput *o, uint16_t *s,
                       size_t n) {
  (void)a;
  (void)b;
  (void)o;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_signed_div(BlSigned a, BlSigned b, BlSignedOutput *q,
                       BlSignedOutput *r, uint16_t *s, size_t n) {
  (void)a;
  (void)b;
  (void)q;
  (void)r;
  (void)s;
  (void)n;
  return BL_INVALID;
}
BlStatus bl_owner_init(BlOwner *o, void *c, BlAllocate a, BlFree f) {
  (void)o;
  (void)c;
  (void)a;
  (void)f;
  return BL_INVALID;
}
BlStatus bl_owner_set(BlOwner *o, BlView v, size_t n) {
  (void)o;
  (void)v;
  (void)n;
  return BL_INVALID;
}
void bl_owner_drop(BlOwner *o) { (void)o; }
