#define _POSIX_C_SOURCE 200809L
#include "capture.h"
#include "dod_buffers.h"
#include "dod_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <time.h>
#ifndef COPY_OBSERVED
#define COPY_OBSERVED 0
#endif
static size_t copied;

void d_copy_observe(size_t n)
{
    copied += n;
}

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
        exit(2);
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "STORAGE_MEASURE:%d:%s\n", __LINE__, #x); \
            exit(1);                                                  \
        }                                                             \
    } while (0)

static void observations(void)
{
    unsigned char bytes[17];
    for (unsigned i = 0; i < 17; ++i)
        bytes[i] = (unsigned char)i;
    const unsigned cycles[] = {1, 8, 64};
    for (unsigned compact = 0; compact < 2; ++compact)
        for (unsigned ci = 0; ci < 3; ++ci)
        {
            unsigned count = cycles[ci];
            struct ow_allocator a = {0};
            struct d_small s;
            d_small_init(&s, ds_counted_bytes(&a));
            copied = 0;
            CHECK(d_small_append(&s, bytes, 16) == D_OK);
            for (unsigned i = 0; i < count; ++i)
            {
                CHECK(d_small_append(&s, bytes + 16, 1) == D_OK &&
                      s.length == 17);
                CHECK(d_small_truncate(&s, 16) == D_OK);
                if (compact)
                    CHECK(d_small_compact(&s) == D_OK);
                for (unsigned j = 0; j < 16; ++j)
                    CHECK(d_small_data(&s)[j] == j);
            }
            size_t before_live = a.live;
            d_small_reset(&s);
            CHECK(!a.live && !a.invalid_releases);
            printf("O %u %u %zu %zu %zu %zu\n", compact, count, copied,
                   a.attempts, a.releases, before_live);
        }
    for (size_t n = 1; n <= 8; ++n)
    {
        int32_t values[8];
        for (unsigned i = 0; i < 8; ++i)
            values[i] = (int32_t)i;
        struct ow_allocator pool = {0};
        struct ds_erased e;
        ds_erased_init(&e, ds_counted_bytes(&pool));
        copied = 0;
        CHECK(ds_capture_i32(&e, values, n, NULL, NULL) == DS_OK);
        size_t encoded = copied;
        ds_erased_drop(&e);
        struct ds_typed_capture t;
        ds_typed_init(&t, ow_allocator(&pool));
        copied = 0;
        CHECK(ds_typed_assign(&t, values, n) == DS_OK);
        size_t typed = copied;
        ds_typed_drop(&t);
        CHECK(!pool.live && !pool.invalid_releases);
        printf("C %zu %zu %zu %zu %zu\n", n, encoded, typed, pool.attempts,
               pool.releases);
    }
    printf("L %zu %zu %zu %zu %zu\n", sizeof(struct d_small),
           sizeof(struct d_inline), sizeof(struct ds_typed_capture),
           _Alignof(struct d_small), _Alignof(struct ds_typed_capture));
}

static uint32_t scalar_lane(uint32_t a, uint32_t b, unsigned bits, int sign)
{
    uint32_t out = 0, mask = bits == 8 ? 255 : 65535;
    for (unsigned lane = 0; lane < 32 / bits; ++lane)
    {
        uint32_t x = (a >> (lane * bits)) & mask,
                 y = (b >> (lane * bits)) & mask;
        if (sign)
        {
            int32_t sx = x < 32768 ? (int32_t)x : -1 - (int32_t)(65535 - x),
                    sy = y < 32768 ? (int32_t)y : -1 - (int32_t)(65535 - y);
            int32_t n = sx + sy;
            if (n > 32767)
                n = 32767;
            if (n < -32768)
                n = -32768;
            out |= ((uint32_t)n & mask) << (lane * bits);
        }
        else
        {
            uint32_t n = x + y;
            if (n > mask)
                n = mask;
            out |= n << (lane * bits);
        }
    }
    return out;
}

static void timings(void)
{
    const unsigned repetitions = 100000;
    int32_t values[8];
    for (unsigned i = 0; i < 8; ++i)
        values[i] = (int32_t)i - 2;
    for (size_t n = 1; n <= 8; ++n)
    {
        struct ow_allocator pool = {0};
        struct ds_erased e;
        struct ds_typed_capture t;
        ds_erased_init(&e, ds_counted_bytes(&pool));
        ds_typed_init(&t, ow_allocator(&pool));
        CHECK(ds_capture_i32(&e, values, n, NULL, NULL) == DS_OK &&
              ds_typed_assign(&t, values, n) == DS_OK);
        for (unsigned trial = 0; trial < 7; ++trial)
            for (unsigned k = 0; k < 3; ++k)
            {
                unsigned kind = (k + trial) % 3;
                int64_t checksum = 0;
                int out = 0;
                for (unsigned warm = 0; warm < 32; ++warm)
                {
                    CHECK(ds_erased_call(&e, 3, &out) == DS_OK &&
                          ds_typed_call(&t, 3, &out) == DS_OK &&
                          ds_borrowed_i32(values, n, 3, &out) == DS_OK);
                }
                atomic_signal_fence(memory_order_seq_cst);
                uint64_t start = now();
                for (unsigned i = 0; i < repetitions; ++i)
                {
                    int arg = (int)(i % 17),
                        status = kind == 0 ? ds_erased_call(&e, arg, &out)
                                 : kind == 1
                                     ? ds_typed_call(&t, arg, &out)
                                     : ds_borrowed_i32(values, n, arg, &out);
                    CHECK(status == DS_OK);
                    checksum += out;
                }
                atomic_signal_fence(memory_order_seq_cst);
                uint64_t elapsed = now() - start;
                int64_t expected = 0;
                for (unsigned i = 0; i < repetitions; ++i)
                {
                    int64_t value = i % 17;
                    for (size_t j = 0; j < n; ++j)
                        value += values[j];
                    expected += value;
                }
                CHECK(checksum == expected);
                printf("T %zu %u %u %llu %lld\n", n, trial, kind,
                       (unsigned long long)elapsed, (long long)checksum);
            }
        ds_erased_drop(&e);
        ds_typed_drop(&t);
        CHECK(!pool.live);
    }
    uint32_t *left = malloc(65536 * sizeof(*left)),
             *right = malloc(65536 * sizeof(*right));
    CHECK(left && right);
    uint32_t random = 7;
    for (unsigned i = 0; i < 65536; ++i)
    {
        random = random * 1664525u + 1013904223u;
        left[i] = random;
        random = random * 1664525u + 1013904223u;
        right[i] = random;
    }
    for (unsigned mode = 0; mode < 3; ++mode)
        for (unsigned ni = 0; ni < 2; ++ni)
        {
            unsigned n = ni ? 65536 : 1024;
            uint64_t expected = 0;
            for (unsigned i = 0; i < n; ++i)
                expected += scalar_lane(left[i], right[i], mode == 0 ? 8 : 16,
                                        mode == 2);
            for (unsigned trial = 0; trial < 7; ++trial)
                for (unsigned k = 0; k < 2; ++k)
                {
                    unsigned kind = (k + trial) % 2;
                    uint64_t sum = 0;
                    atomic_signal_fence(memory_order_seq_cst);
                    uint64_t start = now();
                    for (unsigned i = 0; i < n; ++i)
                        sum += kind == 0
                                   ? scalar_lane(left[i], right[i],
                                                 mode == 0 ? 8 : 16, mode == 2)
                               : mode == 0 ? d_swar_add(left[i], right[i])
                               : mode == 1 ? d_swar16_add(left[i], right[i])
                                           : d_swar16_signed(left[i], right[i]);
                    atomic_signal_fence(memory_order_seq_cst);
                    uint64_t elapsed = now() - start;
                    CHECK(sum == expected);
                    printf("W %u %u %u %u %llu %llu\n", mode, n, trial, kind,
                           (unsigned long long)elapsed,
                           (unsigned long long)sum);
                }
        }
    free(left);
    free(right);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "observe"))
        observations();
    else if (!strcmp(argv[1], "time") && !COPY_OBSERVED)
        timings();
    else
        return 2;
    return 0;
}
