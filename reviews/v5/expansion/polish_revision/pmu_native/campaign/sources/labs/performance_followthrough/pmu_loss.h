/*
 * Sampling loss accounting: decode a sampling event's read() record and
 * reconcile the loss information from the ring with the kernel's count.
 *
 * Linux reports sample loss in three separate ways (perf_event_open(2),
 * man-pages 6.17):
 *
 * - PERF_RECORD_LOST records in the ring ("in-band"). The kernel writes one
 *   only "when possible": when a later record fits again. A ring that stays
 *   full until the event is disabled never receives the record describing its
 *   final losses, so in-band totals are a lower bound.
 * - The event's lost-sample count, read through read() when the event was
 *   opened with PERF_FORMAT_LOST (Linux 6.0 and later). It counts every
 *   sample the event lost and does not depend on later records fitting.
 * - PERF_RECORD_LOST_SAMPLES records, which report samples dropped by a
 *   hardware sampling mechanism (for example Intel PEBS). These are a
 *   different loss and are reported separately, never added to the others.
 *
 * In-band totals and the kernel count describe overlapping events: an
 * in-band record reports losses the kernel count already includes. They are
 * therefore never summed. When the kernel count is unavailable the total is
 * unknown, not zero.
 */
#ifndef PF_PMU_LOSS_H
#define PF_PMU_LOSS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* One non-group read() record of a sampling event opened with
 * TOTAL_TIME_ENABLED | TOTAL_TIME_RUNNING, optionally with LOST. */
struct pf_sample_reading
{
    uint64_t value;
    uint64_t enabled_ns;
    uint64_t running_ns;
    bool lost_known;
    uint64_t lost;
};

enum pf_reading_status
{
    PF_READING_OK = 0,
    PF_READING_SHORT, /* byte count does not match the requested format */
    PF_READING_MALFORMED /* running time exceeds enabled time */
};

/* Decode `length` bytes returned by read(). `with_lost` states whether
 * PERF_FORMAT_LOST was part of the format the kernel accepted. */
enum pf_reading_status pf_sample_reading_decode(const void *bytes,
                                                size_t length, bool with_lost,
                                                struct pf_sample_reading *out);

struct pf_loss_inputs
{
    uint64_t inband_lost; /* sum of PERF_RECORD_LOST.lost consumed */
    uint64_t inband_lost_records; /* number of PERF_RECORD_LOST records */
    uint64_t hardware_lost; /* sum of PERF_RECORD_LOST_SAMPLES.lost */
    bool kernel_count_known; /* LOST format accepted and read */
    uint64_t kernel_lost; /* valid only when kernel_count_known */
};

enum pf_loss_accounting
{
    PF_LOSS_COMPLETE = 0, /* kernel count available and consistent */
    PF_LOSS_UNKNOWN, /* no kernel count: in-band is only a lower bound */
    PF_LOSS_INCONSISTENT /* in-band exceeds the kernel count */
};

struct pf_loss_account
{
    enum pf_loss_accounting accounting;
    uint64_t lower_bound; /* always valid: max(in-band, kernel if known) */
    uint64_t total_lost; /* valid only when COMPLETE */
    uint64_t
        unreported_lost; /* COMPLETE: lost without a consumed in-band record */
    uint64_t hardware_lost; /* reported separately in every state */
    bool lossless; /* COMPLETE, no ring loss and no hardware loss */
};

void pf_loss_reconcile(const struct pf_loss_inputs *inputs,
                       struct pf_loss_account *account);
const char *pf_loss_accounting_name(enum pf_loss_accounting accounting);

#endif /* PF_PMU_LOSS_H */
