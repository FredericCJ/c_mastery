#ifndef CMASTERY_MESSAGING_LIFETIME_H
#define CMASTERY_MESSAGING_LIFETIME_H
#include "messaging.h"
#include "owned_value.h"

struct mfl_allocator
{
    void *context;
    void *(*allocate)(void *, size_t);
    void (*deallocate)(void *, void *);
};
struct mfl_shared;
enum msg_status mfl_make(struct mfl_allocator, struct mfo_value *,
                         struct mfl_shared **);
enum msg_status mfl_retain(struct mfl_shared *);
void mfl_drop(struct mfl_shared **);
const struct mfo_value *mfl_borrow(const struct mfl_shared *);

struct mfg_array;

struct mfg_deque
{
    pthread_mutex_t mutex;
    struct mfl_allocator allocator;
    struct mfg_array *array;
    size_t top, bottom, capacity, maximum;
    unsigned active;
    bool live;
};

typedef void (*mfg_pause_fn)(void *);
enum msg_status mfg_init(struct mfg_deque *, struct mfl_allocator, size_t);
enum msg_status mfg_push(struct mfg_deque *, struct mfo_value *);
enum msg_status mfg_pop(struct mfg_deque *, struct mfl_shared **);
enum msg_status mfg_steal(struct mfg_deque *, struct mfl_shared **,
                          mfg_pause_fn, void *);
enum msg_status mfg_destroy(struct mfg_deque *);

struct mfp_pointer
{
    pthread_mutex_t mutex;
    struct mfl_shared *current;
    uint64_t sequence;
    bool live;
};
enum msg_status mfp_init(struct mfp_pointer *, struct mfl_shared **);
enum msg_status mfp_snapshot(struct mfp_pointer *, struct mfl_shared **);
enum msg_status mfp_replace(struct mfp_pointer *, struct mfl_shared **);
enum msg_status mfp_destroy(struct mfp_pointer *);
#endif
