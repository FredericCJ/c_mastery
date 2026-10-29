#ifndef CMASTERY_DISPATCH_INTRUSIVE_H
#define CMASTERY_DISPATCH_INTRUSIVE_H
#include "dispatch.h"
struct ds_dual_item;
struct ds_dual_list;

struct ds_hook
{
    struct ds_hook *previous, *next;
    struct ds_dual_list *owner;
    struct ds_dual_item *parent;
    unsigned kind;
};

struct ds_dual_item
{
    int value;
    struct ds_hook hooks[2];
};

struct ds_dual_list
{
    struct ds_hook head;
    unsigned kind, count;
};

void ds_dual_init(struct ds_dual_list *, unsigned);
void ds_dual_item_init(struct ds_dual_item *, int);
int ds_dual_push(struct ds_dual_list *, struct ds_dual_item *);
int ds_dual_remove(struct ds_dual_list *, struct ds_dual_item *);
int ds_dual_fold(const struct ds_dual_list *, ds_combine_fn, int, int *);
#define DS_INDEX_NONE 4u

struct ds_index_slot
{
    int value;
    unsigned previous[2], next[2], linked[2];
};

struct ds_index_owner
{
    struct ds_index_slot *slots;
    unsigned live[4], generation[4], head[2], tail[2], count[2];
};

struct ds_index_handle
{
    const struct ds_index_owner *owner;
    unsigned index, generation;
};

void ds_index_init(struct ds_index_owner *, struct ds_index_slot[4]);
int ds_index_new(struct ds_index_owner *, int, struct ds_index_handle *);
int ds_index_get(const struct ds_index_owner *, struct ds_index_handle, int *);
int ds_index_push(struct ds_index_owner *, unsigned, struct ds_index_handle);
int ds_index_remove(struct ds_index_owner *, unsigned, struct ds_index_handle);
int ds_index_release(struct ds_index_owner *, struct ds_index_handle);
int ds_index_relocate(struct ds_index_owner *, struct ds_index_slot[4]);
int ds_index_fold(const struct ds_index_owner *, unsigned, ds_combine_fn, int,
                  int *);
#endif
