/*
 * JSON-lines serialization for the native collector. Contract:
 * pmu_report.h. Output errors are detected once, by the caller's final
 * fflush/ferror check on the stream.
 */
#include "pmu_report.h"

#include <inttypes.h>

static void put_request(FILE *stream, const struct pf_request_summary *request)
{
    (void)fprintf(stream,
                  "\"family\":\"%s\",\"n\":%zu,\"variant\":%u,"
                  "\"repetitions\":%u",
                  pf_family_describe(request->family)->name, request->n,
                  request->variant, request->repetitions);
}

/* A quantity that may be unknown: JSON null, never a stand-in zero. */
static void put_optional(FILE *stream, const char *name, bool known,
                         uint64_t value)
{
    if (known)
    {
        (void)fprintf(stream, ",\"%s\":%" PRIu64, name, value);
    }
    else
    {
        (void)fprintf(stream, ",\"%s\":null", name);
    }
}

static const char *boolean(bool value)
{
    return value ? "true" : "false";
}

void pf_report_count(FILE *stream, const struct pf_count_report *report)
{
    const struct pf_pmu *group = report->group;
    const struct cm_clock_calibration *clock = report->clock;

    (void)fputs("{\"kind\":\"count\",", stream);
    put_request(stream, &report->request);
    (void)fprintf(stream,
                  ",\"checksum\":%" PRIu64 ",\"elapsed_ns\":%" PRIu64
                  ",\"cpu_before\":%d,\"cpu_after\":%d,\"status\":\"%s\","
                  "\"errno\":%d,\"cleanup_errno\":%d",
                  report->checksum, report->elapsed_ns, report->cpu_before,
                  report->cpu_after, pf_pmu_state_name(report->state),
                  group->error, group->cleanup_error);
    (void)fprintf(stream,
                  ",\"enabled\":%" PRIu64 ",\"running\":%" PRIu64
                  ",\"raw_instructions\":%" PRIu64 ",\"raw_cycles\":%" PRIu64
                  ",\"instruction_id\":%" PRIu64 ",\"cycle_id\":%" PRIu64,
                  group->enabled, group->running, group->raw[0], group->raw[1],
                  group->ids[0], group->ids[1]);
    (void)fprintf(stream,
                  ",\"scope\":\"calling-thread-user\",\"strict_group\":%s,"
                  "\"acquisition\":\"%s\","
                  "\"clock\":\"%s\",\"clock_resolution_ns\":%" PRIu64
                  ",\"clock_overhead_max_ns\":%" PRIu64
                  ",\"clock_brackets_ns\":[",
                  boolean(report->strict_group),
                  report->strict_group ? "strict-group" : "perf-control",
                  cm_clock_name(CM_CLOCK_MONOTONIC_RAW), clock->resolution_ns,
                  clock->overhead_max_ns);
    for (size_t i = 0; i < CM_CLOCK_BRACKETS; ++i)
    {
        (void)fprintf(stream, "%s%" PRIu64, (i == 0u) ? "" : ",",
                      clock->brackets_ns[i]);
    }
    (void)fputs("]}\n", stream);
}

static void put_loss(FILE *stream, const struct pf_sampling_result *result)
{
    const struct pf_loss_account *loss = &result->loss;
    bool complete = (loss->accounting == PF_LOSS_COMPLETE);

    /* `lost` keeps its original meaning (in-band PERF_RECORD_LOST total)
     * for readers of earlier receipts; inband_lost is the explicit name. */
    (void)fprintf(stream,
                  ",\"lost\":%" PRIu64 ",\"inband_lost\":%" PRIu64
                  ",\"hardware_lost\":%" PRIu64,
                  result->ring.lost, result->ring.lost,
                  result->ring.hardware_lost);
    put_optional(stream, "kernel_lost", result->reading.lost_known,
                 result->reading.lost);
    (void)fprintf(stream,
                  ",\"lost_support\":\"%s\",\"loss_accounting\":\"%s\","
                  "\"lost_lower_bound\":%" PRIu64,
                  pf_lost_support_name(result->lost_support),
                  pf_loss_accounting_name(loss->accounting), loss->lower_bound);
    put_optional(stream, "total_lost", complete, loss->total_lost);
    put_optional(stream, "unreported_lost", complete, loss->unreported_lost);
    (void)fprintf(stream,
                  ",\"throttled\":%s,\"throttle_records\":%" PRIu64
                  ",\"unthrottle_records\":%" PRIu64 ",\"lossless\":%s",
                  boolean(loss->throttled), result->kinds.throttle,
                  result->kinds.unthrottle, boolean(loss->lossless));
}

static void put_floor(FILE *stream, const struct pf_period_floor *floor,
                      uint64_t period)
{
    put_optional(stream, "max_sample_rate", floor->known,
                 floor->max_sample_rate);
    put_optional(stream, "cpu_max_khz", floor->known, floor->cpu_max_khz);
    put_optional(stream, "period_floor_cycles", floor->known,
                 floor->floor_cycles);
    if (floor->known)
    {
        (void)fprintf(stream, ",\"throttle_bound\":%s",
                      boolean(period < floor->floor_cycles));
    }
    else
    {
        (void)fputs(",\"throttle_bound\":null", stream);
    }
}

void pf_report_sampling(FILE *stream, const struct pf_sampling_report *report)
{
    const struct pf_sampling_result *result = report->result;
    const struct pf_sampling_config *config = report->config;

    (void)fprintf(stream,
                  "{\"kind\":\"sampling\",\"status\":\"%s\",\"stage\":\"%s\","
                  "\"errno\":%d,\"control_errno\":%d,\"cleanup_errno\":%d",
                  result->status, pf_sampling_stage_name(result->stage),
                  result->open_errno, result->control_errno,
                  result->cleanup_errno);
    /* parser_status: the learner parser's last result, null if never run. */
    put_optional(stream, "parser_status", result->parser_ran,
                 (uint64_t)result->parser_status);
    (void)fputc(',', stream);
    put_request(stream, &report->request);
    (void)fprintf(stream, ",\"workload_ran\":%s",
                  boolean(report->workload_ran));
    put_optional(stream, "checksum", report->workload_ran, report->checksum);
    (void)fprintf(stream,
                  ",\"samples\":%" PRIu64 ",\"retained\":%zu"
                  ",\"not_retained\":%" PRIu64,
                  result->ring.samples, result->samples.count,
                  result->samples.not_retained);
    put_loss(stream, result);
    (void)fprintf(
        stream,
        ",\"unknown\":%" PRIu64 ",\"records\":%" PRIu64 ",\"enabled\":%" PRIu64
        ",\"running\":%" PRIu64 ",\"event_count\":%" PRIu64,
        result->ring.unknown, result->ring.records, result->reading.enabled_ns,
        result->reading.running_ns, result->reading.value);
    (void)fprintf(stream,
                  ",\"precise_ip\":%u,\"drain_mode\":\"%s\",\"data_pages\":%u,"
                  "\"sample_period\":%" PRIu64 ",\"drains\":%" PRIu64,
                  config->precise_ip, pf_drain_mode_name(config->drain),
                  config->data_pages, config->sample_period_cycles,
                  result->drains);
    put_floor(stream, report->floor, config->sample_period_cycles);
    (void)fputs(",\"scope\":\"calling-thread-user\"}\n", stream);
}

void pf_report_sample(FILE *stream, const struct pf_sample *sample)
{
    (void)fprintf(stream,
                  "{\"kind\":\"sample\",\"ip\":%" PRIu64
                  ",\"pid\":%u,\"tid\":%u,\"time\":%" PRIu64
                  ",\"period\":%" PRIu64 "}\n",
                  sample->ip, (unsigned)sample->pid, (unsigned)sample->tid,
                  sample->time, sample->period);
}

void pf_report_mapping(FILE *stream, const char *line)
{
    (void)fputs("{\"kind\":\"mapping\",\"line\":\"", stream);
    for (const char *cursor = line; *cursor != '\0'; ++cursor)
    {
        unsigned char byte = (unsigned char)*cursor;

        if ((byte == '\\') || (byte == '"'))
        {
            (void)fputc('\\', stream);
            (void)fputc(byte, stream);
        }
        else if (byte >= 32u)
        {
            (void)fputc(byte, stream);
        }
        else
        {
            /* The newline ends the record; other control bytes cannot
             * appear in a mapping and are dropped rather than emitted. */
        }
    }
    (void)fputs("\"}\n", stream);
}

void pf_report_family(FILE *stream, enum pf_family family)
{
    const struct pf_family_info *info = pf_family_describe(family);

    (void)fprintf(stream,
                  "{\"kind\":\"family\",\"family\":\"%s\",\"available\":%s,"
                  "\"max_n\":%zu,\"requires\":",
                  info->name, boolean(info->available), info->max_n);
    if (info->needs != NULL)
    {
        (void)fprintf(stream, "\"%s\"}\n", info->needs);
    }
    else
    {
        (void)fputs("null}\n", stream);
    }
}

void pf_report_check(FILE *stream, const struct pf_request_summary *request,
                     uint64_t checksum,
                     const struct pf_sampling_config *sampling,
                     const struct pf_period_floor *floor)
{
    (void)fputs("{\"kind\":\"check\",\"status\":\"verified\",", stream);
    put_request(stream, request);
    (void)fprintf(
        stream,
        ",\"checksum\":%" PRIu64 ",\"precise_ip\":%u,\"drain_mode\":\"%s\","
        "\"data_pages\":%u,\"sample_period\":%" PRIu64 ",\"retain\":%zu",
        checksum, sampling->precise_ip, pf_drain_mode_name(sampling->drain),
        sampling->data_pages, sampling->sample_period_cycles,
        sampling->sample_capacity);
    put_floor(stream, floor, sampling->sample_period_cycles);
    (void)fputs(",\"scope\":\"no perf event opened\"}\n", stream);
}
