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
 * - The event's lost count, read through read() when the event was opened
 *   with PERF_FORMAT_LOST (Linux 6.0 and later). The kernel increments it
 *   for every record of any type it failed to write into this event's ring
 *   for lack of room (mostly samples, but also, for example, throttle
 *   records). It does not depend on later records fitting. It does not count
 *   samples that were never taken because the event was throttled.
 * - PERF_RECORD_LOST_SAMPLES records, which report samples dropped by a
 *   hardware sampling mechanism (for example Intel PEBS). These are a
 *   different loss and are reported separately, never added to the others.
 *
 * Throttling is a fourth, uncounted gap: when samples arrive faster than
 * perf_event_max_sample_rate allows, the kernel stops sampling until the next
 * tick and records only PERF_RECORD_THROTTLE/UNTHROTTLE. No count says how
 * many samples were skipped, so a throttled observation is never lossless.
 *
 * In-band totals and the kernel count describe overlapping events: an
 * in-band record reports losses the kernel count already includes. They are
 * therefore never summed. When the kernel count is unavailable, or the ring
 * was not parsed to its end, the total is unknown, not zero.
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
    bool ring_parsed; /* every published record was consumed and parsed */
    uint64_t inband_lost; /* sum of PERF_RECORD_LOST.lost consumed */
    uint64_t inband_lost_records; /* number of PERF_RECORD_LOST records */
    uint64_t hardware_lost; /* sum of PERF_RECORD_LOST_SAMPLES.lost */
    bool kernel_count_known; /* LOST format accepted and read */
    uint64_t kernel_lost; /* valid only when kernel_count_known */
    uint64_t throttle_records; /* PERF_RECORD_THROTTLE records consumed */
    uint64_t unknown_records; /* records the parser did not interpret */
};

enum pf_loss_accounting
{
    PF_LOSS_COMPLETE = 0, /* parsed ring, kernel count known and consistent */
    PF_LOSS_UNKNOWN, /* no kernel count or unparsed ring: lower bound only */
    PF_LOSS_INCONSISTENT, /* in-band exceeds the kernel count */
    /* The ring-loss count is complete, but the event was throttled: samples
     * it never took are in no count, so there is no total. */
    PF_LOSS_COMPLETE_EXCEPT_THROTTLED
};

struct pf_loss_account
{
    enum pf_loss_accounting accounting;
    uint64_t lower_bound; /* always valid: max(in-band, kernel if known) */
    uint64_t total_lost; /* valid only when COMPLETE */
    uint64_t
        unreported_lost; /* COMPLETE: lost without a consumed in-band record */
    uint64_t hardware_lost; /* reported separately in every state */
    bool throttled; /* throttle records seen: skipped samples uncounted */
    bool lossless; /* COMPLETE, no ring, hardware, throttling or unknown-record
                      loss. The caller must still withhold it from an
                      observation that measured nothing. */
};

void pf_loss_reconcile(const struct pf_loss_inputs *inputs,
                       struct pf_loss_account *account);
const char *pf_loss_accounting_name(enum pf_loss_accounting accounting);

#endif /* PF_PMU_LOSS_H */
