/* Exact service accounting, stale-expiry delivery and burst transfer checks. */
#include "mcu.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                        \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "MF_SUPPLEMENT:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)
static _Alignas(8) uint32_t storage[3][160];
static unsigned totals[3][5];

static void account(const struct mf_mf *m, unsigned delta)
{
    for (unsigned t = 1; t < 3; t++)
    {
        totals[t][m->kernel.task[t].state] += delta;
    }
}

static int service(void)
{
    struct mf_mf m;
    mf_handle_id a, b;
    C(mf_init(&m, storage[0], 160, CMX_RR) == CMX_OK);
    C(mf_add(&m, storage[1], 160, 0x101, 0x201, 1, 1, 2, &a) == CMX_OK);
    C(mf_add(&m, storage[2], 160, 0x101, 0x201, 1, 1, 2, &b) == CMX_OK);
    C(mf_schedule(&m) == 1);
    account(&m, 3);
    C(cmx_sleep(&m.kernel, 1, cmx_at(7)) == CMX_BLOCKED);
    C(mf_schedule(&m) == 2);
    account(&m, 4);
    cmx_tick(&m.kernel, 7);
    C(mf_schedule(&m) == 1);
    account(&m, 2);
    C(mf_exit(&m, a, 0) == CMX_OK);
    C(mf_schedule(&m) == 2);
    account(&m, 3);
    C(totals[1][CMX_RUNNING] == 5 && totals[1][CMX_READY] == 0 &&
      totals[1][CMX_WAITING] == 4 && totals[1][CMX_EXITED] == 3);
    C(totals[2][CMX_RUNNING] == 7 && totals[2][CMX_READY] == 5 &&
      totals[2][CMX_WAITING] == 0);
    C(totals[1][CMX_RUNNING] + totals[2][CMX_RUNNING] == 12 &&
      mf_invariant(&m));
    puts(
        "MF_SUPPLEMENT:service intervals[0,3):1,[3,7):2,[7,9):1,[9,12):2; task1 run5 ready0 blocked4 exited3; task2 run7 ready5 blocked0");
    C(mf_init(&m, storage[0], 160, CMX_RR) == CMX_OK);
    C(mf_add(&m, storage[1], 160, 0x101, 0x201, 1, 1, 2, &a) == CMX_OK);
    cmx_sem sem;
    cmx_sem_init(&sem, 0, 1);
    C(cmx_sem_take(&m.kernel, &sem, 1, cmx_forever()) == CMX_BLOCKED);
    C(cmx_sem_give(&m.kernel, &sem) == CMX_OK &&
      m.kernel.task[1].result == CMX_OK);
    C(cmx_sem_take(&m.kernel, &sem, 1, cmx_forever()) == CMX_BLOCKED);
    puts(
        "MF_SUPPLEMENT:wrong resumed-call reentry blocks after already successful stored completion");
    return 0;
}

static int expiry(void)
{
    struct mf_timers t;
    mf_handle_id old, current, events[MF_TIMERS];
    size_t n;
    mf_timers_init(&t, 0);
    C(mf_timer_arm(&t, 3, &old) == CMX_OK);
    C(mf_timer_cancel(&t, old) == CMX_OK);
    C(mf_timer_arm(&t, 4, &current) == CMX_OK && old != current &&
      (old & 31u) == (current & 31u));
    unsigned completions =
        0; /* Consumer owns current ticket, not only slot index. */
    mf_handle_id delayed = old;
    if (delayed == current)
    {
        completions++;
    }
    C(completions == 0);
    n = MF_TIMERS;
    C(mf_timer_poll(&t, 3, events, &n) == CMX_OK && n == 0);
    n = MF_TIMERS;
    C(mf_timer_poll(&t, 4, events, &n) == CMX_OK && n == 1 &&
      events[0] == current);
    if (events[0] == current)
    {
        completions++;
        current = 0;
    }
    if (events[0] == current)
    {
        completions++;
    }
    C(completions == 1);
    C(mf_timer_cancel(&t, old) == CMX_INVALID);
    puts(
        "MF_SUPPLEMENT:old canceled expiry refused after same-slot reuse; current delivery exactly once; duplicate cannot complete replacement");
    return 0;
}

static int burst(void)
{
    uint32_t next, skip;
    C(mf_periodic(20, 10, 47, &next, &skip) == CMX_OK && next == 50 &&
      skip == 2);
    /* A single server executing work27 at release20 finishes47. A catch-up
     * third job nominally30 could start47 (jitter17); selected skip omits it. */
    C(47u - 30u == 17u);
    C(mf_periodic(50, 10, 51, &next, &skip) == CMX_OK && next == 60 &&
      skip == 0);
    uint32_t count;
    uint64_t rest;
    C(mf_counts(101, 25000000, 100000, &count, &rest) == CMX_OK &&
      count == 2525 && !rest);
    C(mf_counts(101, 32768, 100000, &count, &rest) == CMX_OK && count == 4 &&
      !rest);
    puts(
        "MF_SUPPLEMENT:burst requested10/20/30; completion13/47; catchup jitter17 or skip30/40 then release50;25MHz101us=2525counts versus32768Hz4counts");
    return 0;
}

int main(void)
{
    if (service() || expiry() || burst())
    {
        return 1;
    }
    puts("MF_PASS:supplement");
    return 0;
}
