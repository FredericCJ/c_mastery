/*
 * Report a clock's resolution and read cost as one JSON line, so a timing
 * gate can be computed for a benchmark that does not report them itself.
 *
 *     cm_clock_probe monotonic|monotonic-raw
 *
 * Exit status: 0 printed, 2 unknown clock name or failed clock call.
 */
#include "cm_clock.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    enum cm_clock_id clock = CM_CLOCK_MONOTONIC;
    struct cm_clock_calibration calibration;

    if ((argc != 2) || !cm_clock_lookup(argv[1], &clock))
    {
        (void)fputs("usage: cm_clock_probe monotonic|monotonic-raw\n", stderr);
        return 2;
    }
    if (!cm_clock_calibrate(clock, &calibration))
    {
        (void)fputs("cm_clock_probe: clock unavailable\n", stderr);
        return 2;
    }
    (void)printf(
        "{\"kind\":\"clock\",\"clock\":\"%s\",\"resolution_ns\":%" PRIu64
        ",\"overhead_max_ns\":%" PRIu64 ",\"brackets_ns\":[",
        cm_clock_name(clock), calibration.resolution_ns,
        calibration.overhead_max_ns);
    for (size_t i = 0; i < CM_CLOCK_BRACKETS; ++i)
    {
        (void)printf("%s%" PRIu64, (i == 0u) ? "" : ",",
                     calibration.brackets_ns[i]);
    }
    (void)puts("]}");
    return EXIT_SUCCESS;
}
