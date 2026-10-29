#include "dispatch_plugin.h"

int ds_feature_acquire(struct ds_loaded *m, unsigned k, struct ds_feature *o)
{
    (void)m;
    (void)k;
    (void)o;
    return DS_FAILED;
}

int ds_feature_apply(const struct ds_feature *f, int s, int a, int *o)
{
    (void)f;
    (void)s;
    (void)a;
    (void)o;
    return DS_FAILED;
}

int ds_feature_describe(const struct ds_feature *f, unsigned char *o, size_t n,
                        size_t *l)
{
    (void)f;
    (void)o;
    (void)n;
    (void)l;
    return DS_FAILED;
}

int ds_feature_release(struct ds_feature *f)
{
    (void)f;
    return DS_FAILED;
}
