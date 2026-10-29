#ifndef CMASTERY_ABSTRACTION_HANDLES_H
#define CMASTERY_ABSTRACTION_HANDLES_H
#include <stdatomic.h>

enum ah_result
{
    AH_OK,
    AH_INVALID,
    AH_STALE,
    AH_FULL,
    AH_BUSY,
    AH_DEAD,
    AH_NOMEM,
    AH_UNIMPLEMENTED
};

struct ah_slot
{
    unsigned generation, occupied, retired, value;
};

struct ah_pool
{
    struct ah_slot slots[3];
    unsigned epoch, borrowers;
};

struct ah_handle
{
    unsigned index, generation, epoch;
};

void ah_pool_init(struct ah_pool *);
enum ah_result ah_allocate(struct ah_pool *, unsigned value,
                           struct ah_handle *);
enum ah_result ah_read(const struct ah_pool *, struct ah_handle, unsigned *);
enum ah_result ah_release(struct ah_pool *, struct ah_handle);
enum ah_result ah_enter(struct ah_pool *);
enum ah_result ah_leave(struct ah_pool *);
enum ah_result ah_reuse(struct ah_pool *);

struct ah_stats
{
    atomic_uint controls, payloads, control_frees, payload_frees;
};
struct ah_shared;
enum ah_result ah_shared_create(unsigned, struct ah_stats *,
                                struct ah_shared **);
enum ah_result ah_retain(struct ah_shared *);
enum ah_result ah_drop(struct ah_shared *);
enum ah_result ah_weak_retain(struct ah_shared *);
enum ah_result ah_weak_drop(struct ah_shared *);
enum ah_result ah_promote(struct ah_shared *);
enum ah_result ah_counts(struct ah_shared *, unsigned *, unsigned *);
enum ah_result ah_add(struct ah_shared *, unsigned);
/* Strong ownership is required; raw access needs an independent access protocol. */
unsigned *ah_payload(struct ah_shared *);
#endif
