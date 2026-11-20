/*
 * Supplied workloads for the native PMU comparisons (PMU_PROTOCOL.md,
 * "Controlled investigations"). Course infrastructure, not a learner
 * exercise.
 *
 * Every family has two variants that do equivalent work by different means
 * and must produce the same checksum, which an independent arithmetic oracle
 * also predicts. pf_workload_verify checks all three outside any measured
 * interval; a measured run then only has to reproduce the verified checksum.
 *
 * Two families need other course components and are compiled in only on
 * request, never replaced by something else:
 *   sort     the algorithms component (algo.h, algo_insertion/algo_merge):
 *            build with -DPF_WITH_SORT and an algo.c;
 *   service  the corpus events, timer collection and timer components:
 *            build with -DPF_WITH_SERVICE and events.c, collection.c, timer.c.
 * Without them the family reports itself unavailable.
 */
#ifndef PF_PMU_WORKLOADS_H
#define PF_PMU_WORKLOADS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PF_WORKLOAD_MAX_N 4194304u
#define PF_WORKLOAD_MAX_REPETITIONS 1024u
#define PF_WORKLOAD_VARIANTS 2u

enum pf_family
{
    PF_FAMILY_SEARCH = 0,
    PF_FAMILY_LAYOUT,
    PF_FAMILY_NUMERIC,
    PF_FAMILY_DISPATCH,
    PF_FAMILY_ALLOCATION,
    PF_FAMILY_SORT,
    PF_FAMILY_CONCURRENCY,
    PF_FAMILY_SERVICE,
    PF_FAMILY_COUNT
};

struct pf_family_info
{
    const char *name;
    size_t max_n; /* largest accepted element count, at most MAX_N */
    bool available; /* compiled into this build */
    const char *needs; /* missing component when unavailable, else NULL */
};

/* Look up a family by its exact name. Returns false, leaving *family
 * unchanged, for an unknown name. */
bool pf_family_lookup(const char *name, enum pf_family *family);

/* Static description of `family` (never NULL for a valid family). */
const struct pf_family_info *pf_family_describe(enum pf_family family);

enum pf_workload_status
{
    PF_WORKLOAD_OK = 0,
    PF_WORKLOAD_UNAVAILABLE, /* family not compiled into this build */
    PF_WORKLOAD_INVALID, /* n or repetitions outside the family's range */
    PF_WORKLOAD_NO_MEMORY,
    PF_WORKLOAD_FAILED, /* a variant reported an internal failure */
    PF_WORKLOAD_MISMATCH /* variants disagree, or disagree with the oracle */
};

/* Opaque: the input data one family needs (sorted values, wide records,
 * stable-sort records). */
struct pf_workload;

/* Allocate and initialize the inputs for `n` elements of `family`.
 * On success *workload is owned by the caller until pf_workload_destroy. */
enum pf_workload_status pf_workload_create(enum pf_family family, size_t n,
                                           struct pf_workload **workload);

/* Release a workload from pf_workload_create (NULL is ignored). */
void pf_workload_destroy(struct pf_workload *workload);

/* Run `variant` (0 or 1) `repetitions` times. The checksum depends only on
 * the family, n and repetitions. Returns PF_WORKLOAD_FAILED when the variant
 * failed internally (allocation, thread or component error); *checksum is
 * then unspecified. Concurrency starts and joins one peer thread. */
enum pf_workload_status pf_workload_run(struct pf_workload *workload,
                                        unsigned variant, unsigned repetitions,
                                        uint64_t *checksum);

/* Run both variants and the independent oracle. On PF_WORKLOAD_OK,
 * *expected is the checksum every measured run must reproduce. */
enum pf_workload_status pf_workload_verify(struct pf_workload *workload,
                                           unsigned repetitions,
                                           uint64_t *expected);

const char *pf_workload_status_name(enum pf_workload_status status);

#endif /* PF_PMU_WORKLOADS_H */
