/*
 * Concurrent-state dossier driver (WORKED_EXAMPLE.md). Supplied course
 * infrastructure. One invocation is one row: one complete lifecycle of
 * concurrent_state.h, printed as one JSON line.
 *
 *   cs_driver measure LAYOUT CPU0 CPU1 ITERATIONS REPETITIONS CONFIG...
 *   cs_driver check   LAYOUT CPU0 CPU1 ITERATIONS REPETITIONS
 *
 * LAYOUT is shared, adjacent, separate or solo. CPU0 hosts the measured
 * (calling) thread and CPU1 the peer; both must be in the allowed set.
 * ITERATIONS (1..16777216) is the work of one repetition; the window
 * performs ITERATIONS * REPETITIONS (1..1024) increments per thread after
 * ITERATIONS warm-up increments. Each CONFIG is a decimal raw event
 * configuration (up to 8); every active thread opens the same group on
 * itself. `check` counts nothing.
 *
 * Exit status: 0 a row was printed; 1 the run broke its correctness
 * contract; 2 invalid request or infrastructure failure (placement, thread,
 * clock, memory). A refused counter group is not a failure: the row says
 * which state each group reached.
 */
#define _GNU_SOURCE
#include "concurrent_state.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define CS_DRIVER_MAX_ITERATIONS 16777216u
#define CS_DRIVER_MAX_REPETITIONS 1024u

struct cs_invocation
{
    bool measure;
    unsigned repetitions;
    uint64_t per_repetition;
    struct pfd_events events;
    struct cs_request request;
};

static void usage(void)
{
    (void)fputs("usage: cs_driver measure LAYOUT CPU0 CPU1 ITERATIONS "
                "REPETITIONS CONFIG...\n"
                "       cs_driver check LAYOUT CPU0 CPU1 ITERATIONS "
                "REPETITIONS\n",
                stderr);
}

static bool parse(int argc, char **argv, struct cs_invocation *invocation)
{
    memset(invocation, 0, sizeof *invocation);
    if (argc < 7)
    {
        return false;
    }
    invocation->measure = (strcmp(argv[1], "measure") == 0);
    if (!invocation->measure && (strcmp(argv[1], "check") != 0))
    {
        return false;
    }
    if ((invocation->measure && (argc < 8)) ||
        (!invocation->measure && (argc != 7)))
    {
        return false;
    }
    struct cs_request *request = &invocation->request;
    if (!cs_layout_lookup(argv[2], &request->layout))
    {
        (void)fprintf(stderr, "cs_driver: unknown layout \"%s\"\n", argv[2]);
        return false;
    }
    if (!pfd_parse_unsigned("CPU0", argv[3], 0, PFD_MAX_CPU,
                            &request->cpu[0]) ||
        !pfd_parse_unsigned("CPU1", argv[4], 0, PFD_MAX_CPU,
                            &request->cpu[1]) ||
        !pfd_parse_u64("ITERATIONS", argv[5], 1, CS_DRIVER_MAX_ITERATIONS,
                       &invocation->per_repetition) ||
        !pfd_parse_unsigned("REPETITIONS", argv[6], 1,
                            CS_DRIVER_MAX_REPETITIONS,
                            &invocation->repetitions) ||
        !pfd_parse_events(argv + 7, (size_t)argc - 7u, &invocation->events))
    {
        return false;
    }
    /* Both factors are bounded: the product is at most 2^34. */
    request->iterations = invocation->per_repetition * invocation->repetitions;
    request->warmup = invocation->per_repetition;
    request->events = &invocation->events;
    return true;
}

static void print_thread(FILE *out, const char *key,
                         const struct cs_thread_result *thread)
{
    (void)fprintf(out,
                  "\"%s\":{\"active\":%s,\"measured\":%" PRIu64
                  ",\"tail\":%" PRIu64 ",\"tail_end_ns\":%" PRIu64
                  ",\"final_value\":%" PRIu64 ",\"slot_address\":%" PRIuPTR
                  ",\"slot_line\":%" PRIuPTR ",",
                  key, thread->active ? "true" : "false", thread->measured,
                  thread->tail, thread->tail_end_ns, thread->final_value,
                  thread->slot_address, thread->slot_address / CS_LINE_BYTES);
    pfd_print_window(out, "window", &thread->window);
    (void)fputc('}', out);
}

static void print_row(const struct cs_invocation *invocation,
                      const struct cm_clock_calibration *clock,
                      const struct cs_result *result)
{
    const struct cs_request *request = &invocation->request;

    (void)printf("{\"kind\":\"dossier-row\",\"dossier\":\"concurrent_state\","
                 "\"mode\":\"%s\",\"layout\":\"%s\",\"cpus\":[%u,%u],"
                 "\"iterations_per_repetition\":%" PRIu64
                 ",\"repetitions\":%u,\"window_iterations\":%" PRIu64
                 ",\"warmup\":%" PRIu64 ",\"status\":\"%s\",\"contract\":%s,"
                 "\"line_bytes\":%u,\"separation_bytes\":%u,",
                 invocation->measure ? "measure" : "check",
                 cs_layout_name(request->layout), request->cpu[0],
                 request->cpu[1], invocation->per_repetition,
                 invocation->repetitions, request->iterations, request->warmup,
                 cs_status_name(result->status),
                 (result->status == CS_OK) ? "true" : "false", CS_LINE_BYTES,
                 CS_SEPARATION_BYTES);
    pfd_print_clock(stdout, clock);
    (void)fputc(',', stdout);
    print_thread(stdout, "thread0", &result->thread[0]);
    (void)fputc(',', stdout);
    print_thread(stdout, "thread1", &result->thread[1]);
    (void)printf(",\"lifecycle\":{\"create_begin_ns\":%" PRIu64
                 ",\"peer_entry_ns\":%" PRIu64 ",\"create_return_ns\":%" PRIu64
                 ",\"join_return_ns\":%" PRIu64 "}}\n",
                 result->create_begin_ns, result->peer_entry_ns,
                 result->create_return_ns, result->join_return_ns);
}

int main(int argc, char **argv)
{
    struct cs_invocation invocation;
    struct cm_clock_calibration clock;
    struct cs_result result;

    if (!parse(argc, argv, &invocation))
    {
        usage();
        return PFD_EXIT_INVALID;
    }
    /* Calibrated before cs_run pins anything: the allowed set cs_run
     * captures must still be the process's own. */
    if (!cm_clock_calibrate(CM_CLOCK_MONOTONIC_RAW, &clock))
    {
        perror("cs_driver: clock");
        return PFD_EXIT_INVALID;
    }
    enum cs_status status = cs_run(&invocation.request, &result);
    if ((status != CS_OK) && (status != CS_CONTRACT))
    {
        (void)fprintf(stderr, "cs_driver: run failed (%s)\n",
                      cs_status_name(status));
        return PFD_EXIT_INVALID;
    }
    print_row(&invocation, &clock, &result);
    if ((fflush(stdout) != 0) || (ferror(stdout) != 0))
    {
        return PFD_EXIT_INVALID;
    }
    if (status == CS_CONTRACT)
    {
        (void)fputs("CS_DRIVER_ASSERT:correctness contract\n", stderr);
        return PFD_EXIT_CONTRACT;
    }
    return PFD_EXIT_OK;
}
