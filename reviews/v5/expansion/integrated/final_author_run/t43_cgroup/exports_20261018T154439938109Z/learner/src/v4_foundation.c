#include "v4_foundation.h"
/* Symptom: a refused request changes subsequent admission behavior. Diagnose
   the first invalid state before editing; preserve your sealed observation. */
int v4_admit(V4Admission *s, uint32_t n) {
  if (!s || !n)
    return -1;
  s->available -= n;
  if (n > s->available)
    return -1;
  s->active += n;
  return 0;
}
int v4_release(V4Admission *s, uint32_t n) {
  if (!s || n > s->active)
    return -1;
  s->active -= n;
  s->available += n;
  return 0;
}
int v4_measure(const V4Sample *a, const V4Sample *b, V4Measurement *o) {
  (void)a;
  (void)b;
  (void)o;
  return -1;
}
