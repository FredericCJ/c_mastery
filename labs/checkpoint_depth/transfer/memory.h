#ifndef CHECKPOINT_TRANSFER_MEMORY_H
#define CHECKPOINT_TRANSFER_MEMORY_H
#include <stddef.h>
#include <stdint.h>
#include "checkpoint_memory.h"

enum
{
    CPX_LIMIT = 4,
    CPX_NONE = 4,
    CPX_GENERATION_MAX = 3
};

enum cpx_status
{
    CPX_OK,
    CPX_FULL,
    CPX_INVALID,
    CPX_BUSY,
    CPX_PENDING,
    CPX_FAILED,
    CPX_NOT_IMPLEMENTED
};

struct cpx_record
{
    int value;
    size_t length;
    unsigned char bytes[16];
};

struct cpx_handle
{
    unsigned slot, generation;
};

struct cpx_node
{
    struct cpx_handle handle;
    unsigned next, used;
};

struct cpx_backend
{
    void *context;
    void *(*allocate)(void *, size_t);
    void (*release)(void *, void *);
    int (*subscribe)(void *, uint64_t *);
    int (*unsubscribe)(void *, uint64_t);
};

struct cpx_store
{
    struct cpx_backend backend;
    struct cpx_record *records;
    struct cpx_node *nodes;
    unsigned capacity, node_capacity, state[CPX_LIMIT], generation[CPX_LIMIT],
        pins[CPX_LIMIT];
    unsigned head, tail, count, node_attempts, node_fail_at, closing;
    uint64_t cookie;
};
enum cpx_status cpx_init(struct cpx_store *, struct cpx_backend, unsigned,
                         unsigned);
enum cpx_status cpx_close(struct cpx_store *);
enum cpx_status cpx_acknowledge(struct cpx_store *, uint64_t, int);
enum cpx_status cpx_allocate(struct cpx_store *, const struct cpx_record *,
                             struct cpx_handle *);
enum cpx_status cpx_lookup(struct cpx_store *, struct cpx_handle,
                           const struct cpx_record **);
enum cpx_status cpx_pin(struct cpx_store *, struct cpx_handle);
enum cpx_status cpx_unpin(struct cpx_store *, struct cpx_handle);
enum cpx_status cpx_release(struct cpx_store *, struct cpx_handle);
enum cpx_status cpx_relocate(struct cpx_store *);
enum cpx_status cpx_grow(struct cpx_store *, unsigned);
enum cpx_status cpx_push(struct cpx_store *, struct cpx_handle);
enum cpx_status cpx_pop(struct cpx_store *, struct cpx_handle *);
enum cpx_status cpx_batch(struct cpx_store *, const struct cpx_record *, size_t,
                          size_t *);

struct cpx_lease
{
    struct cpm_arena *arena;
    uint64_t cookie;
    unsigned pending, cancelled;
};
enum cpx_status cpx_lease_begin(struct cpx_lease *, struct cpm_arena *,
                                struct cpm_span, uint64_t);
enum cpx_status cpx_lease_cancel(struct cpx_lease *);
enum cpx_status cpx_lease_complete(struct cpx_lease *, uint64_t, int);
#endif
