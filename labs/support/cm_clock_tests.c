/*
 * Tests for the clock helper (cm_clock.h). The clock values themselves are
 * host observations; the tests check only the contract: names round-trip,
 * readings never go backwards, and a calibration's maximum really is the
 * largest bracket it measured.
 */
#include "cm_clock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned failure_count;

#define CM_EXPECT(condition)                                           \
    do                                                                 \
    {                                                                  \
        if (!(condition))                                              \
        {                                                              \
            (void)fprintf(stderr, "CM_CLOCK_ASSERT:%d:%s\n", __LINE__, \
                          #condition);                                 \
            failure_count++;                                           \
        }                                                              \
    } while (0)

static void test_accepted_names(void)
{
    enum cm_clock_id clock = CM_CLOCK_MONOTONIC_RAW;

    CM_EXPECT(cm_clock_lookup("monotonic", &clock));
    CM_EXPECT(clock == CM_CLOCK_MONOTONIC);
    CM_EXPECT(cm_clock_lookup("monotonic-raw", &clock));
    CM_EXPECT(clock == CM_CLOCK_MONOTONIC_RAW);
    CM_EXPECT(strcmp(cm_clock_name(CM_CLOCK_MONOTONIC), "monotonic") == 0);
    CM_EXPECT(strcmp(cm_clock_name(CM_CLOCK_MONOTONIC_RAW), "monotonic-raw") ==
              0);
}

static void test_rejected_names(void)
{
    static const char *const rejected[] = {
        "", "realtime", "MONOTONIC", "monotonic ", "raw", "monotonic-"};
    enum cm_clock_id clock = CM_CLOCK_MONOTONIC_RAW;

    for (size_t i = 0; i < sizeof rejected / sizeof rejected[0]; ++i)
    {
        clock = CM_CLOCK_MONOTONIC_RAW;
        CM_EXPECT(!cm_clock_lookup(rejected[i], &clock));
        CM_EXPECT(clock == CM_CLOCK_MONOTONIC_RAW); /* left unchanged */
    }
    CM_EXPECT(!cm_clock_lookup(NULL, &clock));
}

static void test_monotonic(enum cm_clock_id clock)
{
    uint64_t previous = 0;

    CM_EXPECT(cm_clock_now_ns(clock, &previous));
    for (unsigned i = 0; i < 10000u; ++i)
    {
        uint64_t now = 0;

        CM_EXPECT(cm_clock_now_ns(clock, &now));
        CM_EXPECT(now >= previous);
        previous = now;
    }
}

static void test_calibration(enum cm_clock_id clock)
{
    struct cm_clock_calibration calibration;
    uint64_t largest = 0;

    memset(&calibration, 0xff, sizeof calibration);
    CM_EXPECT(cm_clock_calibrate(clock, &calibration));
    CM_EXPECT(calibration.resolution_ns > 0u);
    for (size_t i = 0; i < CM_CLOCK_BRACKETS; ++i)
    {
        if (calibration.brackets_ns[i] > largest)
        {
            largest = calibration.brackets_ns[i];
        }
    }
    CM_EXPECT(calibration.overhead_max_ns == largest);
    /* A back-to-back pair of reads costs less than a second on any host
     * this course supports; a larger value means an unsigned wrap. */
    CM_EXPECT(largest < UINT64_C(1000000000));
}

int main(void)
{
    test_accepted_names();
    test_rejected_names();
    test_monotonic(CM_CLOCK_MONOTONIC);
    test_monotonic(CM_CLOCK_MONOTONIC_RAW);
    test_calibration(CM_CLOCK_MONOTONIC);
    test_calibration(CM_CLOCK_MONOTONIC_RAW);
    if (failure_count != 0u)
    {
        (void)fprintf(stderr, "CM_CLOCK_FAIL:%u\n", failure_count);
        return EXIT_FAILURE;
    }
    (void)puts("CM_CLOCK_PASS");
    return EXIT_SUCCESS;
}
