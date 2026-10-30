#include "checkpoint_numeric.h"
#include "test.h"

#define BLOCK_SAMPLES 4u
#define SENTINEL 99

/* Fixtures shared by the sections below: impulse and step inputs, the FIR
 * response [1, 2, 1] / 4 to the impulse, and the half-pole IIR response
 * y = 1/2 x + 1/2 y' to the step. */
static const double impulse[BLOCK_SAMPLES] = {1, 0, 0, 0};
static const double step[BLOCK_SAMPLES] = {1, 1, 1, 1};
static const double fir_response[BLOCK_SAMPLES] = {.25, .5, .25, 0};
static const double iir_response[BLOCK_SAMPLES] = {.5, .75, .875, .9375};

/*
 * Splits the four samples at every subset of the first three boundaries (the
 * mask bits) and feeds the pieces one by one with carried state. Every split
 * must reproduce the unsplit response bit for bit.
 */
static void check_partitions(void)
{
    double out[BLOCK_SAMPLES];
    for (unsigned mask = 0; mask < 8; mask++)
    {
        struct cpn_double fir;
        cpn_double_init(&fir);
        double iir_state = 0;
        size_t start = 0;
        for (size_t i = 0; i < BLOCK_SAMPLES; i++)
        {
            if (i == BLOCK_SAMPLES - 1 || (mask & (1u << i)))
            {
                size_t n = i + 1 - start;
                size_t remaining = BLOCK_SAMPLES - start;
                CHECK(cpn_double(&fir, impulse + start, n, out + start,
                                 remaining) == 0);
                CHECK(memcmp(out + start, fir_response + start,
                             n * sizeof(double)) == 0);
                CHECK(cpn_iir_half(&iir_state, step + start, n, out + start,
                                   remaining) == 0);
                CHECK(memcmp(out + start, iir_response + start,
                             n * sizeof(double)) == 0);
                start = i + 1;
            }
        }
    }
}

/* nd_iir with a nonzero initial state, then rejection of an unstable pole. */
static void check_first_order_iir(void)
{
    double out[BLOCK_SAMPLES];
    double state = 2;
    CHECK(nd_iir(&state, step, 4, .5, .5, out, 4) == ND_OK);
    const double expected[] = {1.5, 1.25, 1.125, 1.0625};
    CHECK(memcmp(out, expected, sizeof out) == 0);

    state = 0;
    double snapshot[BLOCK_SAMPLES];
    memcpy(snapshot, out, sizeof out);
    CHECK(nd_iir(&state, step, 4, .5, 1.01, out, 4) == ND_BAD && state == 0 &&
          memcmp(out, snapshot, sizeof out) == 0);
}

/*
 * Injects a failure before sample `fail` (1..4). In prefix mode the samples
 * before it stay committed (done = fail - 1); in atomic mode nothing does
 * (done = 0). Retrying only the unaccepted suffix must give the full response.
 */
static void check_failure_and_retry(void)
{
    double out[BLOCK_SAMPLES];
    for (size_t fail = 1; fail <= BLOCK_SAMPLES; fail++)
    {
        for (int atomic = 0; atomic < 2; atomic++)
        {
            struct cpn_double fir;
            cpn_double_init(&fir);
            size_t done = SENTINEL;
            for (size_t i = 0; i < BLOCK_SAMPLES; i++)
            {
                out[i] = SENTINEL;
            }
            CHECK(cpn_transaction(&fir, impulse, BLOCK_SAMPLES, out,
                                  BLOCK_SAMPLES, fail, atomic, &done) != 0);
            CHECK(done == (atomic ? 0 : fail - 1));
            for (size_t i = 0; i < done; i++)
            {
                CHECK(out[i] == fir_response[i]);
            }
            for (size_t i = done; i < BLOCK_SAMPLES; i++)
            {
                CHECK(out[i] == SENTINEL);
            }
            CHECK(cpn_transaction(&fir, impulse + done, BLOCK_SAMPLES - done,
                                  out + done, BLOCK_SAMPLES - done, 0, atomic,
                                  &done) == 0);
            CHECK(memcmp(out, fir_response, sizeof out) == 0);
        }
    }
}

/* Exact in-place operation, and an IIR block rejected for lack of capacity. */
static void check_in_place_and_capacity(void)
{
    double inplace[BLOCK_SAMPLES] = {1, 0, 0, 0};
    struct cpn_double fir;
    cpn_double_init(&fir);
    CHECK(cpn_double(&fir, inplace, BLOCK_SAMPLES, inplace, BLOCK_SAMPLES) ==
              0 &&
          memcmp(inplace, fir_response, sizeof fir_response) == 0);

    double out[BLOCK_SAMPLES];
    double state = 0;
    CHECK(cpn_iir_half(&state, step, BLOCK_SAMPLES, out, 3) != 0 && state == 0);
}

/* The same recurrences on Q7 storage through nd_qmac: coefficients in 1/128. */
static void check_quantized_filters(void)
{
    const int16_t coefficients[] = {32, 64, 32};
    int16_t history[3] = {0};
    int16_t quantized[BLOCK_SAMPLES];
    const int16_t quantized_expected[] = {32, 64, 32, 0};
    for (size_t i = 0; i < BLOCK_SAMPLES; i++)
    {
        history[2] = history[1];
        history[1] = history[0];
        history[0] = i ? 0 : 127;
        CHECK(nd_qmac(coefficients, history, 3, 7, quantized + i) == ND_OK);
        CHECK(quantized[i] == quantized_expected[i]);
    }

    const int16_t half[] = {64, 64};
    int16_t state = 0;
    int16_t input[2];
    const int expected_state[] = {64, 96, 112, 120};
    for (size_t i = 0; i < BLOCK_SAMPLES; i++)
    {
        input[0] = 127;
        input[1] = state;
        CHECK(nd_qmac(half, input, 2, 7, &state) == ND_OK &&
              state == expected_state[i]);
    }
}

/* A bounded run of an unstable recurrence (gain 1.25 per step, 16 steps). */
static double bounded_unstable_run(void)
{
    double value = 1;
    for (size_t i = 0; i < 16; i++)
    {
        value = 1.25 * value;
    }
    return value;
}

int main(void)
{
    check_partitions();
    check_first_order_iir();
    check_failure_and_retry();
    check_in_place_and_capacity();
    check_quantized_filters();
    double unstable = bounded_unstable_run();
    CHECK(unstable > 35 && unstable < 36);
    printf("Bounded original unstable recurrence16steps=%a\n", unstable);
    puts("PASS actual checkpoint FIR/IIR producer all8 partitions, partial "
         "retry, atomic retry and nonzero-state adapter");
}
