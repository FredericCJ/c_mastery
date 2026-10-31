/*
 * Representation/locality dossier workload. Author-only: it answers the
 * changed condition of an assessed T43 task, so it is not learner content.
 * Derived from the layout family of labs/performance_followthrough
 * (pmu_workloads.c: packed 8-byte values against 64-byte records).
 *
 * The same n keys (key i == 2i) are held in one of four representations
 * and summed. Only the key field is read; the other fields exist, are
 * initialized and are never touched by the measured loop:
 *
 *   soa     struct rl_columns: one key column plus seven cold columns
 *           (the structure-of-arrays form of the 64-byte record)
 *   aos16   struct rl_record16[n]: key plus one cold word
 *   aos64   struct rl_record64[n]: key plus seven cold words
 *   aos128  struct rl_record128[n]: key plus fifteen cold words
 *
 * so one useful 8-byte key costs 8, 16, 64 or 128 bytes of address span.
 * Every storage block is page-aligned (4096 bytes), so records never
 * straddle a 64-byte line.
 *
 * Correctness contract (tests.c, before any measurement): for every layout
 * and every n and repetitions, rl_sum returns rl_oracle(n, repetitions) ==
 * repetitions * n * (n - 1) modulo 2^64, whatever the cold fields contain.
 */
#ifndef RL_LOCALITY_H
#define RL_LOCALITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RL_MAX_N (UINT64_C(1) << 22)
#define RL_PAGE_BYTES 4096u
#define RL_COLD_COLUMNS 7u

enum rl_layout
{
    RL_SOA = 0,
    RL_AOS16,
    RL_AOS64,
    RL_AOS128,
    RL_LAYOUT_COUNT
};

struct rl_record16
{
    uint64_t key;
    uint64_t cold[1];
};

struct rl_record64
{
    uint64_t key;
    uint64_t cold[7];
};

struct rl_record128
{
    uint64_t key;
    uint64_t cold[15];
};

struct rl_columns
{
    uint64_t *key;
    uint64_t *cold[RL_COLD_COLUMNS];
};

struct rl_data
{
    enum rl_layout layout;
    size_t n;
    size_t bytes; /* every allocated byte, all columns included */
    struct rl_columns columns; /* soa */
    void *records; /* aos16, aos64, aos128 */
};

enum rl_status
{
    RL_OK = 0,
    RL_INVALID,
    RL_NO_MEMORY
};

bool rl_layout_lookup(const char *name, enum rl_layout *layout);
const char *rl_layout_name(enum rl_layout layout);

/* Bytes of address span per key in `layout` (8, 16, 64 or 128). */
size_t rl_stride_bytes(enum rl_layout layout);

/* Allocate and initialize n records (1 <= n <= RL_MAX_N): key i is 2i and
 * cold word j of record i is filled from `seed`. Every page is written.
 * On RL_OK the caller owns *data until rl_destroy. */
enum rl_status rl_create(enum rl_layout layout, size_t n, uint64_t seed,
                         struct rl_data *data);
void rl_destroy(struct rl_data *data);

/* Sum the keys `repetitions` times (one call of the layout's kernel per
 * repetition). */
uint64_t rl_sum(const struct rl_data *data, unsigned repetitions);

/* repetitions * n * (n - 1) modulo 2^64, by closed form. */
uint64_t rl_oracle(size_t n, unsigned repetitions);

/* The measured kernels: one pass over the keys. They live in their own
 * translation unit (locality_kernels.c) so that the compiler, seeing only
 * an external call, cannot treat repeated passes as one invariant value. */
uint64_t rl_sum_soa(const struct rl_columns *columns, size_t n);
uint64_t rl_sum_aos16(const struct rl_record16 *records, size_t n);
uint64_t rl_sum_aos64(const struct rl_record64 *records, size_t n);
uint64_t rl_sum_aos128(const struct rl_record128 *records, size_t n);

#endif /* RL_LOCALITY_H */
