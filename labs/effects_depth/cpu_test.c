/*
 * Times one activity (observer, cpu, wait or threads) with wall-clock, process
 * CPU-time and rusage clocks, and prints one JSON line. Exit status: 2 for a
 * bad argument, 3 when a thread cannot start, 4 when a join fails.
 */
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

#define NANOSECONDS_PER_SECOND 1e9
#define MICROSECONDS_PER_SECOND 1000000L
#define SPIN_ITERATIONS 20000000u
#define WAIT_NS 20000000L

/* Spin loop; the volatile sum keeps the compiler from removing the work. */
static void *work(void *result)
{
    volatile uint32_t sum = 0;
    for (uint32_t i = 0; i < SPIN_ITERATIONS; ++i)
    {
        sum += i;
    }
    *(uint32_t *)result = sum;
    return NULL;
}

static double seconds(struct timespec t)
{
    return (double)t.tv_sec + (double)t.tv_nsec / NANOSECONDS_PER_SECOND;
}

/* after - before, in microseconds. */
static long timeval_delta_us(struct timeval after, struct timeval before)
{
    long whole_seconds = (long)(after.tv_sec - before.tv_sec);
    long micro = (long)(after.tv_usec - before.tv_usec);
    return whole_seconds * MICROSECONDS_PER_SECOND + micro;
}

static void sleep_for_wait(void)
{
    struct timespec delay = {0, WAIT_NS};
    while (nanosleep(&delay, &delay))
    {
    }
}

/* Runs two spin loops in parallel threads. Returns 0, 3 or 4 (see header). */
static int run_threads(uint32_t sums[2])
{
    pthread_t first;
    pthread_t second;
    if (pthread_create(&first, NULL, work, &sums[0]) ||
        pthread_create(&second, NULL, work, &sums[1]))
    {
        return 3;
    }
    if (pthread_join(first, NULL) || pthread_join(second, NULL))
    {
        return 4;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    const char *mode = argv[1];
    struct timespec wall_start;
    struct timespec wall_end;
    struct timespec cpu_start;
    struct timespec cpu_end;
    struct timespec resolution;
    struct rusage usage_start;
    struct rusage usage_end;
    uint32_t sums[2] = {0, 0};
    clock_getres(CLOCK_MONOTONIC, &resolution);
    clock_gettime(CLOCK_MONOTONIC, &wall_start);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_start);
    getrusage(RUSAGE_SELF, &usage_start);
    if (!strcmp(mode, "cpu"))
    {
        work(sums);
    }
    else if (!strcmp(mode, "wait"))
    {
        sleep_for_wait();
    }
    else if (!strcmp(mode, "threads"))
    {
        int status = run_threads(sums);
        if (status)
        {
            return status;
        }
    }
    else if (strcmp(mode, "observer"))
    {
        return 2;
    }
    getrusage(RUSAGE_SELF, &usage_end);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_end);
    clock_gettime(CLOCK_MONOTONIC, &wall_end);
    printf(
        "{\"mode\":\"%s\",\"wall_seconds\":%.9f,\"process_cpu_seconds\":%.9f,\"user_us\":%ld,\"system_us\":%ld,\"resolution_ns\":%ld,\"checksum\":%u}\n",
        mode, seconds(wall_end) - seconds(wall_start),
        seconds(cpu_end) - seconds(cpu_start),
        timeval_delta_us(usage_end.ru_utime, usage_start.ru_utime),
        timeval_delta_us(usage_end.ru_stime, usage_start.ru_stime),
        resolution.tv_nsec, sums[0]);
    return 0;
}
