#define _GNU_SOURCE
#include "record.h"
#include "map.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "PL_CHECK:%d:%s errno=%d\n", __LINE__, #x, errno); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)

enum
{
    COUNT = 8192,
    WIDTH = 32
};

static uint64_t calls;
ssize_t __real_pread(int, void *, size_t, off_t);
ssize_t __real___pread_chk(int, void *, size_t, off_t, size_t);

ssize_t __wrap_pread(int fd, void *b, size_t n, off_t offset)
{
    ++calls;
    return __real_pread(fd, b, n, offset);
}

ssize_t __wrap___pread_chk(int fd, void *b, size_t n, off_t offset,
                           size_t bound)
{
    ++calls;
    return __real___pread_chk(fd, b, n, offset, bound);
}

static uint64_t time_ns(clockid_t clock)
{
    struct timespec t;
    CHECK(clock_gettime(clock, &t) == 0);
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

static uint64_t digest(uint64_t value, const unsigned char *b)
{
    for (size_t i = 0; i < WIDTH; ++i)
    {
        value = (value ^ b[i]) * UINT64_C(1099511628211);
    }
    return value;
}

static void sample(int fd, const struct pm_map *map,
                   const size_t offsets[COUNT], const char *workload,
                   unsigned trial, unsigned mapped, uint64_t expected)
{
    unsigned char output[WIDTH];
    struct rusage before, after;
    CHECK(getrusage(RUSAGE_SELF, &before) == 0);
    uint64_t wall = time_ns(CLOCK_MONOTONIC),
             cpu = time_ns(CLOCK_PROCESS_CPUTIME_ID), start_calls = calls,
             sum = UINT64_C(1469598103934665603);
    for (size_t i = 0; i < COUNT; ++i)
    {
        int error = mapped ? pm_copy(map,
                                     (struct pm_borrow){offsets[i], WIDTH,
                                                        map->generation},
                                     output, sizeof output)
                           : pw_read_record(fd, map->logical, offsets[i], WIDTH,
                                            output, sizeof output);
        CHECK(error == 0);
        sum = digest(sum, output);
    }
    cpu = time_ns(CLOCK_PROCESS_CPUTIME_ID) - cpu;
    wall = time_ns(CLOCK_MONOTONIC) - wall;
    CHECK(getrusage(RUSAGE_SELF, &after) == 0 && sum == expected);
    printf(
        "PW_SAMPLE workload=%s trial=%u method=%s records=%u bytes=%u wall_ns=%" PRIu64
        " cpu_ns=%" PRIu64 " minor=%ld major=%ld pread_calls=%" PRIu64
        " digest=%" PRIu64 "\n",
        workload, trial, mapped ? "mapped" : "pread", COUNT, COUNT * WIDTH,
        wall, cpu, after.ru_minflt - before.ru_minflt,
        after.ru_majflt - before.ru_majflt, calls - start_calls, sum);
    CHECK(calls - start_calls == (mapped ? 0 : COUNT));
}

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    CHECK(page > 0 && page <= 65536);
    size_t logical = (size_t)page * 15 + 13, mapped = (size_t)page * 16;
    CHECK(logical <= 1024 * 1024);
    char path[] = "/var/tmp/pw-bench-XXXXXX";
    int fd = mkstemp(path);
    CHECK(fd >= 0 && unlink(path) == 0 && ftruncate(fd, (off_t)logical) == 0);
    unsigned char block[WIDTH];
    for (unsigned p = 0; p < 16; p += 2)
    {
        for (size_t i = 0; i < WIDTH; ++i)
        {
            block[i] = (unsigned char)(p * 7 + i);
        }
        CHECK(pwrite(fd, block, sizeof block,
                     (off_t)((size_t)p * (size_t)page)) == sizeof block);
    }
    struct pm_map map = {0};
    uint64_t setup = time_ns(CLOCK_MONOTONIC);
    CHECK(pm_map(fd, 0, mapped, logical, &map) == 0);
    setup = time_ns(CLOCK_MONOTONIC) - setup;
    unsigned char a[WIDTH], b[WIDTH], sentinel[WIDTH];
    memset(a, 0xcc, sizeof a);
    memcpy(b, a, sizeof a);
    memcpy(sentinel, a, sizeof a);
    CHECK(pw_read_record(fd, logical, logical - 8, WIDTH, a, sizeof a) ==
              ERANGE &&
          pm_copy(&map, (struct pm_borrow){logical - 8, WIDTH, map.generation},
                  b, sizeof b) == ERANGE &&
          !memcmp(a, sentinel, sizeof a) && !memcmp(b, sentinel, sizeof b));
    size_t offsets[COUNT];
    uint32_t state = UINT32_C(0x12345678);
    for (unsigned workload = 0; workload < 2; ++workload)
    {
        uint64_t expected = UINT64_C(1469598103934665603);
        for (size_t i = 0; i < COUNT; ++i)
        {
            state = state * UINT32_C(1664525) + UINT32_C(1013904223);
            offsets[i] = workload ? (size_t)state % (logical - WIDTH + 1)
                                  : (i * WIDTH) % (logical - WIDTH + 1);
            CHECK(pw_read_record(fd, logical, offsets[i], WIDTH, a, sizeof a) ==
                      0 &&
                  pm_copy(&map,
                          (struct pm_borrow){offsets[i], WIDTH, map.generation},
                          b, sizeof b) == 0 &&
                  !memcmp(a, b, sizeof a));
            for (size_t j = 0; j < WIDTH; ++j)
            {
                size_t at = offsets[i] + j, p = at / (size_t)page,
                       within = at % (size_t)page;
                unsigned char wanted = (p % 2 == 0 && within < WIDTH)
                                           ? (unsigned char)(p * 7 + within)
                                           : 0;
                CHECK(a[j] == wanted);
            }
            expected = digest(expected, a);
        }
        for (unsigned trial = 0; trial < 8; ++trial)
        {
            unsigned first = (trial % 4 == 0 || trial % 4 == 3) ? 0 : 1;
            sample(fd, &map, offsets, workload ? "sparse_random" : "sequential",
                   trial, first, expected);
            sample(fd, &map, offsets, workload ? "sparse_random" : "sequential",
                   trial, 1 - first, expected);
        }
    }
    printf(
        "PW_SETUP mapping_ns=%" PRIu64
        " page=%ld logical=%zu mapped=%zu cache=prevalidated-warm-order-uncontrolled\n",
        setup, page, logical, mapped);
    CHECK(pm_unmap(&map) == 0 && close(fd) == 0);
    puts("PW_PASS");
    return 0;
}
