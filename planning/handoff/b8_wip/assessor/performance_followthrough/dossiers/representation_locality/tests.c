/*
 * Deterministic contract tests for the locality workload (locality.h),
 * run before any measurement. Expected sums are computed here by direct
 * enumeration, independently of rl_oracle's closed form, which is checked
 * against the same enumeration.
 *
 * Output: RL_TEST_PASS; RL_TEST_ASSERT:<case> on stderr and exit 1.
 */
#include "locality.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void require(bool condition, const char *what)
{
    if (!condition)
    {
        (void)fprintf(stderr, "RL_TEST_ASSERT:%s\n", what);
        exit(1);
    }
}

static uint64_t enumerated_sum(size_t n, unsigned repetitions)
{
    uint64_t total = 0;

    for (unsigned r = 0; r < repetitions; ++r)
    {
        for (size_t i = 0; i < n; ++i)
        {
            total += 2u * (uint64_t)i;
        }
    }
    return total;
}

/* Address of key i as the representation stores it. */
static uintptr_t key_address(const struct rl_data *data, size_t i)
{
    switch (data->layout)
    {
        case RL_SOA:
            return (uintptr_t)&data->columns.key[i];
        case RL_AOS16:
            return (uintptr_t)&((const struct rl_record16 *)data->records)[i]
                .key;
        case RL_AOS64:
            return (uintptr_t)&((const struct rl_record64 *)data->records)[i]
                .key;
        case RL_AOS128:
            return (uintptr_t)&((const struct rl_record128 *)data->records)[i]
                .key;
        default:
            return 0;
    }
}

static void test_names(void)
{
    static const char *const names[RL_LAYOUT_COUNT] = {"soa", "aos16", "aos64",
                                                       "aos128"};
    static const size_t strides[RL_LAYOUT_COUNT] = {8, 16, 64, 128};
    enum rl_layout layout = RL_LAYOUT_COUNT;

    for (unsigned i = 0; i < RL_LAYOUT_COUNT; ++i)
    {
        require(rl_layout_lookup(names[i], &layout) &&
                    (layout == (enum rl_layout)i),
                "layout lookup");
        require(strcmp(rl_layout_name(layout), names[i]) == 0, "layout name");
        require(rl_stride_bytes(layout) == strides[i], "stride bytes");
    }
    layout = RL_AOS16;
    require(!rl_layout_lookup("", &layout) &&
                !rl_layout_lookup("AOS64", &layout) &&
                !rl_layout_lookup("aos64 ", &layout) &&
                !rl_layout_lookup(NULL, &layout),
            "unknown layout names rejected");
    require(layout == RL_AOS16, "rejected lookup leaves output");
    require(sizeof(struct rl_record16) == 16u &&
                sizeof(struct rl_record64) == 64u &&
                sizeof(struct rl_record128) == 128u,
            "record sizes are the strides");
}

static void test_rejections(void)
{
    struct rl_data data;

    require(rl_create(RL_SOA, 0, 1, &data) == RL_INVALID, "n zero");
    require(rl_create(RL_AOS64, (size_t)RL_MAX_N + 1u, 1, &data) == RL_INVALID,
            "n above the bound");
    require(rl_create(RL_LAYOUT_COUNT, 16, 1, &data) == RL_INVALID,
            "unknown layout");
    require(rl_create(RL_SOA, 16, 1, NULL) == RL_INVALID, "null output");
}

static void check_shape(const struct rl_data *data)
{
    size_t stride = rl_stride_bytes(data->layout);

    require(key_address(data, 0) % RL_PAGE_BYTES == 0u, "page-aligned keys");
    for (size_t i = 1; i < data->n; ++i)
    {
        require(key_address(data, i) - key_address(data, i - 1u) == stride,
                "keys one stride apart");
    }
    for (size_t i = 0; i < RL_COLD_COLUMNS && data->layout == RL_SOA; ++i)
    {
        require((uintptr_t)data->columns.cold[i] % RL_PAGE_BYTES == 0u,
                "page-aligned cold columns");
    }
}

static void test_sums(void)
{
    static const size_t sizes[] = {1, 2, 3, 7, 64, 1000, 2048, 32768, 100003};
    static const unsigned repetitions[] = {1, 2, 5};

    for (unsigned layout = 0; layout < RL_LAYOUT_COUNT; ++layout)
    {
        for (size_t s = 0; s < sizeof sizes / sizeof sizes[0]; ++s)
        {
            struct rl_data data;
            struct rl_data other;

            require(rl_create((enum rl_layout)layout, sizes[s], 1, &data) ==
                        RL_OK,
                    "create");
            require(rl_create((enum rl_layout)layout, sizes[s], UINT64_MAX,
                              &other) == RL_OK,
                    "create with other cold fields");
            check_shape(&data);
            for (size_t r = 0; r < sizeof repetitions / sizeof repetitions[0];
                 ++r)
            {
                uint64_t expected = enumerated_sum(sizes[s], repetitions[r]);

                require(rl_oracle(sizes[s], repetitions[r]) == expected,
                        "closed-form oracle");
                require(rl_sum(&data, repetitions[r]) == expected,
                        "layout sum equals the enumeration");
                require(rl_sum(&other, repetitions[r]) == expected,
                        "cold fields do not change the sum");
            }
            rl_destroy(&data);
            rl_destroy(&other);
        }
    }
}

/* The measured size range, once per layout: the closed form must agree
 * with the kernels where the arithmetic is largest. */
static void test_large(void)
{
    size_t n = (size_t)1 << 20;

    for (unsigned layout = 0; layout < RL_LAYOUT_COUNT; ++layout)
    {
        struct rl_data data;

        require(rl_create((enum rl_layout)layout, n, 7, &data) == RL_OK,
                "create large");
        require(data.bytes >= n * rl_stride_bytes(data.layout),
                "allocation covers every record");
        require(rl_sum(&data, 3) == enumerated_sum(n, 3), "large sum");
        require(rl_oracle(n, 1024) == enumerated_sum(n, 1) * 1024u,
                "oracle at the repetition bound");
        rl_destroy(&data);
    }
}

int main(void)
{
    test_names();
    test_rejections();
    test_sums();
    test_large();
    (void)puts("RL_TEST_PASS");
    return 0;
}
