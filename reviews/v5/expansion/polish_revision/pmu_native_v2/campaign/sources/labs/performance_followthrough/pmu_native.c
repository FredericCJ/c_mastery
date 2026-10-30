/*
 * Supplied native observation adapter (PMU_PROTOCOL.md), not a learner
 * solution. The Linux perf ABI, sched_getcpu and GCC/Clang builtins are
 * explicit dependencies. This file is only the command line: request
 * parsing, mode dispatch and measurement brackets. The workloads live in
 * pmu_workloads.c, sampling in pmu_sampling.c and serialization in
 * pmu_report.c.
 *
 *   pmu_native count  FAMILY N VARIANT [REPETITIONS]
 *   pmu_native work   FAMILY N VARIANT [REPETITIONS]
 *   pmu_native sample FAMILY N VARIANT [REPETITIONS [PRECISION]] [OPTION...]
 *   pmu_native check  FAMILY N VARIANT [REPETITIONS [PRECISION]] [OPTION...]
 *   pmu_native families
 *
 *   OPTION  --drain=final|concurrent    (default final)
 *           --pages=P      data pages, a power of two in 1..256 (default 8)
 *           --period=C     cycles per sample, 1000..4294967295 (200000)
 *           --retain=S     samples kept for output, 0..1048576 (65536)
 *
 * N is 1..4194304 (smaller for some families), VARIANT 0 or 1, REPETITIONS
 * 1..1024 (default 16) and PRECISION (precise_ip) 0..3 (default 0). Every
 * number is plain ASCII decimal (labs/support/cm_parse.h). A request outside
 * these bounds is rejected; it is never clamped, rounded or weakened.
 *
 *   count     strict instructions+cycles group around one measured batch
 *   work      the same batch bracketed by perf stat's control pipe
 *   sample    cycle sampling of the batch into a perf ring
 *   check     validate the request and the workload oracle; opens no event
 *   families  list the workload families and whether this build has them
 *
 * Exit status: 0 an observation was printed (its status field says whether
 * hardware was measured); 1 a component failed: a workload failed its
 * equivalence, oracle or checksum check, or the ring parser pf_ring_drain
 * stopped on the kernel's records; 2 invalid request or infrastructure
 * failure; 3 the family is not compiled into this build.
 */
#define _GNU_SOURCE /* sched_getcpu */
#include "cm_clock.h"
#include "cm_parse.h"
#include "pmu.h"
#include "pmu_control.h"
#include "pmu_report.h"
#include "pmu_sampling.h"
#include "pmu_workloads.h"

#include <inttypes.h>
#include <linux/perf_event.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    EXIT_ASSERT = 1,
    EXIT_INVALID = 2,
    EXIT_UNAVAILABLE = 3
};

enum native_defaults
{
    DEFAULT_REPETITIONS = 16,
    DEFAULT_PAGES = 8,
    MAX_PAGES = 256,
    MAX_PRECISION = 3,
    POLL_INTERVAL_MS = 1
};

/* 200000 cycles keeps 2x margin under the common perf_event_max_sample_rate
 * of 50000/s at 4.4 GHz; every sampling line reports the host's own floor. */
#define DEFAULT_PERIOD_CYCLES UINT64_C(200000)
#define MIN_PERIOD_CYCLES UINT64_C(1000)
#define MAX_PERIOD_CYCLES UINT64_C(4294967295)
#define DEFAULT_RETAIN ((size_t)65536)
#define MAX_RETAIN ((size_t)1048576)

enum native_mode
{
    MODE_COUNT,
    MODE_WORK,
    MODE_SAMPLE,
    MODE_CHECK
};

struct native_request
{
    enum native_mode mode;
    struct pf_request_summary summary;
    struct pf_sampling_config sampling;
};

/* Keeps every checksum observable so no measured batch is optimized away. */
static volatile uint64_t observable;

static void usage(const char *program)
{
    (void)fprintf(stderr,
                  "usage: %s count|work FAMILY N VARIANT [REPETITIONS]\n"
                  "       %s sample|check FAMILY N VARIANT [REPETITIONS "
                  "[PRECISION]] [--drain=final|concurrent] [--pages=P] "
                  "[--period=CYCLES] [--retain=S]\n"
                  "       %s families\n",
                  program, program, program);
}

static bool report_parse(const char *name, const char *text,
                         enum cm_parse_status status, uintmax_t min,
                         uintmax_t max)
{
    if (status == CM_PARSE_OK)
    {
        return true;
    }
    (void)fprintf(stderr,
                  "pmu_native: %s \"%s\" rejected (%s; accepted %" PRIuMAX
                  "..%" PRIuMAX ")\n",
                  name, text, cm_parse_status_name(status), min, max);
    return false;
}

static bool parse_unsigned(const char *name, const char *text, unsigned min,
                           unsigned max, unsigned *value)
{
    return report_parse(name, text, cm_parse_unsigned(text, min, max, value),
                        min, max);
}

static bool parse_size(const char *name, const char *text, size_t min,
                       size_t max, size_t *value)
{
    return report_parse(name, text, cm_parse_size(text, min, max, value), min,
                        max);
}

static bool parse_mode(const char *text, enum native_mode *mode)
{
    static const char *const names[] = {"count", "work", "sample", "check"};

    for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i)
    {
        if (strcmp(text, names[i]) == 0)
        {
            *mode = (enum native_mode)i;
            return true;
        }
    }
    (void)fprintf(stderr, "pmu_native: unknown mode \"%s\"\n", text);
    return false;
}

static bool parse_drain(const char *text, enum pf_drain_mode *drain)
{
    if (strcmp(text, "final") == 0)
    {
        *drain = PF_DRAIN_FINAL;
        return true;
    }
    if (strcmp(text, "concurrent") == 0)
    {
        *drain = PF_DRAIN_CONCURRENT;
        return true;
    }
    (void)fprintf(stderr,
                  "pmu_native: --drain \"%s\" is not final|concurrent\n", text);
    return false;
}

static bool parse_pages(const char *text, unsigned *pages)
{
    if (!parse_unsigned("--pages", text, 1, MAX_PAGES, pages))
    {
        return false;
    }
    if ((*pages & (*pages - 1u)) != 0u)
    {
        (void)fprintf(stderr, "pmu_native: --pages %u is not a power of two\n",
                      *pages);
        return false;
    }
    return true;
}

static bool parse_period(const char *text, uint64_t *period)
{
    uintmax_t value = 0;

    if (!report_parse(
            "--period", text,
            cm_parse_umax(text, MIN_PERIOD_CYCLES, MAX_PERIOD_CYCLES, &value),
            MIN_PERIOD_CYCLES, MAX_PERIOD_CYCLES))
    {
        return false;
    }
    *period = (uint64_t)value; /* value <= MAX_PERIOD_CYCLES */
    return true;
}

/* One --name=value option. Each name may appear once; `seen` records which. */
static bool parse_option(const char *argument,
                         struct pf_sampling_config *config, unsigned *seen)
{
    static const char *const names[] = {
        "--drain=", "--pages=", "--period=", "--retain="};

    for (unsigned i = 0; i < sizeof names / sizeof names[0]; ++i)
    {
        size_t length = strlen(names[i]);
        const char *value = argument + length;

        if (strncmp(argument, names[i], length) != 0)
        {
            continue;
        }
        if ((*seen & (1u << i)) != 0u)
        {
            (void)fprintf(stderr, "pmu_native: %.*s given twice\n",
                          (int)(length - 1u), names[i]);
            return false;
        }
        *seen |= 1u << i;
        switch (i)
        {
            case 0:
                return parse_drain(value, &config->drain);
            case 1:
                return parse_pages(value, &config->data_pages);
            case 2:
                return parse_period(value, &config->sample_period_cycles);
            default:
                return parse_size("--retain", value, 0, MAX_RETAIN,
                                  &config->sample_capacity);
        }
    }
    (void)fprintf(stderr, "pmu_native: unknown option \"%s\"\n", argument);
    return false;
}

/* FAMILY N VARIANT [REPETITIONS [PRECISION]]: `count` arguments. */
static bool parse_positional(char **arguments, size_t count,
                             struct native_request *request)
{
    struct pf_request_summary *summary = &request->summary;
    size_t limit =
        (request->mode == MODE_SAMPLE || request->mode == MODE_CHECK) ? 5u : 4u;

    if ((count < 3u) || (count > limit))
    {
        (void)fputs("pmu_native: wrong number of arguments\n", stderr);
        return false;
    }
    if (!pf_family_lookup(arguments[0], &summary->family))
    {
        (void)fprintf(stderr, "pmu_native: unknown family \"%s\"\n",
                      arguments[0]);
        return false;
    }
    /* The family's own bound applies even when it is not built in, so an
     * invalid request is invalid on every build. */
    size_t max_n = pf_family_describe(summary->family)->max_n;
    return parse_size("N", arguments[1], 1, max_n, &summary->n) &&
           parse_unsigned("VARIANT", arguments[2], 0, PF_WORKLOAD_VARIANTS - 1u,
                          &summary->variant) &&
           ((count < 4u) || parse_unsigned("REPETITIONS", arguments[3], 1,
                                           PF_WORKLOAD_MAX_REPETITIONS,
                                           &summary->repetitions)) &&
           ((count < 5u) ||
            parse_unsigned("PRECISION", arguments[4], 0, MAX_PRECISION,
                           &request->sampling.precise_ip));
}

static bool parse_request(int argc, char **argv, struct native_request *request)
{
    *request = (struct native_request){
        .summary = {.repetitions = DEFAULT_REPETITIONS},
        .sampling = {.sample_period_cycles = DEFAULT_PERIOD_CYCLES,
                     .data_pages = DEFAULT_PAGES,
                     .drain = PF_DRAIN_FINAL,
                     .poll_interval_ms = POLL_INTERVAL_MS,
                     .request_lost_count = true,
                     .sample_capacity = DEFAULT_RETAIN},
    };
    if ((argc < 2) || !parse_mode(argv[1], &request->mode))
    {
        return false;
    }
    /* Positional arguments first, then options; never interleaved. */
    size_t positional = 0;
    while ((positional + 2u < (size_t)argc) &&
           (strncmp(argv[positional + 2u], "--", 2) != 0))
    {
        ++positional;
    }
    if (!parse_positional(argv + 2, positional, request))
    {
        return false;
    }
    bool takes_options =
        (request->mode == MODE_SAMPLE) || (request->mode == MODE_CHECK);
    unsigned seen = 0;
    for (size_t i = positional + 2u; i < (size_t)argc; ++i)
    {
        if (!takes_options)
        {
            (void)fputs("pmu_native: options apply to sample|check only\n",
                        stderr);
            return false;
        }
        if ((strncmp(argv[i], "--", 2) != 0) ||
            !parse_option(argv[i], &request->sampling, &seen))
        {
            return false;
        }
    }
    return true;
}

/* Close the request bracket: report a disagreement with the verified
 * checksum as an assertion, never as a valid observation. */
static int verdict(enum pf_workload_status status, uint64_t checksum,
                   uint64_t expected)
{
    observable ^= checksum;
    if ((status != PF_WORKLOAD_OK) || (checksum != expected))
    {
        (void)fputs("PF_NATIVE_ASSERT:workload checksum\n", stderr);
        return EXIT_ASSERT;
    }
    return EXIT_SUCCESS;
}

/* The measured interval itself: the batch between two clock reads, with
 * the CPU recorded at both ends. Returns false if a clock read failed. */
static bool run_timed(struct pf_workload *workload,
                      const struct pf_request_summary *summary,
                      struct pf_count_report *report,
                      enum pf_workload_status *status)
{
    uint64_t start = 0;
    uint64_t end = 0;

    report->cpu_before = sched_getcpu();
    bool timed = cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &start);
    *status = pf_workload_run(workload, summary->variant, summary->repetitions,
                              &report->checksum);
    timed = cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &end) && timed;
    report->cpu_after = sched_getcpu();
    report->elapsed_ns = end - start;
    return timed;
}

/* count: the strict group (pmu.h) brackets the batch. work: perf stat's
 * control pipe does (pmu_control.h). The clock brackets sit inside either. */
static int measure_count(const struct native_request *request,
                         struct pf_workload *workload, uint64_t expected)
{
    static const struct pf_pmu_event events[] = {
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS},
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES}};
    struct cm_clock_calibration clock;
    struct pf_pmu group;
    struct pf_count_report report = {.request = request->summary,
                                     .state = PF_PMU_EMPTY,
                                     .strict_group =
                                         (request->mode == MODE_COUNT),
                                     .group = &group,
                                     .clock = &clock};
    enum pf_workload_status status = PF_WORKLOAD_FAILED;
    bool count = (request->mode == MODE_COUNT);

    memset(&group, 0, sizeof group);
    if (!cm_clock_calibrate(CM_CLOCK_MONOTONIC_RAW, &clock))
    {
        perror("clock");
        return EXIT_INVALID;
    }
    if (count)
    {
        report.state = pf_pmu_open(&group, pf_pmu_linux(), events, 2);
        if (report.state == PF_PMU_READY)
        {
            report.state = pf_pmu_start(&group);
        }
    }
    else if (pf_perf_control(1) != 0)
    {
        perror("perf-control enable");
        return EXIT_INVALID;
    }
    bool timed = run_timed(workload, &request->summary, &report, &status);
    if (count && (report.state == PF_PMU_ACTIVE))
    {
        report.state = pf_pmu_stop(&group);
    }
    else if (!count && (pf_perf_control(0) != 0))
    {
        perror("perf-control disable");
        return EXIT_INVALID;
    }
    int result = verdict(status, report.checksum, expected);
    if (timed)
    {
        pf_report_count(stdout, &report);
    }
    else
    {
        perror("clock");
        result = EXIT_INVALID;
    }
    pf_pmu_close(&group);
    return result;
}

struct sampled_batch
{
    struct pf_workload *workload;
    const struct pf_request_summary *summary;
    bool ran;
    enum pf_workload_status status;
    uint64_t checksum;
};

static void run_sampled_batch(void *context)
{
    struct sampled_batch *batch = context;

    batch->status =
        pf_workload_run(batch->workload, batch->summary->variant,
                        batch->summary->repetitions, &batch->checksum);
    batch->ran = true;
}

/* The executable mappings let a reader resolve sampled addresses later. */
static void report_mappings(void)
{
    char line[4096];
    FILE *maps = fopen("/proc/self/maps", "r");

    if (maps == NULL)
    {
        return;
    }
    while (fgets(line, sizeof line, maps) != NULL)
    {
        pf_report_mapping(stdout, line);
    }
    (void)fclose(maps);
}

/* One positive decimal value from a one-line kernel file. */
static bool read_decimal_file(const char *path, uint64_t *value)
{
    char text[32];
    uintmax_t parsed = 0;
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return false;
    }
    bool read = (fgets(text, sizeof text, file) != NULL);
    (void)fclose(file);
    if (!read)
    {
        return false;
    }
    text[strcspn(text, "\n")] = '\0';
    if (cm_parse_umax(text, 1, UINT32_MAX, &parsed) != CM_PARSE_OK)
    {
        return false;
    }
    *value = (uint64_t)parsed;
    return true;
}

/* The host's throttle floor for the requested period (pmu_report.h). */
static struct pf_period_floor period_floor(void)
{
    struct pf_period_floor floor = {0};

    floor.known =
        read_decimal_file("/proc/sys/kernel/perf_event_max_sample_rate",
                          &floor.max_sample_rate) &&
        read_decimal_file("/sys/devices/system/cpu/cpu0/cpufreq/"
                          "cpuinfo_max_freq",
                          &floor.cpu_max_khz);
    if (floor.known)
    {
        /* <= 4294967295 kHz * 1000: no overflow in 64 bits. */
        uint64_t hz = floor.cpu_max_khz * UINT64_C(1000);

        floor.floor_cycles =
            (hz + floor.max_sample_rate - 1u) / floor.max_sample_rate;
    }
    return floor;
}

/* Exit status of a sampling run: only the parser (pf_ring_drain) is the
 * learner's component; open and map refusals are observations, every other
 * failed stage is the host or the supplied adapter. */
static int sampling_verdict(const struct pf_sampling_result *result)
{
    switch (result->stage)
    {
        case PF_SAMPLING_COMPLETE:
        case PF_SAMPLING_OPEN:
        case PF_SAMPLING_MAP:
            return EXIT_SUCCESS;
        case PF_SAMPLING_PARSER:
            (void)fprintf(stderr,
                          "PF_NATIVE_ASSERT:ring parser pf_ring_drain stopped "
                          "on the kernel's records (status %d)\n",
                          result->parser_status);
            return EXIT_ASSERT;
        default:
            (void)fprintf(stderr, "pmu_native: sampling failed at stage %s\n",
                          pf_sampling_stage_name(result->stage));
            return EXIT_INVALID;
    }
}

static int measure_samples(const struct native_request *request,
                           struct pf_workload *workload, uint64_t expected)
{
    struct pf_period_floor floor = period_floor();

    size_t capacity = request->sampling.sample_capacity;
    struct pf_sample *items = NULL;

    if (capacity > 0u)
    {
        items = calloc(capacity, sizeof *items);
        if (items == NULL)
        {
            perror("sample buffer");
            return EXIT_INVALID;
        }
    }
    report_mappings();
    struct sampled_batch batch = {.workload = workload,
                                  .summary = &request->summary};
    struct pf_sampling_result result = {.samples = {.items = items}};
    pf_sampling_run(&request->sampling, run_sampled_batch, &batch, &result);

    /* Serialization happens only now, after the event is disabled. */
    for (size_t i = 0; i < result.samples.count; ++i)
    {
        pf_report_sample(stdout, &items[i]);
    }
    struct pf_sampling_report report = {
        .request = request->summary,
        .workload_ran = batch.ran,
        .checksum = batch.checksum,
        .config = &request->sampling,
        .result = &result,
        .floor = &floor,
    };
    pf_report_sampling(stdout, &report);
    free(items);
    if (batch.ran && (verdict(batch.status, batch.checksum, expected) != 0))
    {
        return EXIT_ASSERT;
    }
    return sampling_verdict(&result);
}

static int list_families(void)
{
    for (unsigned i = 0; i < PF_FAMILY_COUNT; ++i)
    {
        pf_report_family(stdout, (enum pf_family)i);
    }
    return EXIT_SUCCESS;
}

/* Build the inputs, verify both variants against the oracle outside any
 * measured interval, warm the requested variant once, then measure. */
static int run_request(const struct native_request *request)
{
    struct pf_workload *workload = NULL;
    uint64_t expected = 0;
    enum pf_workload_status status = pf_workload_create(
        request->summary.family, request->summary.n, &workload);

    if (status != PF_WORKLOAD_OK)
    {
        (void)fprintf(stderr, "pmu_native: workload %s\n",
                      pf_workload_status_name(status));
        return EXIT_INVALID;
    }
    status =
        pf_workload_verify(workload, request->summary.repetitions, &expected);
    int result = EXIT_ASSERT;
    if (status != PF_WORKLOAD_OK)
    {
        (void)fprintf(stderr,
                      "PF_NATIVE_ASSERT:equivalent results and independent "
                      "oracle (%s)\n",
                      pf_workload_status_name(status));
    }
    else if (request->mode == MODE_CHECK)
    {
        struct pf_period_floor floor = period_floor();

        pf_report_check(stdout, &request->summary, expected, &request->sampling,
                        &floor);
        result = EXIT_SUCCESS;
    }
    else
    {
        uint64_t warm = 0;

        status = pf_workload_run(workload, request->summary.variant, 1, &warm);
        observable ^= warm;
        if (status != PF_WORKLOAD_OK)
        {
            (void)fputs("PF_NATIVE_ASSERT:warm-up run\n", stderr);
        }
        else if (request->mode == MODE_SAMPLE)
        {
            result = measure_samples(request, workload, expected);
        }
        else
        {
            result = measure_count(request, workload, expected);
        }
    }
    pf_workload_destroy(workload);
    return result;
}

static int dispatch(int argc, char **argv)
{
    struct native_request request;

    if ((argc >= 2) && (strcmp(argv[1], "families") == 0))
    {
        if (argc != 2)
        {
            usage(argv[0]);
            return EXIT_INVALID;
        }
        return list_families();
    }
    if (!parse_request(argc, argv, &request))
    {
        usage((argc > 0) ? argv[0] : "pmu_native");
        return EXIT_INVALID;
    }
    const struct pf_family_info *info =
        pf_family_describe(request.summary.family);
    if (!info->available)
    {
        pf_report_family(stdout, request.summary.family);
        (void)fprintf(stderr, "pmu_native: family %s needs the %s\n",
                      info->name, info->needs);
        return EXIT_UNAVAILABLE;
    }
    return run_request(&request);
}

int main(int argc, char **argv)
{
    int result = dispatch(argc, argv);

    /* An observation that did not reach its reader was not reported. */
    if ((fflush(stdout) != 0) || (ferror(stdout) != 0))
    {
        return EXIT_INVALID;
    }
    return result;
}
