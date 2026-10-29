#include "dispatch_intrusive.h"

void ds_dual_init(struct ds_dual_list *p, unsigned k)
{
    (void)p;
    (void)k;
}

void ds_dual_item_init(struct ds_dual_item *p, int v)
{
    (void)p;
    (void)v;
}

int ds_dual_push(struct ds_dual_list *p, struct ds_dual_item *i)
{
    (void)p;
    (void)i;
    return DS_FAILED;
}

int ds_dual_remove(struct ds_dual_list *p, struct ds_dual_item *i)
{
    (void)p;
    (void)i;
    return DS_FAILED;
}

int ds_dual_fold(const struct ds_dual_list *p, ds_combine_fn f, int v, int *o)
{
    (void)p;
    (void)f;
    (void)v;
    (void)o;
    return DS_FAILED;
}

void ds_index_init(struct ds_index_owner *p, struct ds_index_slot s[4])
{
    (void)p;
    (void)s;
}

int ds_index_new(struct ds_index_owner *p, int v, struct ds_index_handle *o)
{
    (void)p;
    (void)v;
    (void)o;
    return DS_FAILED;
}

int ds_index_get(const struct ds_index_owner *p, struct ds_index_handle h,
                 int *o)
{
    (void)p;
    (void)h;
    (void)o;
    return DS_FAILED;
}

int ds_index_push(struct ds_index_owner *p, unsigned k,
                  struct ds_index_handle h)
{
    (void)p;
    (void)k;
    (void)h;
    return DS_FAILED;
}

int ds_index_remove(struct ds_index_owner *p, unsigned k,
                    struct ds_index_handle h)
{
    (void)p;
    (void)k;
    (void)h;
    return DS_FAILED;
}

int ds_index_release(struct ds_index_owner *p, struct ds_index_handle h)
{
    (void)p;
    (void)h;
    return DS_FAILED;
}

int ds_index_relocate(struct ds_index_owner *p, struct ds_index_slot s[4])
{
    (void)p;
    (void)s;
    return DS_FAILED;
}

int ds_index_fold(const struct ds_index_owner *p, unsigned k, ds_combine_fn f,
                  int v, int *o)
{
    (void)p;
    (void)k;
    (void)f;
    (void)v;
    (void)o;
    return DS_FAILED;
}
