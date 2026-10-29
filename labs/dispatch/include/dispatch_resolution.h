#ifndef CMASTERY_DISPATCH_RESOLUTION_H
#define CMASTERY_DISPATCH_RESOLUTION_H
#include "dispatch.h"

enum
{
    DS_AMBIGUOUS = 10
};

struct ds_method_lease
{
    ds_combine_fn method;
    struct ds_loaded *module;
};

struct ds_resolution
{
    struct ds_method_lease entries[4][4];
    unsigned epoch;
};

struct ds_resolution_entry
{
    const struct ds_resolution *owner;
    unsigned epoch, left, right;
    int status;
    struct ds_method_lease lease;
};

struct ds_resolution_cache
{
    struct ds_resolution_entry entries[2];
    unsigned next, hits, misses;
};

void ds_resolution_init(struct ds_resolution *);
int ds_resolution_set(struct ds_resolution *, unsigned, unsigned, ds_combine_fn,
                      struct ds_loaded *);
void ds_resolution_clear(struct ds_resolution *);
int ds_resolution_uncached(const struct ds_resolution *, unsigned, unsigned,
                           int, int, int *);
void ds_resolution_cache_init(struct ds_resolution_cache *);
void ds_resolution_cache_clear(struct ds_resolution_cache *);
int ds_resolution_call(const struct ds_resolution *,
                       struct ds_resolution_cache *, unsigned, unsigned, int,
                       int, int *);
#endif
