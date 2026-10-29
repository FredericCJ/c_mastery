#ifndef BF_FIXED_POINT_H
#define BF_FIXED_POINT_H
#include "depth.h"

struct bf_signal
{
    int32_t product_sum, rounded_raw, error_numerator;
    int8_t output;
    unsigned clipped;
};

int bf_q7_even(const int8_t *, const int8_t *, size_t, int8_t *);
int bf_q11(const int16_t *, const int16_t *, size_t, int16_t *);
int bf_signal(const int8_t *, const int8_t *, size_t, struct bf_signal *);

struct bf_small
{
    int32_t raw_numerator, raw_denominator, rounded_raw, error_numerator;
    int8_t output;
    unsigned clipped;
};

struct bf_cascade
{
    struct bf_small first, second;
    int32_t error_numerator;
    unsigned bound_numerator, bounded;
};

enum
{
    BF_AWAY,
    BF_EVEN
};

int bf_mul16(int8_t, int8_t, unsigned, struct bf_small *);
int bf_mix16_32(int8_t, int8_t, unsigned, struct bf_small *);
int bf_two16(int8_t, int8_t, int8_t, unsigned, struct bf_cascade *);
#endif
