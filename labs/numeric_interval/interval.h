#ifndef NUMERIC_INTERVAL_H
#define NUMERIC_INTERVAL_H
#include <stdint.h>
/* Value denotes the closed REAL interval [lo/2^bits, hi/2^bits]. */
struct ni_interval { int32_t lo, hi; unsigned bits; };
enum { NI_OK=0, NI_INVALID=1, NI_SCALE=2, NI_ZERO=3, NI_RANGE=4, NI_EMPTY=5 };
enum { NI_ADD=0, NI_SUB=1, NI_MUL=2, NI_DIV=3, NI_HULL=4, NI_INTERSECT=5 };
int ni_ratio(int32_t numerator, int32_t denominator, unsigned bits, struct ni_interval *out);
int ni_binary(unsigned operation, struct ni_interval a, struct ni_interval b, struct ni_interval *out);
int ni_square(struct ni_interval a, struct ni_interval *out);
int ni_rescale(struct ni_interval a, unsigned bits, struct ni_interval *out);
#endif
