#ifndef NUMERIC_INTERVAL_H
#define NUMERIC_INTERVAL_H
#include <stdint.h>
/* Value denotes the closed REAL interval [lo/2^bits, hi/2^bits]. */
typedef struct { int32_t lo, hi; unsigned bits; } NiInterval;
enum { NI_OK=0, NI_INVALID=1, NI_SCALE=2, NI_ZERO=3, NI_RANGE=4, NI_EMPTY=5 };
enum { NI_ADD=0, NI_SUB=1, NI_MUL=2, NI_DIV=3, NI_HULL=4, NI_INTERSECT=5 };
int ni_ratio(int32_t numerator, int32_t denominator, unsigned bits, NiInterval *out);
int ni_binary(unsigned operation, NiInterval a, NiInterval b, NiInterval *out);
int ni_square(NiInterval a, NiInterval *out);
int ni_rescale(NiInterval a, unsigned bits, NiInterval *out);
#endif
