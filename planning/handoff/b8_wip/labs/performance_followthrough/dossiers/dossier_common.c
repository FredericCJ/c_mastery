/*
 * Shared dossier support. Contract: dossier_common.h.
 */
#define _GNU_SOURCE /* CPU_SET, sched_getcpu, pthread_*affinity_np */
#include "dossier_common.h"

#include "cm_parse.h"

#include <errno.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <sched.h>
#include <string.h>
#include <sys/resource.h>

/* The calling thread's context-switch counts (Linux RUSAGE_THREAD). */
static bool read_switches(long *involuntary, long *voluntary)
{
    struct rusage usage;

    if (getrusage(RUSAGE_THREAD, &usage) != 0)
    {
        return false;
    }
    *involuntary = usage.ru_nivcsw;
    *voluntary = usage.ru_nvcsw;
    return true;
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
                  "dossier: %s \"%s\" rejected (%s; accepted %" PRIuMAX
                  "..%" PRIuMAX ")\n",
                  name, (text != NULL) ? text : "(null)",
                  cm_parse_status_name(status), min, max);
    return false;
}

bool pfd_parse_unsigned(const char *name, const char *text, unsigned min,
                        unsigned max, unsigned *value)
{
    return report_parse(name, text, cm_parse_unsigned(text, min, max, value),
                        min, max);
}

bool pfd_parse_u64(const char *name, const char *text, uint64_t min,
                   uint64_t max, uint64_t *value)
{
    uintmax_t parsed = 0;

    if (!report_parse(name, text, cm_parse_umax(text, min, max, &parsed), min,
                      max))
    {
        return false;
    }
    *value = (uint64_t)parsed; /* parsed <= max <= UINT64_MAX: exact */
    return true;
}

bool pfd_parse_events(char *const *texts, size_t count,
                      struct pfd_events *events)
{
    if (count > PFD_MAX_EVENTS)
    {
        (void)fprintf(stderr,
                      "dossier: %zu event configurations given; at most %u\n",
                      count, (unsigned)PFD_MAX_EVENTS);
        return false;
    }
    events->count = 0;
    for (size_t i = 0; i < count; ++i)
    {
        uint64_t config = 0;

        if (!pfd_parse_u64("CONFIG", texts[i], 0, UINT64_MAX, &config))
        {
            return false;
        }
        events->events[i] =
            (struct pf_pmu_event){.type = PERF_TYPE_RAW, .config = config};
    }
    events->count = count;
    return true;
}

bool pfd_allowed_capture(struct pfd_allowed *allowed)
{
    cpu_set_t current;

    memset(allowed, 0, sizeof *allowed);
    CPU_ZERO(&current);
    if (sched_getaffinity(0, sizeof current, &current) != 0)
    {
        return false;
    }
    for (unsigned cpu = 0; (cpu <= PFD_MAX_CPU) && (cpu < CPU_SETSIZE); ++cpu)
    {
        if (CPU_ISSET(cpu, &current) != 0)
        {
            allowed->bits[cpu / 64u] |= UINT64_C(1) << (cpu % 64u);
        }
    }
    return true;
}

bool pfd_allowed_has(const struct pfd_allowed *allowed, unsigned cpu)
{
    if ((allowed == NULL) || (cpu > PFD_MAX_CPU) || (cpu >= CPU_SETSIZE))
    {
        return false;
    }
    return ((allowed->bits[cpu / 64u] >> (cpu % 64u)) & 1u) != 0u;
}

/* One-CPU set for `cpu`, which the caller has checked is allowed. */
static cpu_set_t single_cpu(unsigned cpu)
{
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    return set;
}

int pfd_pin_calling_thread(const struct pfd_allowed *allowed, unsigned cpu)
{
    if (!pfd_allowed_has(allowed, cpu))
    {
        return EINVAL;
    }
    cpu_set_t set = single_cpu(cpu);
    return pthread_setaffinity_np(pthread_self(), sizeof set, &set);
}

int pfd_place_attr(const struct pfd_allowed *allowed, pthread_attr_t *attr,
                   unsigned cpu)
{
    if (!pfd_allowed_has(allowed, cpu))
    {
        return EINVAL;
    }
    cpu_set_t set = single_cpu(cpu);
    return pthread_attr_setaffinity_np(attr, sizeof set, &set);
}

enum pf_pmu_state pfd_window_open(struct pfd_window *window,
                                  const struct pfd_events *events)
{
    memset(window, 0, sizeof *window);
    window->events = events;
    window->state = PF_PMU_EMPTY;
    window->cpu_start = -1;
    window->cpu_end = -1;
    if ((events != NULL) && (events->count > 0u))
    {
        window->state = pf_pmu_open(&window->group, pf_pmu_linux(),
                                    events->events, events->count);
    }
    return window->state;
}

void pfd_window_begin(struct pfd_window *window)
{
    window->switches_ok =
        read_switches(&window->involuntary_before, &window->voluntary_before);
    if (window->state == PF_PMU_READY)
    {
        window->state = pf_pmu_start(&window->group);
    }
    window->cpu_start = sched_getcpu();
    window->clock_ok =
        cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &window->start_ns);
}

void pfd_window_end(struct pfd_window *window)
{
    bool ended = cm_clock_now_ns(CM_CLOCK_MONOTONIC_RAW, &window->end_ns);

    window->cpu_end = sched_getcpu();
    window->clock_ok = window->clock_ok && ended;
    if (window->state == PF_PMU_ACTIVE)
    {
        window->state = pf_pmu_stop(&window->group);
    }
    window->switches_ok =
        read_switches(&window->involuntary_after, &window->voluntary_after) &&
        window->switches_ok;
}

void pfd_window_close(struct pfd_window *window)
{
    if ((window->events != NULL) && (window->events->count > 0u))
    {
        pf_pmu_close(&window->group);
    }
}

uint64_t pfd_window_elapsed_ns(const struct pfd_window *window)
{
    if (!window->clock_ok || (window->end_ns < window->start_ns))
    {
        return 0;
    }
    return window->end_ns - window->start_ns;
}

void pfd_print_u64_array(FILE *out, const char *key, const uint64_t *values,
                         size_t count)
{
    (void)fprintf(out, "\"%s\":[", key);
    for (size_t i = 0; i < count; ++i)
    {
        (void)fprintf(out, "%s%" PRIu64, (i > 0u) ? "," : "", values[i]);
    }
    (void)fputc(']', out);
}

void pfd_print_window(FILE *out, const char *key,
                      const struct pfd_window *window)
{
    const struct pf_pmu *group = &window->group;
    size_t count = (window->events != NULL) ? window->events->count : 0u;
    uint64_t configs[PFD_MAX_EVENTS] = {0};
    bool measured = (window->state == PF_PMU_MEASURED);

    for (size_t i = 0; i < count; ++i)
    {
        configs[i] = window->events->events[i].config;
    }
    (void)fprintf(out, "\"%s\":{\"state\":\"%s\",", key,
                  pf_pmu_state_name(window->state));
    pfd_print_u64_array(out, "configs", configs, count);
    (void)fputc(',', out);
    /* Counts and IDs are printed only for a complete group read; a refused
     * or never-scheduled group has none to report. */
    pfd_print_u64_array(out, "ids", group->ids, measured ? count : 0u);
    (void)fputc(',', out);
    pfd_print_u64_array(out, "raw", group->raw, measured ? count : 0u);
    (void)fprintf(
        out,
        ",\"enabled\":%" PRIu64 ",\"running\":%" PRIu64
        ",\"error\":%d,\"cleanup_errno\":%d,\"failed_event\":%lld,"
        "\"clock_ok\":%s,\"start_ns\":%" PRIu64 ",\"end_ns\":%" PRIu64
        ",\"elapsed_ns\":%" PRIu64 ",\"cpu_start\":%d,\"cpu_end\":%d,"
        "\"switches_ok\":%s,\"involuntary_switches\":%ld,"
        "\"voluntary_switches\":%ld}",
        measured ? group->enabled : 0u, measured ? group->running : 0u,
        group->error, group->cleanup_error,
        ((window->state == PF_PMU_EMPTY) || (group->failed_event == SIZE_MAX))
            ? -1LL
            : (long long)group->failed_event,
        window->clock_ok ? "true" : "false", window->start_ns, window->end_ns,
        pfd_window_elapsed_ns(window), window->cpu_start, window->cpu_end,
        window->switches_ok ? "true" : "false",
        window->switches_ok
            ? (window->involuntary_after - window->involuntary_before)
            : -1L,
        window->switches_ok
            ? (window->voluntary_after - window->voluntary_before)
            : -1L);
}

void pfd_print_clock(FILE *out, const struct cm_clock_calibration *clock)
{
    (void)fprintf(out, "\"clock_resolution_ns\":%" PRIu64 ",",
                  clock->resolution_ns);
    pfd_print_u64_array(out, "clock_brackets_ns", clock->brackets_ns,
                        CM_CLOCK_BRACKETS);
}
