#ifndef CM_SECURITY_EXTENSION_H
#define CM_SECURITY_EXTENSION_H
#include <stddef.h>
#include <stdint.h>

struct mde_object
{
    uint32_t generation;
    unsigned live, owner;
    unsigned value;
};

struct mde_event
{
    uint32_t generation;
    unsigned owner;
    unsigned value;
};

/* All pointers designate live distinct control/storage objects. Serialized
 * transitions; the generation must never wrap or be reset under queued events. */
int mde_deliver(struct mde_object *, struct mde_event);
int mde_retire(struct mde_object *);
int mde_recreate(struct mde_object *, unsigned, unsigned);
/* Binary logging produces exactly4*n uppercase hex-escape bytes plus NUL,
 * checks n <= (capacity-1)/4 before multiplication; output unchanged on reject. */
int mde_log(const unsigned char *, size_t, char *, size_t);
struct mde_owner;

struct mde_owner
{
    unsigned state;
    void *first, *second;
    void (*release)(void *, void *);
    void *context;
};

/* state0 closed/empty; state1 acquired. May destroy partially constructed
 * first/second. Publish closed and detach both pointers before callbacks.
 * Callbacks may reenter destroy, but may not free control or mutate it. */
int mde_destroy(struct mde_owner *);
#endif
