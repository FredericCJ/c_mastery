#include "checkpoint_numeric.h"

/*
 * Learner scaffold for the streaming FIR/IIR producer. Implement this
 * compatible producer as your T17 artifact (no later checkpoint acceptance is
 * a prerequisite). Only the two init functions work; every other entry point
 * ignores its arguments and returns CPN_INVALID, so filter_test rejects this
 * file until the producer is written.
 */
void cpn_exact_init(struct cpn_exact *state)
{
    state->previous = 0;
    state->older = 0;
}

void cpn_double_init(struct cpn_double *state)
{
    state->previous = 0;
    state->older = 0;
}

enum cpn_status cpn_exact(struct cpn_exact *state, const int32_t *input,
                          size_t count, int64_t *output, size_t capacity)
{
    (void)state;
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    return CPN_INVALID;
}

enum cpn_status cpn_double(struct cpn_double *state, const double *input,
                           size_t count, double *output, size_t capacity)
{
    (void)state;
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    return CPN_INVALID;
}

enum cpn_status cpn_transaction(struct cpn_double *state, const double *input,
                                size_t count, double *output, size_t capacity,
                                size_t fail_at, int atomic, size_t *done)
{
    (void)state;
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    (void)fail_at;
    (void)atomic;
    (void)done;
    return CPN_INVALID;
}

enum cpn_status cpn_round_even(int64_t numerator, int32_t *out)
{
    (void)numerator;
    (void)out;
    return CPN_INVALID;
}

enum cpn_status cpn_iir_half(double *state, const double *input, size_t count,
                             double *output, size_t capacity)
{
    (void)state;
    (void)input;
    (void)count;
    (void)output;
    (void)capacity;
    return CPN_INVALID;
}
