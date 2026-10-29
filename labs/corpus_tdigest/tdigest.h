#ifndef CORPUS_TDIGEST_H
#define CORPUS_TDIGEST_H
#include <stdint.h>
#include <stddef.h>

enum
{
    TD_OK = 0,
    TD_INVALID = 1,
    TD_CAPACITY = 2
};

struct td_centroid
{
    double mean;
    uint32_t weight;
};

struct td_td
{
    unsigned compression, used;
    uint32_t count;
    struct td_centroid centroids[128];
};

/* C17 hosted double, finite samples in [-1e9,1e9], total<=65536.
   Live complete disjoint objects; exact self-merge admitted; caller serializes. */
int td_init(struct td_td *, unsigned compression); /* 8..32 */
int td_valid(const struct td_td *);
int td_add(struct td_td *, const double *values,
           size_t count); /* batch <=64, zero admits NULL */
int td_merge(struct td_td *,
             const struct td_td *); /* destination compression <= source */
int td_recompress(struct td_td *,
                  unsigned compression); /* only equal/coarser */
int td_quantile(const struct td_td *, double q,
                double *); /* generalized inverse, q in[0,1] */
int td_cdf(const struct td_td *, double x,
           double *); /* right-continuous fitted distribution */
#endif
