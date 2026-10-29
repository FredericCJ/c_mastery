#ifndef CMASTERY_DISPATCH_H
#define CMASTERY_DISPATCH_H
#include "dod.h"
#include <stddef.h>

enum ds_status
{
    DS_OK,
    DS_INVALID,
    DS_RANGE,
    DS_EMPTY,
    DS_BUSY,
    DS_FULL,
    DS_FAILED,
    DS_DENIED,
    DS_STALE,
    DS_UNSUPPORTED
};

struct ds_ops
{
    int (*measure)(const void *, int *);
};

struct ds_view
{
    const void *object;
    const struct ds_ops *ops;
};

struct ds_box
{
    const struct ds_ops *ops;
    int value;
};

int ds_measure(struct ds_view, int *);
int ds_embedded(const struct ds_box *, int *);
typedef int (*ds_combine_fn)(int, int, int *);
int ds_fold(const int *, size_t, int, ds_combine_fn, int *);
int ds_sum(const int *, size_t, int, int *);

enum ds_mode
{
    DS_SHARED,
    DS_MUTABLE,
    DS_ONCE
};

struct ds_closure
{
    int capture;
    enum ds_mode mode;
    unsigned active;
};

int ds_closure_init(struct ds_closure *, int, enum ds_mode);
int ds_closure_call(struct ds_closure *, int, int *);
typedef int (*ds_thunk_fn)(const unsigned char *, size_t, int, int *);

struct ds_erased
{
    struct d_small capture;
    ds_thunk_fn call;
    void (*destroy)(void *);
    void *resource;
    unsigned active;
};

void ds_erased_init(struct ds_erased *, struct d_allocator);
int ds_erased_assign(struct ds_erased *, const unsigned char *, size_t,
                     ds_thunk_fn, void (*)(void *), void *);
int ds_erased_call(const struct ds_erased *, int, int *);
int ds_erased_move(struct ds_erased *, struct ds_erased *);
void ds_erased_drop(struct ds_erased *);

struct ds_methods
{
    ds_combine_fn entries[3][3];
    unsigned epoch;
};

struct ds_cache
{
    unsigned epoch, left, right;
    ds_combine_fn method;
};

void ds_methods_init(struct ds_methods *);
int ds_methods_set(struct ds_methods *, unsigned, unsigned, ds_combine_fn);
int ds_dispatch(const struct ds_methods *, struct ds_cache *, unsigned,
                unsigned, int, int, int *);
struct ds_list;

struct ds_link
{
    struct ds_link *previous, *next;
    struct ds_list *owner;
};

struct ds_item
{
    struct ds_link link;
    int value;
};

struct ds_list
{
    struct ds_link head;
    unsigned count;
};

void ds_list_init(struct ds_list *);
void ds_item_init(struct ds_item *, int);
int ds_list_push(struct ds_list *, struct ds_item *);
int ds_list_remove(struct ds_list *, struct ds_item *);
int ds_list_sum(const struct ds_list *, int *);
struct ds_module;

struct ds_service
{
    int (*read)(void *, int *);
    void *context;
};

int ds_module_open(struct ds_service, struct ds_module **);
int ds_module_twice(struct ds_module *, int *);
void ds_module_close(struct ds_module *);

struct ds_authority
{
    unsigned rights, epoch;
    int counter;
};

struct ds_capability
{
    const struct ds_authority *owner;
    unsigned rights, epoch;
};

void ds_authority_init(struct ds_authority *, unsigned);
int ds_cap_issue(const struct ds_authority *, unsigned, struct ds_capability *);
int ds_cap_attenuate(struct ds_capability, unsigned, struct ds_capability *);
int ds_cap_add(struct ds_authority *, struct ds_capability, int);
int ds_cap_revoke(struct ds_authority *);

struct ds_plugin
{
    unsigned abi;
    size_t size;
    unsigned schema;
    int (*apply)(int, int, int *);
    int (*migrate)(unsigned, int, int *);
};

struct ds_loaded
{
    void *handle;
    const struct ds_plugin *api;
    unsigned borrows;
};

void ds_loaded_init(struct ds_loaded *);
int ds_loaded_open(struct ds_loaded *, const char *);
int ds_loaded_acquire(struct ds_loaded *, const struct ds_plugin **);
int ds_loaded_release(struct ds_loaded *);
int ds_loaded_close(struct ds_loaded *);

struct ds_host
{
    struct ds_loaded current, retired;
    int state;
    unsigned generation;
};

void ds_host_init(struct ds_host *);
int ds_host_reload(struct ds_host *, struct ds_loaded *);
int ds_host_collect(struct ds_host *);
#endif
