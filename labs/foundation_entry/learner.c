#include "entry.h"

/* Learner scaffold: intentionally unfinished. Every function compiles and
 * returns its sentinel (FE_NOT_IMPLEMENTED, -1 or 0) so the public tests fail
 * until you implement the contract in entry.h. */

enum fe_status fe_fixed(const int32_t *input, size_t count, int32_t *output,
                        size_t capacity)
{
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_recursive(const int32_t *input, size_t count, int32_t *total,
                            struct fe_trace *trace)
{
    (void)input;
    (void)count;
    (void)total;
    (void)trace;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_explicit_stack(const int32_t *input, size_t count,
                                 int32_t *total, struct fe_trace *trace)
{
    (void)input;
    (void)count;
    (void)total;
    (void)trace;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_two_pass(const int32_t *input, size_t count, int32_t *output,
                           size_t capacity)
{
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_staged(const int32_t *input, size_t count, int32_t *output,
                         size_t capacity, struct fe_allocator allocator)
{
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    (void)allocator;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_maximum(const int32_t *input, size_t count, int32_t *maximum)
{
    (void)input;
    (void)count;
    (void)maximum;
    return FE_NOT_IMPLEMENTED;
}

enum fe_status fe_encode24(uint32_t value, unsigned char *output,
                           size_t capacity_bytes)
{
    (void)value;
    (void)output;
    (void)capacity_bytes;
    return FE_NOT_IMPLEMENTED;
}

int fe_any(uint8_t bits, uint8_t mask)
{
    (void)bits;
    (void)mask;
    return -1;
}

int fe_all(uint8_t bits, uint8_t mask)
{
    (void)bits;
    (void)mask;
    return -1;
}

int fe_forbidden_clear(uint8_t bits, uint8_t forbidden)
{
    (void)bits;
    (void)forbidden;
    return -1;
}

int32_t fe_saturating_add(int32_t a, int32_t b)
{
    (void)a;
    (void)b;
    return 0;
}
