/*
 * Process-level effect observations, one mode per command-line argument:
 * cache, startup, callbacks, bypass, stdio, console, refill, resources.
 *
 * The campaign links this file with `--wrap=malloc --wrap=free`, so malloc and
 * free references in this executable's own objects reach __wrap_malloc and
 * __wrap_free (GNU linker interface, not ISO C). Each mode prints one
 * line (JSON except stdio and console) and returns 0, or returns 1 from the
 * first failing CHECK; an unknown mode or a bad argument count returns 2.
 */
#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

void *ef_hidden_take(size_t bytes);
void ef_hidden_release(void *storage);
void *__real_malloc(size_t bytes);
void __real_free(void *storage);

#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "EFFECTS_PROCESS_ASSERT:%d:%s\n", __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

#define NANOSECONDS_PER_SECOND 1e9
#define MICROSECONDS_PER_SECOND 1000000L
#define SLEEP_NS 1000000L
#define WARM_CALLS 100u
#define MAPPED_PAGES 256u
#define SPIN_ITERATIONS 1000000u
#define CALLBACK_RESULT 7
#define CALLBACK_ALLOCATION_BYTES 37u

/* Counts of calls routed through the linker wrappers. */
static unsigned malloc_requests;
static unsigned free_releases;

void *__wrap_malloc(size_t bytes)
{
    ++malloc_requests;
    return __real_malloc(bytes);
}

void __wrap_free(void *storage)
{
    ++free_releases;
    __real_free(storage);
}

static unsigned char *take(void *context, size_t bytes)
{
    (void)context;
    return malloc(bytes);
}

static void release(void *context, unsigned char *storage)
{
    (void)context;
    free(storage);
}

static double seconds(struct timespec t)
{
    return (double)t.tv_sec + (double)t.tv_nsec / NANOSECONDS_PER_SECOND;
}

/* after - before, in microseconds; both values come from getrusage. */
static long timeval_delta_us(struct timeval after, struct timeval before)
{
    long whole_seconds = (long)(after.tv_sec - before.tv_sec);
    long micro = (long)(after.tv_usec - before.tv_usec);
    return whole_seconds * MICROSECONDS_PER_SECOND + micro;
}

/* Sleeps at least SLEEP_NS, resuming after signal interruptions. */
static int sleep_one_millisecond(void)
{
    struct timespec remaining = {0, SLEEP_NS};
    while (nanosleep(&remaining, &remaining))
    {
        if (errno != EINTR)
        {
            return -1;
        }
    }
    return 0;
}

static _Thread_local unsigned tls_calls;

static int pure_callback(void)
{
    return CALLBACK_RESULT;
}

static int effect_callback(void)
{
    unsigned char *scratch = malloc(CALLBACK_ALLOCATION_BYTES);
    if (!scratch)
    {
        return -1;
    }
    scratch[0] = CALLBACK_RESULT;
    if (sleep_one_millisecond())
    {
        free(scratch);
        return -1;
    }
    int value = scratch[0];
    free(scratch);
    return value;
}

static int entry(int (*callback)(void))
{
    ++tls_calls;
    return callback();
}

static int mode_callbacks(void)
{
    unsigned before = malloc_requests;
    int first = entry(pure_callback);
    unsigned pure = malloc_requests - before;
    before = malloc_requests;
    int second = entry(effect_callback);
    unsigned effects = malloc_requests - before;
    CHECK(first == CALLBACK_RESULT && second == CALLBACK_RESULT && pure == 0 &&
          effects == 1 && tls_calls == 2);
    printf(
        "{\"same_result\":7,\"pure_requests\":%u,\"effect_requests\":%u,\"tls_calls\":%u,\"changed_callback_timed_wait\":true}\n",
        pure, effects, tls_calls);
    return 0;
}

/*
 * Counts backend requests on the first call, on WARM_CALLS repeated calls and
 * after a reset. With `with_startup`, an empty call before timing takes the
 * allocation, so the timed first call is not the one that acquires storage.
 */
static int mode_cache(int with_startup)
{
    struct ef_cache cache = {{NULL, take, release}, NULL, 1, 0, 0, 0};
    uint32_t checksum;
    unsigned before = malloc_requests;
    unsigned startup = 0;
    if (with_startup)
    {
        CHECK(ef_cached(&cache, NULL, 0, &checksum) == EF_OK);
        startup = malloc_requests - before;
        before = malloc_requests;
    }
    struct timespec start;
    struct timespec end;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
    CHECK(ef_cached(&cache, (const unsigned char *)"abc", 3, &checksum) ==
          EF_OK);
    unsigned cold = malloc_requests - before;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
    double cold_seconds = seconds(end) - seconds(start);
    CHECK(checksum == 294);

    before = malloc_requests;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
    for (unsigned i = 0; i < WARM_CALLS; ++i)
    {
        CHECK(ef_cached(&cache, (const unsigned char *)"abc", 3, &checksum) ==
              EF_OK);
    }
    CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
    double warm_seconds = seconds(end) - seconds(start);
    unsigned warm = malloc_requests - before;

    CHECK(ef_cache_reset(&cache) == EF_OK);
    before = malloc_requests;
    CHECK(ef_cached(&cache, NULL, 0, &checksum) == EF_OK);
    unsigned refill = malloc_requests - before;
    CHECK(ef_cache_reset(&cache) == EF_OK);
    CHECK(cold + startup == 1 && warm == 0 && refill == 1);
    printf(
        "{\"startup_requests\":%u,\"cold_requests\":%u,\"warm_requests\":%u,\"refill_requests\":%u,\"releases\":%u,\"cold_seconds\":%.9f,\"warm100_seconds\":%.9f}\n",
        startup, cold, warm, refill, free_releases, cold_seconds, warm_seconds);
    return 0;
}

static int mode_bypass(void)
{
    unsigned before = malloc_requests;
    void *hidden = ef_hidden_take(CALLBACK_ALLOCATION_BYTES);
    CHECK(hidden);
    memset(hidden, 3, CALLBACK_ALLOCATION_BYTES);
    ef_hidden_release(hidden);
    unsigned hidden_requests = malloc_requests - before;

    struct ef_pool pool = {{0}, 0, 0};
    unsigned char *slot;
    before = malloc_requests;
    CHECK(ef_pool_take(&pool, &slot) == EF_OK);
    *slot = 7;
    CHECK(hidden_requests == 0 && malloc_requests == before &&
          pool.requests == 1);
    printf(
        "{\"dso_hook_requests\":%u,\"pool_requests\":%u,\"pool_hook_requests\":%u}\n",
        hidden_requests, pool.requests, malloc_requests - before);
    return 0;
}

static int mode_stdio(void)
{
    CHECK(setlocale(LC_ALL, "C"));
    FILE *stream = tmpfile();
    CHECK(stream);
    char buffer[128];
    CHECK(setvbuf(stream, buffer, _IOFBF, sizeof buffer) == 0);
    CHECK(fputs("synthetic effects\n", stream) >= 0 && fflush(stream) == 0 &&
          fseek(stream, 0, SEEK_SET) == 0);
    char observed[32];
    CHECK(fgets(observed, sizeof observed, stream) &&
          !strcmp(observed, "synthetic effects\n"));
    CHECK(fclose(stream) == 0);
    CHECK(sleep_one_millisecond() == 0);
    printf("integer=%d floating=%.3f locale=%s\n", 17, 0.125,
           setlocale(LC_NUMERIC, NULL));
    return 0;
}

static int mode_console(void)
{
    printf("terminal=%d\n", isatty(STDOUT_FILENO));
    printf("one\n");
    printf("two\n");
    return fflush(stdout) != 0;
}

/*
 * Fills the pool, then returns and retakes slot 0 WARM_CALLS times: no backend
 * request may occur. A second, heap-allocated pool is the one acquisition.
 */
static int mode_refill(void)
{
    struct ef_pool pool = {{0}, 0, 0};
    unsigned char *slot;
    for (unsigned i = 0; i < EF_POOL_SLOTS; ++i)
    {
        CHECK(ef_pool_take(&pool, &slot) == EF_OK);
    }
    unsigned before = malloc_requests;
    for (unsigned i = 0; i < WARM_CALLS; ++i)
    {
        CHECK(ef_pool_return(&pool, 0) == EF_OK &&
              ef_pool_take(&pool, &slot) == EF_OK);
    }
    CHECK(malloc_requests == before && ef_pool_take(&pool, &slot) == EF_FULL);
    struct ef_pool *second = malloc(sizeof *second);
    CHECK(second);
    memset(second, 0, sizeof *second);
    CHECK(ef_pool_take(second, &slot) == EF_OK);
    unsigned acquisition = malloc_requests - before;
    free(second);
    CHECK(acquisition == 1);
    printf(
        "{\"first_pool_requests\":%u,\"warm_backend_requests\":0,\"refill_backend_requests\":%u}\n",
        pool.requests, acquisition);
    return 0;
}

/*
 * Measures wall time, process CPU time, rusage and page faults around a file
 * mapping and a spin loop. Time deltas are in seconds, rusage deltas in
 * microseconds, the clock resolution in nanoseconds.
 */
static int mode_resources(void)
{
    struct timespec wall_start;
    struct timespec wall_end;
    struct timespec cpu_start;
    struct timespec cpu_end;
    struct timespec resolution;
    struct rusage before;
    struct rusage after;
    CHECK(clock_getres(CLOCK_MONOTONIC, &resolution) == 0 &&
          clock_gettime(CLOCK_MONOTONIC, &wall_start) == 0 &&
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_start) == 0 &&
          getrusage(RUSAGE_SELF, &before) == 0);
    FILE *zero = fopen("/dev/zero", "r+");
    CHECK(zero);
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0);
    size_t length = (size_t)page * MAPPED_PAGES;
    volatile unsigned char *mapping = mmap(NULL, length, PROT_READ | PROT_WRITE,
                                           MAP_PRIVATE, fileno(zero), 0);
    CHECK((const void *)mapping != MAP_FAILED);
    for (size_t offset = 0; offset < length; offset += (size_t)page)
    {
        mapping[offset] = 1;
    }
    volatile uint32_t value = 0;
    for (uint32_t i = 0; i < SPIN_ITERATIONS; ++i)
    {
        value += i;
    }
    CHECK(munmap((void *)mapping, length) == 0 && fclose(zero) == 0 &&
          getrusage(RUSAGE_SELF, &after) == 0 &&
          clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu_end) == 0 &&
          clock_gettime(CLOCK_MONOTONIC, &wall_end) == 0);
    printf(
        "{\"wall_seconds\":%.9f,\"process_cpu_seconds\":%.9f,\"user_us\":%ld,\"system_us\":%ld,\"minor_faults\":%ld,\"clock_resolution_ns\":%ld,\"checksum\":%u}\n",
        seconds(wall_end) - seconds(wall_start),
        seconds(cpu_end) - seconds(cpu_start),
        timeval_delta_us(after.ru_utime, before.ru_utime),
        timeval_delta_us(after.ru_stime, before.ru_stime),
        after.ru_minflt - before.ru_minflt, resolution.tv_nsec,
        (unsigned)value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    const char *mode = argv[1];
    if (!strcmp(mode, "callbacks"))
    {
        return mode_callbacks();
    }
    if (!strcmp(mode, "cache"))
    {
        return mode_cache(0);
    }
    if (!strcmp(mode, "startup"))
    {
        return mode_cache(1);
    }
    if (!strcmp(mode, "bypass"))
    {
        return mode_bypass();
    }
    if (!strcmp(mode, "stdio"))
    {
        return mode_stdio();
    }
    if (!strcmp(mode, "console"))
    {
        return mode_console();
    }
    if (!strcmp(mode, "refill"))
    {
        return mode_refill();
    }
    if (!strcmp(mode, "resources"))
    {
        return mode_resources();
    }
    return 2;
}
