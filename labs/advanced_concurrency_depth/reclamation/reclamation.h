#ifndef AP_RECLAMATION_H
#define AP_RECLAMATION_H
#include <limits.h>
#include <stdatomic.h>
#include <stdint.h>
_Static_assert(UINT_MAX == UINT32_MAX,
               "this queue profile requires32-bit unsigned");
#define AP_NONE UINT32_MAX
#define AP_NODES 32u
#define AP_PARTICIPANTS 8u

enum
{
    AP_OK,
    AP_RETRY,
    AP_FULL,
    AP_INVALID,
    AP_UNIMPLEMENTED
};

enum
{
    AP_LOADED,
    AP_VALIDATED,
    AP_RETIRE_SNAPSHOT,
    AP_LINKED
};

struct ap_value
{
    uint32_t sequence, inverse;
};

struct ap_node
{
    atomic_uint next, state;
    struct ap_value value;
};

typedef void (*ap_hook_fn)(void *, unsigned point, unsigned participant,
                           unsigned node);

struct ap_queue
{
    atomic_uint head, tail, hazard[AP_PARTICIPANTS][2],
        registration[AP_PARTICIPANTS];
    struct ap_node node[AP_NODES];
    unsigned participants, generation_limit;
    ap_hook_fn hook;
    void *hook_context;
};

struct ap_lease
{
    const struct ap_queue *owner;
    unsigned participant, generation;
};

int ap_init(struct ap_queue *, unsigned participants, unsigned generation_limit,
            ap_hook_fn, void *);
int ap_register(struct ap_queue *, struct ap_lease *);
/* Owner or externally joined successor only: no in-flight operation/borrow. */
int ap_leave(struct ap_queue *, struct ap_lease);
int ap_push(struct ap_queue *, struct ap_lease, struct ap_value);
int ap_pop(struct ap_queue *, struct ap_lease, struct ap_value *);
unsigned ap_collect(struct ap_queue *);
#endif
