/*
 * Supplied workloads for the native PMU comparisons. Contract:
 * pmu_workloads.h. The noinline attribute is a GCC/Clang extension used so
 * the dispatch family keeps real calls to compare against indirect calls.
 */
#include "pmu_workloads.h"

#include "pmu_integrations.h"

#if defined(PF_WITH_SORT)
#include "algo.h"
#define PF_HAVE_SORT true
#define PF_SORT_REQUIRES NULL
#else
#define PF_HAVE_SORT false
#define PF_SORT_REQUIRES \
    "algorithms component (build with -DPF_WITH_SORT and algo.c)"
#endif

#if defined(PF_WITH_SERVICE)
#define PF_HAVE_SERVICE true
#define PF_SERVICE_REQUIRES NULL
#else
#define PF_HAVE_SERVICE false
#define PF_SERVICE_REQUIRES                                \
    "events, collection and timer components (build with " \
    "-DPF_WITH_SERVICE and events.c, collection.c, timer.c)"
#endif

#include <stdlib.h>

enum
{
    SEARCH_QUERIES = 256, /* lower-bound queries per repetition */
    SORT_KEYS = 17 /* distinct keys, so the stable order is tested */
};

/* One 8-byte value padded to 64 bytes: the layout family sums the same
 * values while touching eight times as many bytes. */
struct pf_wide
{
    uint64_t value;
    unsigned char padding[56];
};

struct pf_workload
{
    enum pf_family family;
    size_t n;
    uint64_t *values; /* values[i] == 2 * i: sorted, distinct, even */
    struct pf_wide *wide; /* layout only */
#if defined(PF_WITH_SORT)
    algo_pair *records; /* sort only; rebuilt on every repetition */
#endif
    uint64_t (*dispatch[2])(uint64_t); /* operation0, operation1 */
};

static const struct pf_family_info families[PF_FAMILY_COUNT] = {
    [PF_FAMILY_SEARCH] = {"search", PF_WORKLOAD_MAX_N, true, NULL},
    [PF_FAMILY_LAYOUT] = {"layout", PF_WORKLOAD_MAX_N, true, NULL},
    [PF_FAMILY_NUMERIC] = {"numeric", PF_WORKLOAD_MAX_N, true, NULL},
    [PF_FAMILY_DISPATCH] = {"dispatch", PF_WORKLOAD_MAX_N, true, NULL},
    /* One allocation per element per repetition. */
    [PF_FAMILY_ALLOCATION] = {"allocation", 65536u, true, NULL},
    /* Insertion sort is quadratic; the retained interface's bound. */
    [PF_FAMILY_SORT] = {"sort", 1024u, PF_HAVE_SORT, PF_SORT_REQUIRES},
    [PF_FAMILY_CONCURRENCY] = {"concurrency", PF_WORKLOAD_MAX_N, true, NULL},
    /* The service adapter's timer storage holds 1024 timers. */
    [PF_FAMILY_SERVICE] = {"service", 1024u, PF_HAVE_SERVICE,
                           PF_SERVICE_REQUIRES},
};

bool pf_family_lookup(const char *name, enum pf_family *family)
{
    for (size_t i = 0; (name != NULL) && (i < PF_FAMILY_COUNT); ++i)
    {
        const char *candidate = families[i].name;
        size_t j = 0;

        while ((name[j] != '\0') && (name[j] == candidate[j]))
        {
            ++j;
        }
        if ((name[j] == '\0') && (candidate[j] == '\0'))
        {
            *family = (enum pf_family)i;
            return true;
        }
    }
    return false;
}

const struct pf_family_info *pf_family_describe(enum pf_family family)
{
    if ((unsigned)family >= PF_FAMILY_COUNT)
    {
        return NULL;
    }
    return &families[family];
}

__attribute__((noinline)) static uint64_t operation0(uint64_t value)
{
    return value * 33u + 17u;
}

__attribute__((noinline)) static uint64_t operation1(uint64_t value)
{
    return value * 33u + 19u;
}

void pf_workload_destroy(struct pf_workload *workload)
{
    if (workload == NULL)
    {
        return;
    }
    free(workload->values);
    free(workload->wide);
#if defined(PF_WITH_SORT)
    free(workload->records);
#endif
    free(workload);
}

enum pf_workload_status pf_workload_create(enum pf_family family, size_t n,
                                           struct pf_workload **workload)
{
    const struct pf_family_info *info = pf_family_describe(family);

    if (info == NULL)
    {
        return PF_WORKLOAD_INVALID;
    }
    if (!info->available)
    {
        return PF_WORKLOAD_UNAVAILABLE;
    }
    if ((n == 0u) || (n > info->max_n))
    {
        return PF_WORKLOAD_INVALID;
    }
    struct pf_workload *created = calloc(1, sizeof *created);
    if (created == NULL)
    {
        return PF_WORKLOAD_NO_MEMORY;
    }
    created->family = family;
    created->n = n;
    /* n <= PF_WORKLOAD_MAX_N, so none of these sizes can wrap. */
    created->values = malloc(n * sizeof *created->values);
    bool complete = (created->values != NULL);
    if (family == PF_FAMILY_LAYOUT)
    {
        created->wide = calloc(n, sizeof *created->wide);
        complete = complete && (created->wide != NULL);
    }
#if defined(PF_WITH_SORT)
    if (family == PF_FAMILY_SORT)
    {
        created->records = malloc(n * sizeof *created->records);
        complete = complete && (created->records != NULL);
    }
#endif
    if (!complete)
    {
        pf_workload_destroy(created);
        return PF_WORKLOAD_NO_MEMORY;
    }
    for (size_t i = 0; i < n; ++i)
    {
        created->values[i] = (uint64_t)i * 2u;
        if (created->wide != NULL)
        {
            created->wide[i].value = (uint64_t)i * 2u;
        }
    }
    created->dispatch[0] = operation0;
    created->dispatch[1] = operation1;
    *workload = created;
    return PF_WORKLOAD_OK;
}

/* Index of the first value >= key in sorted values[0..n). */
static size_t lower_bound_linear(const uint64_t *values, size_t n, uint64_t key)
{
    size_t low = 0;

    while ((low < n) && (values[low] < key))
    {
        ++low;
    }
    return low;
}

static size_t lower_bound_binary(const uint64_t *values, size_t n, uint64_t key)
{
    size_t low = 0;
    size_t high = n;

    while (low < high)
    {
        size_t middle = low + (high - low) / 2u;

        if (values[middle] < key)
        {
            low = middle + 1u;
        }
        else
        {
            high = middle;
        }
    }
    return low;
}

/* Lower bound in the same sorted immutable extent: variant 0 scans
 * linearly, variant 1 bisects. */
static uint64_t run_search(const struct pf_workload *w, unsigned variant,
                           unsigned repetitions)
{
    uint64_t sum = 0;

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        for (unsigned q = 0; q < SEARCH_QUERIES; ++q)
        {
            uint64_t key =
                ((uint64_t)q * 2654435761u + repeat) % (2u * w->n + 1u);

            sum += (variant != 0u) ? lower_bound_binary(w->values, w->n, key)
                                   : lower_bound_linear(w->values, w->n, key);
        }
    }
    return sum;
}

/* The same values packed in 8 bytes (variant 0) or padded to 64 (1). */
static uint64_t run_layout(const struct pf_workload *w, unsigned variant,
                           unsigned repetitions)
{
    uint64_t sum = 0;

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        if (variant != 0u)
        {
            for (size_t i = 0; i < w->n; ++i)
            {
                sum += w->wide[i].value;
            }
        }
        else
        {
            for (size_t i = 0; i < w->n; ++i)
            {
                sum += w->values[i];
            }
        }
    }
    return sum;
}

/* The same modular sum with one accumulator (variant 0) or four (1). */
static uint64_t run_numeric(const struct pf_workload *w, unsigned variant,
                            unsigned repetitions)
{
    uint64_t sum = 0;

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        size_t i = 0;

        if (variant != 0u)
        {
            uint64_t a = 0;
            uint64_t b = 0;
            uint64_t c = 0;
            uint64_t d = 0;

            for (; i + 3u < w->n; i += 4u)
            {
                a += w->values[i];
                b += w->values[i + 1u];
                c += w->values[i + 2u];
                d += w->values[i + 3u];
            }
            sum += a + b + c + d;
        }
        for (; i < w->n; ++i)
        {
            sum += w->values[i];
        }
    }
    return sum;
}

/* The same operations chosen by a branch (variant 0) or by an indirect
 * call through a runtime table (variant 1). */
static uint64_t run_dispatch(const struct pf_workload *w, unsigned variant,
                             unsigned repetitions)
{
    uint64_t sum = 0;

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        for (size_t i = 0; i < w->n; ++i)
        {
            uint64_t value = w->values[i] + repeat;

            if (variant != 0u)
            {
                sum += w->dispatch[value & 1u](value);
            }
            else if ((value & 1u) != 0u)
            {
                sum += operation1(value);
            }
            else
            {
                sum += operation0(value);
            }
        }
    }
    return sum;
}

/* One object per value (variant 0) or one block (variant 1), with the same
 * values and the same complete lifetime inside each repetition. */
static enum pf_workload_status run_allocation(const struct pf_workload *w,
                                              unsigned variant,
                                              unsigned repetitions,
                                              uint64_t *checksum)
{
    uint64_t sum = 0;

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        if (variant != 0u)
        {
            uint64_t *block = malloc(w->n * sizeof *block);

            if (block == NULL)
            {
                return PF_WORKLOAD_FAILED;
            }
            for (size_t i = 0; i < w->n; ++i)
            {
                block[i] = w->values[i] + repeat;
            }
            for (size_t i = 0; i < w->n; ++i)
            {
                sum += block[i];
            }
            free(block);
            continue;
        }
        uint64_t **items = calloc(w->n, sizeof *items);
        if (items == NULL)
        {
            return PF_WORKLOAD_FAILED;
        }
        size_t made = 0;
        for (; made < w->n; ++made)
        {
            items[made] = malloc(sizeof *items[made]);
            if (items[made] == NULL)
            {
                break;
            }
            *items[made] = w->values[made] + repeat;
        }
        for (size_t i = 0; i < made; ++i)
        {
            sum += *items[i];
            free(items[i]);
        }
        free(items);
        if (made != w->n)
        {
            return PF_WORKLOAD_FAILED;
        }
    }
    *checksum = sum;
    return PF_WORKLOAD_OK;
}

#if defined(PF_WITH_SORT)
/* Stable sort of (key, original position) records through the retained
 * algorithms interfaces: insertion (variant 0) or merge (variant 1). The
 * position-weighted checksum changes if equal keys change order. */
static enum pf_workload_status run_sort(struct pf_workload *w, unsigned variant,
                                        unsigned repetitions,
                                        uint64_t *checksum)
{
    uint64_t sum = 0;
    uint32_t count = (uint32_t)w->n; /* n <= 1024 */

    for (unsigned repeat = 0; repeat < repetitions; ++repeat)
    {
        for (size_t i = 0; i < w->n; ++i)
        {
            w->records[i] =
                (algo_pair){(uint32_t)((w->n - i) % SORT_KEYS), (uint32_t)i};
        }
        int result = (variant != 0u)
                         ? algo_merge(w->records, count, count, NULL)
                         : algo_insertion(w->records, count, count, NULL);
        if (result != 0)
        {
            return PF_WORKLOAD_FAILED;
        }
        for (size_t i = 0; i < w->n; ++i)
        {
            sum += (i + 1u) * (w->records[i].key + 31u * w->records[i].tag);
        }
    }
    *checksum = sum;
    return PF_WORKLOAD_OK;
}
#endif

/* The integrated adapters return UINT64_MAX on failure; neither checksum
 * can legitimately reach it within the accepted bounds. */
static enum pf_workload_status from_adapter(uint64_t result, uint64_t *checksum)
{
    if (result == UINT64_MAX)
    {
        return PF_WORKLOAD_FAILED;
    }
    *checksum = result;
    return PF_WORKLOAD_OK;
}

enum pf_workload_status pf_workload_run(struct pf_workload *workload,
                                        unsigned variant, unsigned repetitions,
                                        uint64_t *checksum)
{
    if ((variant >= PF_WORKLOAD_VARIANTS) || (repetitions == 0u) ||
        (repetitions > PF_WORKLOAD_MAX_REPETITIONS))
    {
        return PF_WORKLOAD_INVALID;
    }
    switch (workload->family)
    {
        case PF_FAMILY_SEARCH:
            *checksum = run_search(workload, variant, repetitions);
            return PF_WORKLOAD_OK;
        case PF_FAMILY_LAYOUT:
            *checksum = run_layout(workload, variant, repetitions);
            return PF_WORKLOAD_OK;
        case PF_FAMILY_NUMERIC:
            *checksum = run_numeric(workload, variant, repetitions);
            return PF_WORKLOAD_OK;
        case PF_FAMILY_DISPATCH:
            *checksum = run_dispatch(workload, variant, repetitions);
            return PF_WORKLOAD_OK;
        case PF_FAMILY_ALLOCATION:
            return run_allocation(workload, variant, repetitions, checksum);
#if defined(PF_WITH_SORT)
        case PF_FAMILY_SORT:
            return run_sort(workload, variant, repetitions, checksum);
#endif
        case PF_FAMILY_CONCURRENCY:
            return from_adapter(
                pf_concurrency_work(workload->n, variant, repetitions),
                checksum);
#if defined(PF_WITH_SERVICE)
        case PF_FAMILY_SERVICE:
            return from_adapter(
                pf_service_work(workload->n, variant, repetitions), checksum);
#endif
        default:
            return PF_WORKLOAD_UNAVAILABLE;
    }
}

/* Independent arithmetic prediction of every family's checksum: closed
 * forms, or (sort, service) a direct enumeration that shares no code with
 * either variant. Unsigned arithmetic wraps identically on both sides. */
static uint64_t sort_oracle(uint64_t n)
{
    uint64_t total = 0;
    uint64_t position = 0;

    for (uint64_t key = 0; key < SORT_KEYS; ++key)
    {
        for (uint64_t tag = 0; tag < n; ++tag)
        {
            if ((n - tag) % SORT_KEYS == key)
            {
                ++position;
                total += position * (key + 31u * tag);
            }
        }
    }
    return total;
}

static uint64_t service_oracle(uint64_t n)
{
    uint64_t total = 0;

    for (uint64_t i = 0; i < n; ++i)
    {
        if (i % 5u != 0u) /* every fifth timer is cancelled */
        {
            total += 18u + i; /* parsed document sum 7 + 11, plus slot */
        }
    }
    return total;
}

static uint64_t oracle(const struct pf_workload *w, unsigned repetitions)
{
    uint64_t n = w->n;
    uint64_t total = 0;

    for (uint64_t r = 0; r < repetitions; ++r)
    {
        switch (w->family)
        {
            case PF_FAMILY_SEARCH:
                for (uint64_t q = 0; q < SEARCH_QUERIES; ++q)
                {
                    uint64_t key = (q * 2654435761u + r) % (2u * n + 1u);

                    total += (key + 1u) / 2u; /* even values below key */
                }
                break;
            case PF_FAMILY_LAYOUT:
            case PF_FAMILY_NUMERIC:
                total += n * (n - 1u); /* sum of 2i for i < n */
                break;
            case PF_FAMILY_DISPATCH:
                total +=
                    33u * (n * (n - 1u) + n * r) + (17u + 2u * (r & 1u)) * n;
                break;
            case PF_FAMILY_ALLOCATION:
                total += n * (n - 1u) + n * r;
                break;
            case PF_FAMILY_SORT:
                total += sort_oracle(n);
                break;
            case PF_FAMILY_CONCURRENCY:
                total += 2u * n; /* two threads, n increments each */
                break;
            case PF_FAMILY_SERVICE:
                total += service_oracle(n);
                break;
            default:
                return UINT64_MAX;
        }
    }
    return total;
}

enum pf_workload_status pf_workload_verify(struct pf_workload *workload,
                                           unsigned repetitions,
                                           uint64_t *expected)
{
    uint64_t first = 0;
    uint64_t second = 0;
    enum pf_workload_status status =
        pf_workload_run(workload, 0, repetitions, &first);

    if (status == PF_WORKLOAD_OK)
    {
        status = pf_workload_run(workload, 1, repetitions, &second);
    }
    if (status != PF_WORKLOAD_OK)
    {
        return status;
    }
    if ((first != second) || (first != oracle(workload, repetitions)))
    {
        return PF_WORKLOAD_MISMATCH;
    }
    *expected = first;
    return PF_WORKLOAD_OK;
}

const char *pf_workload_status_name(enum pf_workload_status status)
{
    switch (status)
    {
        case PF_WORKLOAD_OK:
            return "ok";
        case PF_WORKLOAD_UNAVAILABLE:
            return "unavailable";
        case PF_WORKLOAD_INVALID:
            return "invalid";
        case PF_WORKLOAD_NO_MEMORY:
            return "no-memory";
        case PF_WORKLOAD_FAILED:
            return "failed";
        case PF_WORKLOAD_MISMATCH:
            return "mismatch";
        default:
            return "invalid-status";
    }
}
