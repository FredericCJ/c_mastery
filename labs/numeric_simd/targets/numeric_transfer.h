#ifndef CM_NUMERIC_TRANSFER_H
#define CM_NUMERIC_TRANSFER_H
#include <stddef.h>
#include <stdint.h>
/* Exact int16 products, n<=4096, unaligned int16 arrays; mode0 scalar,
 * mode1 baseline x86-64 SSE2 (unsupported other targets reject unchanged). */
int nt_dot(const int16_t *, const int16_t *, size_t, int64_t *, unsigned);
/* Same midpoint tree as nd_pairwise; two workers only; schedule0/1 changes
 * which worker yields, never the mathematical joins. Failed call preserves out. */
int nt_parallel(const double *, size_t, double *, unsigned);
int nt_ulp(double, double, uint64_t *);
/* Finite |a|,|b|<=1e100 and tolerances0..1; absolute OR relative threshold. */
int nt_close(double, double, double, double);
#endif
