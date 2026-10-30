#ifndef MC_SHARED_H
#define MC_SHARED_H
#include "clinic.h"
#include <pthread.h>

/*
 * A reference-counted unsigned payload protected by one mutex. Start from
 * {0}; the object must keep its address until every user has joined.
 * references counts ownership rights (at most 3); releases counts payload
 * releases (diagnostic).
 */
struct mc_shared
{
    pthread_mutex_t mutex;
    unsigned *value;
    unsigned references, releases;
    int initialized;
    struct mc_allocator allocation;
};

/*
 * Contract conventions: 0 on success, -1 on rejection with outputs and the
 * payload unchanged. Mutex operations are assumed to succeed once the mutex
 * exists; an unexpected mutex failure is a terminal protocol failure, not a
 * rollback case. Callbacks must not re-enter. Each call may block on the
 * mutex. CONTRACTS.md is the normative text.
 */

/* Allocates a zeroed payload and creates the mutex; references becomes 1.
 * On failure everything acquired so far is released. */
int mc_shared_init(struct mc_shared *shared, struct mc_allocator allocator);

/* The caller already owns a reference. Adds one more (limit 3) and stores
 * shared in *out, only on success. */
int mc_shared_retain(struct mc_shared *shared, struct mc_shared **out);

/* Adds amount to the payload under the lock; refused on overflow. */
int mc_shared_add(struct mc_shared *shared, unsigned amount);

/* Copies the payload to *out under the lock. */
int mc_shared_read(struct mc_shared *shared, unsigned *out);

/* Consumes the caller's reference and sets *owner to NULL. The last drop
 * releases the payload exactly once, under the lock. */
int mc_shared_drop(struct mc_shared **owner);

/* Destroys the mutex once no reference remains. The caller guarantees that
 * every other thread has joined and nobody is still using the object. */
int mc_shared_finish(struct mc_shared *shared);
#endif
