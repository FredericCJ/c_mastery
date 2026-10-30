/*
 * Monotonic clock readings and their cost. Contract: cm_clock.h.
 */
#define _GNU_SOURCE /* CLOCK_MONOTONIC_RAW is a Linux clock */
#include "cm_clock.h"

#include <string.h>
#include <time.h>

static clockid_t posix_clock(enum cm_clock_id clock)
{
    return (clock == CM_CLOCK_MONOTONIC_RAW) ? CLOCK_MONOTONIC_RAW
                                             : CLOCK_MONOTONIC;
}

/* A monotonic clock never reports a negative time, and seconds since boot
 * times 1e9 fits in 64 bits for about 584 years. */
static bool to_ns(const struct timespec *time, uint64_t *ns)
{
    if ((time->tv_sec < 0) || (time->tv_nsec < 0) ||
        (time->tv_nsec >= 1000000000L))
    {
        return false;
    }
    *ns =
        (uint64_t)time->tv_sec * UINT64_C(1000000000) + (uint64_t)time->tv_nsec;
    return true;
}

bool cm_clock_now_ns(enum cm_clock_id clock, uint64_t *now_ns)
{
    struct timespec time;

    if (clock_gettime(posix_clock(clock), &time) != 0)
    {
        return false;
    }
    return to_ns(&time, now_ns);
}

bool cm_clock_calibrate(enum cm_clock_id clock,
                        struct cm_clock_calibration *calibration)
{
    struct timespec resolution;

    if ((clock_getres(posix_clock(clock), &resolution) != 0) ||
        !to_ns(&resolution, &calibration->resolution_ns))
    {
        return false;
    }
    calibration->overhead_max_ns = 0;
    for (size_t i = 0; i < CM_CLOCK_BRACKETS; ++i)
    {
        uint64_t begin = 0;
        uint64_t end = 0;

        if (!cm_clock_now_ns(clock, &begin) || !cm_clock_now_ns(clock, &end))
        {
            return false;
        }
        calibration->brackets_ns[i] = end - begin; /* monotonic: end >= begin */
        if (calibration->brackets_ns[i] > calibration->overhead_max_ns)
        {
            calibration->overhead_max_ns = calibration->brackets_ns[i];
        }
    }
    return true;
}

bool cm_clock_lookup(const char *name, enum cm_clock_id *clock)
{
    if (name == NULL)
    {
        return false;
    }
    if (strcmp(name, "monotonic") == 0)
    {
        *clock = CM_CLOCK_MONOTONIC;
        return true;
    }
    if (strcmp(name, "monotonic-raw") == 0)
    {
        *clock = CM_CLOCK_MONOTONIC_RAW;
        return true;
    }
    return false;
}

const char *cm_clock_name(enum cm_clock_id clock)
{
    switch (clock)
    {
        case CM_CLOCK_MONOTONIC:
            return "monotonic";
        case CM_CLOCK_MONOTONIC_RAW:
            return "monotonic-raw";
        default:
            return "invalid";
    }
}
