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
void *ef_hidden_take(size_t);
void ef_hidden_release(void *);
void *__real_malloc(size_t);
void __real_free(void *);
static unsigned requests, releases;

void *__wrap_malloc(size_t n)
{
    ++requests;
    return __real_malloc(n);
}

void __wrap_free(void *p)
{
    ++releases;
    __real_free(p);
}

static unsigned char *take(void *c, size_t n)
{
    (void)c;
    return malloc(n);
}

static void release(void *c, unsigned char *p)
{
    (void)c;
    free(p);
}

static double seconds(struct timespec t)
{
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static _Thread_local unsigned tls_calls;

static int pure_callback(void)
{
    return 7;
}

static int effect_callback(void)
{
    unsigned char *p = malloc(37);
    if (!p)
    {
        return -1;
    }
    p[0] = 7;
    struct timespec wait = {0, 1000000};
    while (nanosleep(&wait, &wait))
    {
        if (errno != EINTR)
        {
            free(p);
            return -1;
        }
    }
    int value = p[0];
    free(p);
    return value;
}

static int entry(int (*callback)(void))
{
    ++tls_calls;
    return callback();
}

#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "EFFECTS_PROCESS_ASSERT:%d:%s\n", __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "callbacks"))
    {
        unsigned before = requests;
        int a = entry(pure_callback);
        unsigned pure = requests - before;
        before = requests;
        int b = entry(effect_callback);
        unsigned effects = requests - before;
        CHECK(a == 7 && b == 7 && pure == 0 && effects == 1 && tls_calls == 2);
        printf(
            "{\"same_result\":7,\"pure_requests\":%u,\"effect_requests\":%u,\"tls_calls\":%u,\"changed_callback_timed_wait\":true}\n",
            pure, effects, tls_calls);
        return 0;
    }
    if (!strcmp(argv[1], "cache") || !strcmp(argv[1], "startup"))
    {
        struct ef_cache c = {{NULL, take, release}, NULL, 1, 0, 0, 0};
        uint32_t out;
        unsigned before = requests;
        unsigned startup = 0;
        if (!strcmp(argv[1], "startup"))
        {
            CHECK(ef_cached(&c, NULL, 0, &out) == EF_OK);
            startup = requests - before;
            before = requests;
        }
        struct timespec start, end;
        CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
        CHECK(ef_cached(&c, (const unsigned char *)"abc", 3, &out) == EF_OK);
        unsigned cold = requests - before;
        CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
        double cold_seconds = seconds(end) - seconds(start);
        CHECK(out == 294);
        before = requests;
        CHECK(clock_gettime(CLOCK_MONOTONIC, &start) == 0);
        for (unsigned i = 0; i < 100; ++i)
        {
            CHECK(ef_cached(&c, (const unsigned char *)"abc", 3, &out) ==
                  EF_OK);
        }
        CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
        double warm_seconds = seconds(end) - seconds(start);
        unsigned warm = requests - before;
        CHECK(ef_cache_reset(&c) == EF_OK);
        before = requests;
        CHECK(ef_cached(&c, NULL, 0, &out) == EF_OK);
        unsigned refill = requests - before;
        CHECK(ef_cache_reset(&c) == EF_OK);
        CHECK(cold + startup == 1 && warm == 0 && refill == 1);
        printf(
            "{\"startup_requests\":%u,\"cold_requests\":%u,\"warm_requests\":%u,\"refill_requests\":%u,\"releases\":%u,\"cold_seconds\":%.9f,\"warm100_seconds\":%.9f}\n",
            startup, cold, warm, refill, releases, cold_seconds, warm_seconds);
        return 0;
    }
    if (!strcmp(argv[1], "bypass"))
    {
        unsigned before = requests;
        void *p = ef_hidden_take(37);
        CHECK(p);
        memset(p, 3, 37);
        ef_hidden_release(p);
        unsigned hidden = requests - before;
        struct ef_pool pool = {0};
        unsigned char *b;
        before = requests;
        CHECK(ef_pool_take(&pool, &b) == EF_OK);
        *b = 7;
        CHECK(hidden == 0 && requests == before && pool.requests == 1);
        printf(
            "{\"dso_hook_requests\":%u,\"pool_requests\":%u,\"pool_hook_requests\":%u}\n",
            hidden, pool.requests, requests - before);
        return 0;
    }
    if (!strcmp(argv[1], "stdio"))
    {
        CHECK(setlocale(LC_ALL, "C"));
        FILE *f = tmpfile();
        CHECK(f);
        char buffer[128];
        CHECK(setvbuf(f, buffer, _IOFBF, sizeof buffer) == 0);
        CHECK(fputs("synthetic effects\n", f) >= 0 && fflush(f) == 0 &&
              fseek(f, 0, SEEK_SET) == 0);
        char observed[32];
        CHECK(fgets(observed, sizeof observed, f) &&
              !strcmp(observed, "synthetic effects\n"));
        CHECK(fclose(f) == 0);
        struct timespec delay = {0, 1000000};
        while (nanosleep(&delay, &delay) && errno == EINTR)
        {
        }
        printf("integer=%d floating=%.3f locale=%s\n", 17, 0.125,
               setlocale(LC_NUMERIC, NULL));
        return 0;
    }
    if (!strcmp(argv[1], "console"))
    {
        printf("terminal=%d\n", isatty(STDOUT_FILENO));
        printf("one\n");
        printf("two\n");
        return fflush(stdout) != 0;
    }
    if (!strcmp(argv[1], "refill"))
    {
        struct ef_pool pool = {0};
        unsigned char *p;
        for (unsigned i = 0; i < 16; ++i)
        {
            CHECK(ef_pool_take(&pool, &p) == EF_OK);
        }
        unsigned before = requests;
        for (unsigned i = 0; i < 100; ++i)
        {
            CHECK(ef_pool_return(&pool, 0) == EF_OK &&
                  ef_pool_take(&pool, &p) == EF_OK);
        }
        CHECK(requests == before && ef_pool_take(&pool, &p) == EF_FULL);
        struct ef_pool *second = malloc(sizeof *second);
        CHECK(second);
        memset(second, 0, sizeof *second);
        CHECK(ef_pool_take(second, &p) == EF_OK);
        unsigned acquisition = requests - before;
        free(second);
        CHECK(acquisition == 1);
        printf(
            "{\"first_pool_requests\":%u,\"warm_backend_requests\":0,\"refill_backend_requests\":%u}\n",
            pool.requests, acquisition);
        return 0;
    }
    if (!strcmp(argv[1], "resources"))
    {
        struct timespec wall0, wall1, cpu0, cpu1, resolution;
        struct rusage before, after;
        CHECK(clock_getres(CLOCK_MONOTONIC, &resolution) == 0 &&
              clock_gettime(CLOCK_MONOTONIC, &wall0) == 0 &&
              clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu0) == 0 &&
              getrusage(RUSAGE_SELF, &before) == 0);
        FILE *zero = fopen("/dev/zero", "r+");
        CHECK(zero);
        long page = sysconf(_SC_PAGESIZE);
        CHECK(page > 0);
        size_t length = (size_t)page * 256;
        volatile unsigned char *p = mmap(NULL, length, PROT_READ | PROT_WRITE,
                                         MAP_PRIVATE, fileno(zero), 0);
        CHECK((const void *)p != MAP_FAILED);
        for (size_t i = 0; i < length; i += (size_t)page)
        {
            p[i] = 1;
        }
        volatile uint32_t value = 0;
        for (uint32_t i = 0; i < 1000000; ++i)
        {
            value += i;
        }
        CHECK(munmap((void *)p, length) == 0 && fclose(zero) == 0 &&
              getrusage(RUSAGE_SELF, &after) == 0 &&
              clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu1) == 0 &&
              clock_gettime(CLOCK_MONOTONIC, &wall1) == 0);
        printf(
            "{\"wall_seconds\":%.9f,\"process_cpu_seconds\":%.9f,\"user_us\":%ld,\"system_us\":%ld,\"minor_faults\":%ld,\"clock_resolution_ns\":%ld,\"checksum\":%u}\n",
            seconds(wall1) - seconds(wall0), seconds(cpu1) - seconds(cpu0),
            (long)((after.ru_utime.tv_sec - before.ru_utime.tv_sec) * 1000000 +
                   after.ru_utime.tv_usec - before.ru_utime.tv_usec),
            (long)((after.ru_stime.tv_sec - before.ru_stime.tv_sec) * 1000000 +
                   after.ru_stime.tv_usec - before.ru_stime.tv_usec),
            after.ru_minflt - before.ru_minflt, resolution.tv_nsec,
            (unsigned)value);
        return 0;
    }
    return 2;
}
