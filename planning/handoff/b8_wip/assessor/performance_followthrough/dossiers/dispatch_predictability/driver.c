/*
 * Dispatch-predictability dossier driver. Author-only (dispatch.h). One
 * invocation is one row, printed as one JSON line.
 *
 *   dp_driver measure MECHANISM DISTRIBUTION N CPU REPETITIONS CONFIG...
 *   dp_driver check   MECHANISM DISTRIBUTION N CPU REPETITIONS
 *
 * MECHANISM is branch, indirect, hinted or branchless; DISTRIBUTION constant,
 * alternating or random; N calls per repetition (1..16777216); CPU in the
 * allowed set; REPETITIONS 1..1024; each CONFIG a decimal raw event
 * configuration (up to 8). Outside the window: pin, verify one repetition
 * of all four mechanisms against the oracle, warm the requested mechanism
 * for one repetition, open the group. Inside: REPETITIONS repetitions.
 *
 * Exit status: 0 a row was printed; 1 a sum broke the contract; 2 invalid
 * request or infrastructure failure.
 */
#define _GNU_SOURCE
#include "dispatch.h"
#include "dossier_common.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define DP_DRIVER_MAX_REPETITIONS 1024u

/* Keeps every sum observable so no pass is optimized away. */
static volatile uint64_t observable;

struct dp_invocation
{
    bool measure;
    enum dp_mechanism mechanism;
    enum dp_distribution distribution;
    uint64_t n;
    unsigned cpu;
    unsigned repetitions;
    struct pfd_events events;
};

static bool parse(int argc, char **argv, struct dp_invocation *invocation)
{
    memset(invocation, 0, sizeof *invocation);
    if (argc < 7)
    {
        return false;
    }
    invocation->measure = (strcmp(argv[1], "measure") == 0);
    if ((!invocation->measure && (strcmp(argv[1], "check") != 0)) ||
        (invocation->measure && (argc < 8)) ||
        (!invocation->measure && (argc != 7)))
    {
        return false;
    }
    if (!dp_mechanism_lookup(argv[2], &invocation->mechanism) ||
        !dp_distribution_lookup(argv[3], &invocation->distribution))
    {
        (void)fprintf(stderr,
                      "dp_driver: unknown mechanism or distribution "
                      "\"%s\" \"%s\"\n",
                      argv[2], argv[3]);
        return false;
    }
    return pfd_parse_u64("N", argv[4], 1, DP_MAX_N, &invocation->n) &&
           pfd_parse_unsigned("CPU", argv[5], 0, PFD_MAX_CPU,
                              &invocation->cpu) &&
           pfd_parse_unsigned("REPETITIONS", argv[6], 1,
                              DP_DRIVER_MAX_REPETITIONS,
                              &invocation->repetitions) &&
           pfd_parse_events(argv + 7, (size_t)argc - 7u, &invocation->events);
}

static void print_row(const struct dp_invocation *invocation,
                      const struct cm_clock_calibration *clock,
                      const struct pfd_window *window, uint64_t checksum,
                      uint64_t selected, bool contract)
{
    (void)printf("{\"kind\":\"dossier-row\",\"dossier\":"
                 "\"dispatch_predictability\",\"mode\":\"%s\","
                 "\"mechanism\":\"%s\",\"distribution\":\"%s\",\"n\":%" PRIu64
                 ",\"cpu\":%u,\"repetitions\":%u,\"contract\":%s,"
                 "\"checksum\":%" PRIu64 ",\"selected_operation1\":%" PRIu64
                 ",",
                 invocation->measure ? "measure" : "check",
                 dp_mechanism_name(invocation->mechanism),
                 dp_distribution_name(invocation->distribution), invocation->n,
                 invocation->cpu, invocation->repetitions,
                 contract ? "true" : "false", checksum, selected);
    pfd_print_clock(stdout, clock);
    (void)fputc(',', stdout);
    pfd_print_window(stdout, "window", window);
    (void)puts("}");
}

/* Every mechanism must agree with the oracle before any is measured. */
static bool verify(const struct dp_selector *selector,
                   const struct dp_table *table, enum dp_distribution dist,
                   size_t n)
{
    uint64_t expected = dp_oracle(dist, n, 1);

    for (unsigned m = 0; m < DP_MECHANISM_COUNT; ++m)
    {
        uint64_t sum = dp_run((enum dp_mechanism)m, selector, table, n, 1);

        observable ^= sum;
        if (sum != expected)
        {
            return false;
        }
    }
    return true;
}

static int measure(const struct dp_invocation *invocation,
                   const struct cm_clock_calibration *clock)
{
    struct dp_selector selector = dp_selector_for(invocation->distribution);
    struct dp_table table = dp_table_make();
    size_t n = (size_t)invocation->n;
    struct pfd_window window;
    uint64_t selected = 0;

    if (!verify(&selector, &table, invocation->distribution, n))
    {
        (void)fputs("DP_DRIVER_ASSERT:equivalent mechanisms and oracle\n",
                    stderr);
        return PFD_EXIT_CONTRACT;
    }
    observable ^= dp_run(invocation->mechanism, &selector, &table, n, 1);
    (void)pfd_window_open(&window, &invocation->events);
    pfd_window_begin(&window);
    uint64_t checksum = dp_run(invocation->mechanism, &selector, &table, n,
                               invocation->repetitions);
    pfd_window_end(&window);
    pfd_window_close(&window);
    observable ^= checksum;
    for (unsigned r = 0; r < invocation->repetitions; ++r)
    {
        selected += dp_oracle_selected(invocation->distribution, n, r);
    }
    bool contract = (checksum == dp_oracle(invocation->distribution, n,
                                           invocation->repetitions));
    print_row(invocation, clock, &window, checksum, selected, contract);
    if (!window.clock_ok)
    {
        (void)fputs("dp_driver: clock read failed\n", stderr);
        return PFD_EXIT_INVALID;
    }
    if (!contract)
    {
        (void)fputs("DP_DRIVER_ASSERT:measured sum\n", stderr);
        return PFD_EXIT_CONTRACT;
    }
    return PFD_EXIT_OK;
}

int main(int argc, char **argv)
{
    struct dp_invocation invocation;
    struct pfd_allowed allowed;
    struct cm_clock_calibration clock;

    if (!parse(argc, argv, &invocation))
    {
        (void)fputs("usage: dp_driver measure|check MECHANISM DISTRIBUTION N "
                    "CPU REPETITIONS [CONFIG...]\n",
                    stderr);
        return PFD_EXIT_INVALID;
    }
    if (!pfd_allowed_capture(&allowed) ||
        (pfd_pin_calling_thread(&allowed, invocation.cpu) != 0))
    {
        (void)fprintf(stderr, "dp_driver: cannot place on CPU %u\n",
                      invocation.cpu);
        return PFD_EXIT_INVALID;
    }
    if (!cm_clock_calibrate(CM_CLOCK_MONOTONIC_RAW, &clock))
    {
        perror("dp_driver: clock");
        return PFD_EXIT_INVALID;
    }
    int status = measure(&invocation, &clock);
    if ((fflush(stdout) != 0) || (ferror(stdout) != 0))
    {
        return PFD_EXIT_INVALID;
    }
    return status;
}
