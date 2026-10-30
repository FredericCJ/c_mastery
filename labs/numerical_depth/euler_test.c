#include "test.h"

#define SENTINEL 99
#define OUT_CAPACITY (ND_STREAM_MAX + 1)

/* Forward Euler for y' = -rate*y: y_{k+1} = (1 - rate*h) * y_k. */
static void check_recurrence(void)
{
    double out[OUT_CAPACITY];
    CHECK(nd_euler(1, .5, 1, 4, out, OUT_CAPACITY) == ND_OK && out[4] == .0625);
    CHECK(nd_euler(1, 3, 1, 4, out, OUT_CAPACITY) == ND_OK && out[4] == 16);
    CHECK(nd_euler(1, .25, 2, 4, out, OUT_CAPACITY) == ND_OK &&
          out[4] == .0625);
    CHECK(nd_euler(1, 2, 1, 4, out, OUT_CAPACITY) == ND_OK && out[1] == -1 &&
          out[2] == 1 && out[4] == 1);
}

static void check_admission(void)
{
    double out[OUT_CAPACITY];
    out[0] = SENTINEL;
    CHECK(nd_euler(1, -1, 1, 4, out, OUT_CAPACITY) == ND_BAD &&
          out[0] == SENTINEL);
    CHECK(nd_euler(1, .5, 1, 4, out, 4) == ND_BAD && out[0] == SENTINEL);
}

/* Halving h at fixed T = 1 must shrink the error against exp(-1). */
static void check_step_halving(void)
{
    double out[OUT_CAPACITY];
    double previous_error = 1;
    for (size_t steps = 2; steps <= 8; steps *= 2)
    {
        CHECK(nd_euler(1, 1.0 / (double)steps, 1, steps, out, OUT_CAPACITY) ==
              ND_OK);
        double error = fabs(out[steps] - exp(-1));
        CHECK(error < previous_error);
        previous_error = error;
        printf("Euler n=%zu observed=%a error=%a\n", steps, out[steps], error);
    }
}

int main(void)
{
    check_recurrence();
    check_admission();
    check_step_halving();
    puts(
        "PASS recurrence, rate, convergence, bounded oscillation and admission");
}
