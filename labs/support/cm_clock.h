/*
 * Monotonic clock readings and the cost of reading them.
 *
 * A measured interval is trustworthy only when it is long compared with the
 * clock's resolution and with the cost of the two reads that bracket it.
 * cm_clock_calibrate measures both on the calling thread so that a caller can
 * report them next to every interval; labs/support/measurement_gates.py turns
 * them into a timing target.
 *
 * Hosted POSIX/Linux only: it uses clock_gettime and clock_getres. The clock
 * is named by a course enumeration rather than a clockid_t so this header
 * needs no feature-test macro; CLOCK_MONOTONIC_RAW (not slewed by NTP) is a
 * Linux clock.
 */
#ifndef CM_CLOCK_H
#define CM_CLOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CM_CLOCK_BRACKETS 31u

enum cm_clock_id
{
    CM_CLOCK_MONOTONIC = 0,
    CM_CLOCK_MONOTONIC_RAW
};

struct cm_clock_calibration
{
    uint64_t resolution_ns;
    uint64_t overhead_max_ns; /* the largest element of brackets_ns */
    uint64_t brackets_ns[CM_CLOCK_BRACKETS]; /* back-to-back read costs */
};

/* Read `clock` in nanoseconds. Returns false, leaving *now_ns unchanged, when
 * the clock cannot be read. Nonblocking. */
bool cm_clock_now_ns(enum cm_clock_id clock, uint64_t *now_ns);

/* Fill *calibration with the clock's resolution and CM_CLOCK_BRACKETS
 * back-to-back read costs. Returns false if any clock call fails; the
 * contents are then unspecified. Nonblocking. */
bool cm_clock_calibrate(enum cm_clock_id clock,
                        struct cm_clock_calibration *calibration);

/* "monotonic" or "monotonic-raw". Returns false, leaving *clock unchanged,
 * for any other text (including NULL). */
bool cm_clock_lookup(const char *name, enum cm_clock_id *clock);
const char *cm_clock_name(enum cm_clock_id clock);

#endif /* CM_CLOCK_H */
