#ifndef FC_COMPARISON_H
#define FC_COMPARISON_H
#include <stdint.h>

struct fc_report
{
    unsigned count, sum, full, empty, boosted, restored, closed, trace[3],
        traces;
    unsigned ticks, elapsed, isr, nested_retained;
};

/* urgency1 is highest; vendor priority3 maps urgency1; original CMX uses urgency directly. */
int fc_ticks(unsigned milliseconds, unsigned frequency, unsigned *ticks);
unsigned fc_priority(unsigned urgency);
void fc_run(struct fc_report *result);
void fc_done(const struct fc_report *result);
void fc_fail(unsigned line);
#define FC_REQUIRE(x)          \
    do                         \
    {                          \
        if (!(x))              \
            fc_fail(__LINE__); \
    } while (0)
#ifndef FC_MUTANT
#define FC_MUTANT 0
#endif
#ifndef FC_TICK_RATE
#define FC_TICK_RATE 100
#endif
#endif
