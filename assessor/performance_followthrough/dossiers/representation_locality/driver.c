/*
 * Representation/locality dossier driver. Author-only (locality.h). One
 * invocation is one row, printed as one JSON line.
 *
 *   rl_driver measure LAYOUT N CPU REPETITIONS CONFIG...
 *   rl_driver check   LAYOUT N CPU REPETITIONS
 *
 * LAYOUT is soa, aos16, aos64 or aos128; N keys (1..4194304); CPU must be
 * in the allowed set; REPETITIONS 1..1024; each CONFIG a decimal raw event
 * configuration (up to 8). The calling thread is pinned first, so every
 * page is first written on the measured CPU. Outside the window: allocate
 * and initialize (timed as setup), verify one pass against the oracle,
 * warm one more pass, open the group. Inside: REPETITIONS passes.
 *
 * Exit status: 0 a row was printed; 1 a sum broke the contract; 2 invalid
 * request or infrastructure failure.
 */
#define _GNU_SOURCE
#include "dossier_common.h"
#include "locality.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define RL_DRIVER_MAX_REPETITIONS 1024u
#define RL_SEED UINT64_C(0x243F6A8885A308D3)

/* Keeps every sum observable so no pass is optimized away. */
static volatile uint64_t observable;

struct rl_invocation
{
    bool measure;
    enum rl_layout layout;
    uint64_t n;
    unsigned cpu;
    unsigned repetitions;
    struct pfd_events events;
};

static bool parse(int argc, char **argv, struct rl_invocation *invocation)
{
    memset(invocation, 0, sizeof *invocation);
    if (argc < 6)
    {
        return false;
    }
    invocation->measure = (strcmp(argv[1], "measure") == 0);
    if ((!invocation->measure && (strcmp(argv[1], "check") != 0)) ||
        (invocation->measure && (argc < 7)) ||
        (!invocation->measure && (argc != 6)))
    {
        return false;
    }
    if (!rl_layout_lookup(argv[2], &invocation->layout))
    {
        (void)fprintf(stderr, "rl_driver: unknown layout \"%s\"\n", argv[2]);
        return false;
    }
    return pfd_parse_u64("N", argv[3], 1, RL_MAX_N, &invocation->n) &&
           pfd_parse_unsigned("CPU", argv[4], 0, PFD_MAX_CPU,
                              &invocation->cpu) &&
           pfd_parse_unsigned("REPETITIONS", argv[5], 1,
                              RL_DRIVER_MAX_REPETITIONS,
                              &invocation->repetitions) &&
           pfd_parse_events(argv + 6, (size_t)argc - 6u, &invocation->events);
}

static void print_row(const struct rl_invocation *invocation,
                      const struct rl_data *data,
                      const struct cm_clock_calibration *clock,
                      const struct pfd_window *window, uint64_t checksum,
                      uint64_t setup_ns, bool contract)
{
    (void)printf("{\"kind\":\"dossier-row\",\"dossier\":"
                 "\"representation_locality\",\"mode\":\"%s\","
                 "\"layout\":\"%s\",\"n\":%" PRIu64
                 ",\"stride_bytes\":%zu,\"allocated_bytes\":%zu,"
                 "\"cpu\":%u,\"repetitions\":%u,\"contract\":%s,"
                 "\"checksum\":%" PRIu64 ",\"setup_ns\":%" PRIu64 ",",
                 invocation->measure ? "measure" : "check",
                 rl_layout_name(invocation->layout), invocation->n,
                 rl_stride_bytes(invocation->layout), data->bytes,
                 invocation->cpu, invocation->repetitions,
                 contract ? "true" : "false", checksum, setup_ns);
    pfd_print_clock(stdout, clock);
    (void)fputc(',', stdout);
    pfd_print_window(stdout, "window", window);
    (void)puts("}");
}

/* Verify one pass, warm one pass, then measure. Returns the exit status;
 * the row is printed whenever the window ran. */
static int measure(const struct rl_invocation *invocation,
                   const struct rl_data *data,
                   const struct cm_clock_calibration *clock, uint64_t setup_ns)
{
    struct pfd_window window;
    uint64_t one_pass = rl_oracle(data->n, 1);
    uint64_t verified = rl_sum(data, 1);

    observable ^= verified;
    if (verified != one_pass)
    {
        (void)fputs("RL_DRIVER_ASSERT:verification pass\n", stderr);
        return PFD_EXIT_CONTRACT;
    }
    observable ^= rl_sum(data, 1);
    (void)pfd_window_open(&window, &invocation->events);
    pfd_window_begin(&window);
    uint64_t checksum = rl_sum(data, invocation->repetitions);
    pfd_window_end(&window);
    pfd_window_close(&window);
    observable ^= checksum;
    bool contract = (checksum == rl_oracle(data->n, invocation->repetitions));
    print_row(invocation, data, clock, &window, checksum, setup_ns, contract);
    if (!window.clock_ok)
    {
        (void)fputs("rl_driver: clock read failed\n", stderr);
        return PFD_EXIT_INVALID;
    }
    if (!contract)
    {
        (void)fputs("RL_DRIVER_ASSERT:measured sum\n", stderr);
        return PFD_EXIT_CONTRACT;
    }
    return PFD_EXIT_OK;
}

int main(int argc, char **argv)
{
    struct rl_invocation invocation;
    struct pfd_allowed allowed;
    struct cm_clock_calibration clock;
    struct rl_data data;
    uint64_t setup_start = 0;
    uint64_t setup_end = 0;

    if (!parse(argc, argv, &invocation))
    {
        (void)fputs("usage: rl_driver measure|check LAYOUT N CPU REPETITIONS "
                    "[CONFIG...]\n",
                    stderr);
        return PFD_EXIT_INVALID;
    }
    if (!pfd_allowed_capture(&allowed) ||
        (pfd_pin_calling_thread(&allowed, invocation.cpu) != 0))
    {
        (void)fprintf(stderr, "rl_driver: cannot place on CPU %u\n",
                      invocation.cpu);
        return PFD_EXIT_INVALID;
    }
    if (!cm_clock_calibrate(CM_CLOCK_MONOTONIC_RAW, &clock) ||
        !cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &setup_start))
    {
        perror("rl_driver: clock");
        return PFD_EXIT_INVALID;
    }
    if (rl_create(invocation.layout, (size_t)invocation.n, RL_SEED, &data) !=
        RL_OK)
    {
        (void)fputs("rl_driver: allocation failed\n", stderr);
        return PFD_EXIT_INVALID;
    }
    int status = PFD_EXIT_INVALID;
    if (cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &setup_end))
    {
        status = measure(&invocation, &data, &clock, setup_end - setup_start);
    }
    rl_destroy(&data);
    if ((fflush(stdout) != 0) || (ferror(stdout) != 0))
    {
        return PFD_EXIT_INVALID;
    }
    return status;
}
