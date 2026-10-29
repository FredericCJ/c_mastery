#define _POSIX_C_SOURCE 200809L
#include "algorithms.h"
#include "free.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static volatile uint64_t consumed;

static uint64_t clock_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        exit(2);
    }
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (unsigned)t.tv_nsec;
}

static enum ad_status sort(unsigned method, struct ad_record *p, size_t n,
                           struct ad_record *scratch, struct ad_work *w)
{
    switch (method)
    {
        case 0:
            return ad_insertion(p, n, w);
        case 1:
            return ad_merge_sort(p, n, scratch, 16, w);
        case 2:
            return ad_heap_sort(p, n, w);
        case 3:
            return ad_quick_sort(p, n, 8, 0, w);
        case 4:
            return ad_quick_sort(p, n, 1, 1, w);
        case 5:
            return ad_counting(p, n, scratch, 16, w);
        case 6:
            return ad_radix(p, n, 4, scratch, 16, w);
        default:
            return ad_radix(p, n, 8, scratch, 16, w);
    }
}

int main(void)
{
    unsigned sizes[] = {0, 1, 4, 8, 16};
    for (unsigned distribution = 0; distribution < 3; ++distribution)
    {
        for (unsigned s = 0; s < 5; ++s)
        {
            for (unsigned method = 0; method < 8; ++method)
            {
                size_t n = sizes[s];
                struct ad_record original[16], p[16], scratch[16];
                for (size_t i = 0; i < n; ++i)
                {
                    original[i] = (struct ad_record){
                        (int32_t)(distribution == 0   ? i % 8
                                  : distribution == 1 ? (n - i) % 8
                                                      : i % 2),
                        (unsigned)i};
                }
                struct ad_work work = {0};
                for (size_t i = 0; i < n; ++i)
                {
                    p[i] = original[i];
                }
                if (sort(method, p, n, scratch, &work) != AD_OK)
                {
                    return 3;
                }
                for (size_t i = 1; i < n; ++i)
                {
                    if (p[i - 1].key > p[i].key)
                    {
                        return 4;
                    }
                }
                for (unsigned trial = 0; trial < 3; ++trial)
                {
                    uint64_t start = clock_ns();
                    for (unsigned repeat = 0; repeat < 64; ++repeat)
                    {
                        for (size_t i = 0; i < n; ++i)
                        {
                            p[i] = original[i];
                        }
                        struct ad_work repeated = {0};
                        if (sort(method, p, n, scratch, &repeated) != AD_OK)
                        {
                            return 3;
                        }
                        consumed += n ? (unsigned)p[n - 1].key : 0;
                    }
                    uint64_t elapsed = clock_ns() - start;
                    printf(
                        "{\"kind\":\"sort\",\"method\":%u,\"n\":%zu,\"distribution\":%u,\"trial\":%u,\"repetitions\":64,\"nanoseconds\":%llu,\"key_comparisons\":%llu,\"tracked_record_assignments\":%llu,\"expensive_comparison_units\":%llu,\"heap_control_bytes\":%zu,\"record_bytes\":%zu}\n",
                        method, n, distribution, trial,
                        (unsigned long long)elapsed,
                        (unsigned long long)work.comparisons,
                        (unsigned long long)work.writes,
                        (unsigned long long)(100 * work.comparisons),
                        sizeof(struct cd_heap), sizeof(struct ad_record));
                }
            }
        }
    }
    unsigned byte_sizes[] = {0, 1, 16, 64, 256, 1024};
    unsigned char input[1024], encoded[2048], decoded[1024];
    for (unsigned distribution = 0; distribution < 3; ++distribution)
    {
        for (unsigned s = 0; s < 6; ++s)
        {
            for (unsigned method = 0; method < 3; ++method)
            {
                size_t n = byte_sizes[s], encoded_length = 0,
                       decoded_length = 0;
                uint32_t random = 1515;
                for (size_t i = 0; i < n; ++i)
                {
                    random = random * 1664525u + 1013904223u;
                    input[i] =
                        (unsigned char)(distribution == 0   ? 7
                                        : distribution == 1 ? i % 2
                                                            : random >> 24);
                }
                if (method == 0)
                {
                    fr_copy(encoded, input, n);
                    fr_copy(decoded, encoded, n);
                    encoded_length = decoded_length = n;
                }
                else if (method == 1)
                {
                    if (ad_rle_encode(input, n, encoded, 2048,
                                      &encoded_length) != AD_OK ||
                        ad_rle_decode(encoded, encoded_length, 1, decoded, 1024,
                                      &decoded_length) != AD_OK)
                    {
                        return 5;
                    }
                }
                else
                {
                    if (ad_tagged_encode(input, n, encoded, 2048,
                                         &encoded_length) != AD_OK ||
                        ad_tagged_decode(encoded, encoded_length, decoded, 1024,
                                         &decoded_length) != AD_OK)
                    {
                        return 5;
                    }
                }
                if (decoded_length != n || fr_compare(input, decoded, n))
                {
                    return 6;
                }
                for (unsigned trial = 0; trial < 3; ++trial)
                {
                    uint64_t start = clock_ns();
                    for (unsigned repeat = 0; repeat < 64; ++repeat)
                    {
                        if (method == 0)
                        {
                            fr_copy(encoded, input, n);
                            fr_copy(decoded, encoded, n);
                        }
                        else if (method == 1)
                        {
                            if (ad_rle_encode(input, n, encoded, 2048,
                                              &encoded_length) != AD_OK ||
                                ad_rle_decode(encoded, encoded_length, 1,
                                              decoded, 1024,
                                              &decoded_length) != AD_OK)
                            {
                                return 5;
                            }
                        }
                        else
                        {
                            if (ad_tagged_encode(input, n, encoded, 2048,
                                                 &encoded_length) != AD_OK ||
                                ad_tagged_decode(encoded, encoded_length,
                                                 decoded, 1024,
                                                 &decoded_length) != AD_OK)
                            {
                                return 5;
                            }
                        }
                        consumed += n ? decoded[n - 1] : 0;
                    }
                    uint64_t elapsed = clock_ns() - start;
                    printf(
                        "{\"kind\":\"lossless_roundtrip\",\"method\":%u,\"n\":%zu,\"distribution\":%u,\"trial\":%u,\"repetitions\":64,\"nanoseconds\":%llu,\"encoded_bytes\":%zu,\"caller_encoded_capacity\":2048,\"caller_decoded_capacity\":1024}\n",
                        method, n, distribution, trial,
                        (unsigned long long)elapsed, encoded_length);
                }
            }
        }
    }
    return 0;
}
