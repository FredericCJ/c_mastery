#include "dispatch_resolution.h"

void ds_resolution_init(struct ds_resolution *m)
{
    *m = (struct ds_resolution){0};
}

int ds_resolution_set(struct ds_resolution *m, unsigned l, unsigned r,
                      ds_combine_fn f, struct ds_loaded *p)
{
    (void)m;
    (void)l;
    (void)r;
    (void)f;
    (void)p;
    return DS_FAILED;
}

void ds_resolution_clear(struct ds_resolution *m)
{
    (void)m;
}

int ds_resolution_uncached(const struct ds_resolution *m, unsigned l,
                           unsigned r, int a, int b, int *out)
{
    (void)m;
    (void)l;
    (void)r;
    (void)a;
    (void)b;
    (void)out;
    return DS_FAILED;
}

void ds_resolution_cache_init(struct ds_resolution_cache *c)
{
    *c = (struct ds_resolution_cache){0};
}

void ds_resolution_cache_clear(struct ds_resolution_cache *c)
{
    (void)c;
}

int ds_resolution_call(const struct ds_resolution *m,
                       struct ds_resolution_cache *c, unsigned l, unsigned r,
                       int a, int b, int *out)
{
    (void)m;
    (void)c;
    (void)l;
    (void)r;
    (void)a;
    (void)b;
    (void)out;
    return DS_FAILED;
}
