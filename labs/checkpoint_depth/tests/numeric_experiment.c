#define _POSIX_C_SOURCE 200809L
#include "checkpoint_vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "CP_CHECK: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)
void cp_test_numeric_transaction(void);
void cp_test_numeric(void);
void cp_test_numeric_policy(void);
void cp_test_dispatch(void);
void cp_test_vector(void);
static volatile double sink;

static unsigned long long now(void)
{
    struct timespec t;
    CHECK(!clock_gettime(CLOCK_MONOTONIC_RAW, &t));
    return (unsigned long long)t.tv_sec * 1000000000ULL +
           (unsigned long long)t.tv_nsec;
}

static void barrier(const void *p)
{
    __asm__ volatile("" ::"r"(p) : "memory");
}

int main(int argc, char **argv)
{
    cp_test_numeric_transaction();
    cp_test_numeric();
    cp_test_numeric_policy();
    cp_test_dispatch();
    cp_test_vector();
    if (argc == 2 && !strcmp(argv[1], "check"))
        return 0;
    CHECK(argc == 1);
    const size_t sizes[] = {1, 5, 256, 256, 256, 256},
                 blocks[] = {1, 1, 1, 64, 1024, 16384};
    const size_t evict_size = 128U * 1024U * 1024U;
    unsigned char *evict = malloc(evict_size);
    CHECK(evict);
    memset(evict, 1, evict_size);
    for (size_t workload = 0; workload < sizeof sizes / sizeof sizes[0];
         ++workload)
    {
        size_t n = sizes[workload], count = blocks[workload],
               elements = n * count, repeats = 16384 / elements;
        if (!repeats)
            repeats = 1;
        double *in = malloc(elements * sizeof *in),
               *out = malloc(elements * sizeof *out),
               *expected = malloc(elements * sizeof *expected);
        CHECK(in && out && expected);
        for (size_t i = 0; i < elements; ++i)
            in[i] = (double)((int)(i % 31) - 15) / 8.0;
        for (size_t b = 0; b < count; ++b)
            for (size_t i = 0; i < n; ++i)
            {
                size_t at = b * n + i;
                expected[at] = (0.25 * in[at] + (i ? 0.5 * in[at - 1] : 0)) +
                               (i > 1 ? 0.25 * in[at - 2] : 0);
            }
        for (unsigned sample = 0; sample < 13; ++sample)
            for (unsigned mode = 0; mode < 2; ++mode)
                for (unsigned order = 0; order < 3; ++order)
                {
                    /* Rotate paired backend order; no cherry-picked best sample. */
                    unsigned which = (order + sample) % 3;
                    enum cpv_backend requested = which == 0   ? CPV_SCALAR
                                                 : which == 1 ? CPV_COMPILER
                                                              : CPV_AVX;
                    struct cpv_plan plan;
                    if (cpv_plan(&plan, requested) == CPN_UNSUPPORTED)
                        continue;
                    if (mode)
                    {
                        for (size_t i = 0; i < evict_size; i += 64)
                            evict[i] = (unsigned char)(evict[i] + 1);
                        barrier(evict);
                    }
                    else
                    {
                        struct cpn_double warm = {0, 0};
                        CHECK(cpv_run(&plan, &warm, in, n, out, n) == CPN_OK);
                    }
                    unsigned long long a = now();
                    for (size_t r = 0; r < repeats; ++r)
                        for (size_t b = 0; b < count; ++b)
                            barrier(in + b * n);
                    unsigned long long overhead = now() - a;
                    a = now();
                    for (size_t r = 0; r < repeats; ++r)
                        for (size_t b = 0; b < count; ++b)
                        {
                            struct cpn_double s = {0, 0};
                            barrier(in + b * n);
                            CHECK(cpv_run(&plan, &s, in + b * n, n, out + b * n,
                                          n) == CPN_OK);
                        }
                    unsigned long long elapsed = now() - a;
                    CHECK(!memcmp(out, expected, elements * sizeof(double)));
                    double sum = 0;
                    for (size_t i = 0; i < elements; ++i)
                        sum += out[i];
                    sink = sum;
                    printf(
                        "{\"n\":%zu,\"blocks\":%zu,\"repeats\":%zu,\"working_bytes\":%zu,\"sample\":%u,\"cache\":\"%s\",\"backend\":%u,\"elapsed_ns\":%llu,\"empty_ns\":%llu,\"checksum\":%.17g}\n",
                        n, count, repeats, 2 * elements * sizeof(double),
                        sample,
                        mode ? "128MiB-perturbed" : "first-block-warmed", which,
                        elapsed, overhead, (double)sink);
                }
        free(expected);
        free(out);
        free(in);
    }
    free(evict);
    return 0;
}
