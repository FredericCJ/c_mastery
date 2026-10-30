/*
 * Reference implementation of the shared-payload part of the clinic
 * laboratory (private assessor copy; POSIX threads, hosted only). Contracts
 * are stated in labs/object_followthrough/clinic/shared.h and CONTRACTS.md.
 *
 * Locking rule: once mc_shared_init() has succeeded, every access to value,
 * references and releases happens with the mutex held (the initialized flag
 * is read before locking). A function that locks unlocks on every path that
 * follows a successful lock; a failed lock or unlock is reported as -1 (the
 * contract treats it as a terminal protocol failure, not as a rollback
 * case). MC_MUTANT_RACE removes the lock from mc_shared_add() on purpose.
 *
 * Test hooks: MC_MUTANT_RETAIN and MC_MUTANT_RACE each break exactly one
 * contract so the campaign can prove that the tests reject the breakage.
 */
#include "shared.h"
#include <limits.h>

enum
{
    MC_MAX_REFERENCES = 3
};

int mc_shared_init(struct mc_shared *shared, struct mc_allocator allocator)
{
    if (shared == NULL || shared->initialized || allocator.allocate == NULL ||
        allocator.release == NULL)
    {
        return -1;
    }
    unsigned *payload = allocator.allocate(allocator.context, sizeof *payload);
    if (payload == NULL)
    {
        return -1;
    }
    *payload = 0;
    if (pthread_mutex_init(&shared->mutex, NULL) != 0)
    {
        /* Roll back the only resource acquired so far. */
        allocator.release(allocator.context, payload);
        return -1;
    }
    shared->value = payload;
    shared->allocation = allocator;
    shared->references = 1;
    shared->releases = 0;
    shared->initialized = 1;
    return 0;
}

int mc_shared_retain(struct mc_shared *shared, struct mc_shared **out)
{
    if (shared == NULL || out == NULL || !shared->initialized ||
        pthread_mutex_lock(&shared->mutex) != 0)
    {
        return -1;
    }
    int status = -1;
    if (shared->value != NULL && shared->references < MC_MAX_REFERENCES)
    {
#ifndef MC_MUTANT_RETAIN
        ++shared->references;
#endif
        *out = shared;
        status = 0;
    }
    if (pthread_mutex_unlock(&shared->mutex) != 0)
    {
        return -1;
    }
    return status;
}

int mc_shared_add(struct mc_shared *shared, unsigned amount)
{
    if (shared == NULL || !shared->initialized)
    {
        return -1;
    }
#ifndef MC_MUTANT_RACE
    if (pthread_mutex_lock(&shared->mutex) != 0)
    {
        return -1;
    }
#endif
    int status = -1;
    /* Overflow is rejected before the payload is changed. */
    if (shared->value != NULL && amount <= UINT_MAX - *shared->value)
    {
        *shared->value += amount;
        status = 0;
    }
#ifndef MC_MUTANT_RACE
    if (pthread_mutex_unlock(&shared->mutex) != 0)
    {
        return -1;
    }
#endif
    return status;
}

int mc_shared_read(struct mc_shared *shared, unsigned *out)
{
    if (shared == NULL || out == NULL || !shared->initialized ||
        pthread_mutex_lock(&shared->mutex) != 0)
    {
        return -1;
    }
    int status = -1;
    if (shared->value != NULL)
    {
        *out = *shared->value;
        status = 0;
    }
    if (pthread_mutex_unlock(&shared->mutex) != 0)
    {
        return -1;
    }
    return status;
}

/*
 * The caller's handle is cleared while the lock is held, and the payload is
 * released under the same lock by whichever drop brings the count to zero.
 */
int mc_shared_drop(struct mc_shared **owner)
{
    if (owner == NULL || *owner == NULL)
    {
        return -1;
    }
    struct mc_shared *shared = *owner;
    if (!shared->initialized || pthread_mutex_lock(&shared->mutex) != 0)
    {
        return -1;
    }
    if (shared->references == 0)
    {
        (void)pthread_mutex_unlock(&shared->mutex);
        return -1;
    }
    *owner = NULL;
    --shared->references;
    if (shared->references == 0)
    {
        void *payload = shared->value;
        shared->value = NULL;
        ++shared->releases;
        shared->allocation.release(shared->allocation.context, payload);
    }
    return pthread_mutex_unlock(&shared->mutex) != 0 ? -1 : 0;
}

/* Valid only after external quiescence: the lock cannot protect its own
 * destruction. */
int mc_shared_finish(struct mc_shared *shared)
{
    if (shared == NULL || !shared->initialized ||
        pthread_mutex_lock(&shared->mutex) != 0)
    {
        return -1;
    }
    if (shared->references != 0)
    {
        (void)pthread_mutex_unlock(&shared->mutex);
        return -1;
    }
    if (pthread_mutex_unlock(&shared->mutex) != 0 ||
        pthread_mutex_destroy(&shared->mutex) != 0)
    {
        return -1;
    }
    shared->initialized = 0;
    return 0;
}
