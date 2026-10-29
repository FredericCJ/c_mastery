#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
static int copy(struct nd_view a, const double *b, size_t n, double m[3][4],
                double scale[3]) {
  if (nd_view(a) || !b || n < 2 || n > 3 || a.rows != n || a.cols != n)
    return 0;
  for (size_t i = 0; i < n; i++) {
    scale[i] = 0;
    for (size_t j = 0; j < n; j++) {
      double x = a.data[i * a.rs + j * a.cs];
      if (!isfinite(x) || fabs(x) > 1e100)
        return 0;
      m[i][j] = x;
      if (fabs(x) > scale[i])
        scale[i] = fabs(x);
    }
    if (!isfinite(b[i]) || fabs(b[i]) > 1e100)
      return 0;
    m[i][n] = b[i];
  }
  return 1;
}
int nd_solve(struct nd_view a, const double *b, size_t n, double tolerance, double *out,
             struct nd_cost *cost) {
  double m[3][4], s[3], x[3];
  struct nd_cost c = {0};
  if (!out || !cost || !isfinite(tolerance) || tolerance < 0 ||
      tolerance >= 1 || !copy(a, b, n, m, s))
    return ND_BAD;
  for (size_t k = 0; k < n; k++) {
    size_t p = k;
    for (size_t i = k + 1; i < n; i++)
      if (s[i] && (!s[p] || fabs(m[i][k]) / s[i] > fabs(m[p][k]) / s[p]))
        p = i;
    if (ND_MUTANT == 5)
      p = k;
    if (!s[p] || fabs(m[p][k]) <= tolerance * s[p])
      return ND_SINGULAR;
    if (p != k) {
      for (size_t j = 0; j <= n; j++) {
        double t = m[p][j];
        m[p][j] = m[k][j];
        m[k][j] = t;
      }
      double t = s[p];
      s[p] = s[k];
      s[k] = t;
      c.row_moves++;
    }
    for (size_t i = k + 1; i < n; i++) {
      double f = m[i][k] / m[k][k];
      c.divisions++;
      m[i][k] = 0;
      for (size_t j = k + 1; j <= n; j++) {
        m[i][j] -= f * m[k][j];
        c.multiplies++;
        c.adds++;
        if (!isfinite(m[i][j]))
          return ND_RANGE;
      }
    }
  }
  for (size_t z = n; z; z--) {
    size_t i = z - 1;
    double t = m[i][n];
    for (size_t j = i + 1; j < n; j++) {
      t -= m[i][j] * x[j];
      c.multiplies++;
      c.adds++;
    }
    x[i] = t / m[i][i];
    c.divisions++;
    if (!isfinite(x[i]))
      return ND_RANGE;
  }
  for (size_t i = 0; i < n; i++)
    out[i] = x[i];
  *cost = c;
  return ND_OK;
}
int nd_jacobi(struct nd_view a, const double *b, size_t n, double tol, unsigned limit,
              double *out, unsigned *steps) {
  double m[3][4], s[3], x[3] = {0}, next[3];
  if (!out || !steps || !isfinite(tol) || tol < 0 || limit > 10000 ||
      !copy(a, b, n, m, s))
    return ND_BAD;
  for (size_t i = 0; i < n; i++)
    if (m[i][i] == 0)
      return ND_SINGULAR;
  for (unsigned k = 0; k < limit; k++) {
    int unchanged = 1;
    for (size_t i = 0; i < n; i++) {
      double t = m[i][n];
      for (size_t j = 0; j < n; j++)
        if (i != j)
          t -= m[i][j] * x[j];
      next[i] = t / m[i][i];
      if (!isfinite(next[i]))
        return ND_RANGE;
      if (next[i] != x[i])
        unchanged = 0;
    }
    double residual = 0;
    for (size_t i = 0; i < n; i++) {
      double r = -m[i][n];
      for (size_t j = 0; j < n; j++)
        r += m[i][j] * next[j];
      if (!isfinite(r))
        return ND_RANGE;
      if (fabs(r) > residual)
        residual = fabs(r);
    }
    if (residual <= tol) {
      for (size_t i = 0; i < n; i++)
        out[i] = next[i];
      *steps = k + 1;
      return ND_OK;
    }
    if (unchanged)
      return ND_STAGNANT;
    for (size_t i = 0; i < n; i++)
      x[i] = next[i];
  }
  return ND_LIMIT;
}
