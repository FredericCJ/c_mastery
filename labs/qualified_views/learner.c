/*
 * Learner scaffold: every function is an unfinished stub that returns its
 * sentinel. Implement the behaviour documented in views.h and CONTRACTS.md;
 * the tests reject the stubs.
 */
#include "views.h"

enum fv_status fv_reset(struct fv_record *owner)
{
    (void)owner;
    return FV_UNFINISHED;
}

struct fv_view fv_initialize(struct fv_record *caller_owner)
{
    (void)caller_owner;
    return (struct fv_view){0};
}

enum fv_status fv_update(struct fv_record *owner, size_t index, int value)
{
    (void)owner;
    (void)index;
    (void)value;
    return FV_UNFINISHED;
}

struct fv_view fv_view(const struct fv_record *owner)
{
    (void)owner;
    return (struct fv_view){0};
}

enum fv_status fv_copy(struct fv_destination destination, struct fv_view source)
{
    (void)destination;
    (void)source;
    return FV_UNFINISHED;
}

enum fv_status fv_move(int *base, size_t extent, size_t destination,
                       size_t source, size_t count)
{
    (void)base;
    (void)extent;
    (void)destination;
    (void)source;
    (void)count;
    return FV_UNFINISHED;
}

struct fv_owner *fv_owner(void)
{
    return NULL;
}

struct fv_view fv_owner_view(const struct fv_owner *owner)
{
    (void)owner;
    return (struct fv_view){0};
}

int *fv_owner_mutable(struct fv_owner *owner)
{
    (void)owner;
    return NULL;
}

unsigned fv_device_twice(volatile unsigned *status)
{
    (void)status;
    return 1;
}
