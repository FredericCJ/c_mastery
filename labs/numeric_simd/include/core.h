#ifndef B06_NS_H
#define B06_NS_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

int ns_q15(int16_t a, int16_t b);
int ns_round_even(int numerator, unsigned denominator, int *out);
int ns_dot(const int16_t *a, const int16_t *b, size_t n, int64_t *out);
int ns_sum(const double *a, size_t n, double *out);
int ns_norm(const double *a, size_t n, double *out);
int ns_matmul(const int32_t *a, const int32_t *b, unsigned rows, unsigned inner, unsigned cols, int64_t *out);
int ns_add(const int16_t *a, const int16_t *b, size_t n, int16_t *out);
int ns_close(double actual, double expected, double atol, double rtol);
#endif
