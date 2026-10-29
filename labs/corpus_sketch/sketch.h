#ifndef CORPUS_SKETCH_H
#define CORPUS_SKETCH_H
#include <stdint.h>
#include <stddef.h>

/* All pointers name live disjoint complete objects, except exact self-merge.
   Objects are initialized through the API. Callers serialize access. */
enum
{
    SK_OK = 0,
    SK_INVALID = 1,
    SK_OVERFLOW = 2
};

struct sk_cm_config
{
    unsigned p, depth;
    uint32_t mask[4][5];
    unsigned bias[4];
};

struct sk_cm
{
    struct sk_cm_config config;
    uint64_t total, cell[4][32];
};

int sk_cm_init(struct sk_cm *, const struct sk_cm_config *);
int sk_cm_add(struct sk_cm *, uint32_t key, uint64_t amount);
int sk_cm_query(const struct sk_cm *, uint32_t key, uint64_t *);
int sk_cm_merge(struct sk_cm *, const struct sk_cm *);

struct sk_hll
{
    unsigned p;
    uint32_t identity;
    uint8_t reg[256];
};

int sk_hll_init(struct sk_hll *, unsigned p, uint32_t hash_identity);
int sk_hll_add(struct sk_hll *, uint32_t hash);
int sk_hll_estimate(const struct sk_hll *, double *);
int sk_hll_merge(struct sk_hll *, const struct sk_hll *);
/* Merge a finer sketch into an equal/coarser destination; preserve hash bits. */
int sk_hll_fold(struct sk_hll *, const struct sk_hll *);

struct sk_mh_config
{
    unsigned universe, count;
    uint8_t rank[120][16];
};

struct sk_mh
{
    struct sk_mh_config config;
    uint8_t minimum[120];
};

struct sk_fraction
{
    unsigned numerator, denominator;
};

int sk_mh_init(struct sk_mh *, const struct sk_mh_config *);
int sk_mh_add(struct sk_mh *, unsigned element);
int sk_mh_similarity(const struct sk_mh *, const struct sk_mh *,
                     struct sk_fraction *);
int sk_mh_merge(struct sk_mh *, const struct sk_mh *);
/* Count complete matching bands, not a Jaccard estimate. */
int sk_mh_bands(const struct sk_mh *, const struct sk_mh *, unsigned rows,
                unsigned *matching);
#endif
