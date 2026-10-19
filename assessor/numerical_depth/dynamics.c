#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
int nd_euler(double y, double h, double rate, size_t n, double *out,
             size_t cap) {
  if (!out || n > 256 || cap < n + 1 || !isfinite(y) || !isfinite(h) ||
      !isfinite(rate) || h <= 0 || rate <= 0 || fabs(y) > 1e100 || h > 1e100 ||
      rate > 1e100)
    return ND_BAD;
  double t[257];
  t[0] = y;
  for (size_t i = 0; i < n; i++) {
    y = y + h * (-rate * y);
    if (!isfinite(y))
      return ND_RANGE;
    t[i + 1] = y;
  }
  if (ND_MUTANT == 6)
    t[n] = t[0];
  for (size_t i = 0; i <= n; i++)
    out[i] = t[i];
  return ND_OK;
}
int nd_iir(double *state, const double *in, size_t n, double feed,
           double feedback, double *out, size_t cap) {
  if (!state || (!in && n) || (!out && n) || n > 256 || cap < n ||
      !isfinite(*state) || fabs(*state) > 1e100 || !isfinite(feed) ||
      fabs(feed) > 1 || !isfinite(feedback) || fabs(feedback) >= 1)
    return ND_BAD;
  double t[256], s = *state;
  if (ND_MUTANT == 8)
    s = 0;
  for (size_t i = 0; i < n; i++) {
    if (!isfinite(in[i]) || fabs(in[i]) > 1e100)
      return ND_BAD;
    s = feed * in[i] + feedback * s;
    t[i] = s;
  }
  for (size_t i = 0; i < n; i++)
    out[i] = t[i];
  *state = s;
  return ND_OK;
}
