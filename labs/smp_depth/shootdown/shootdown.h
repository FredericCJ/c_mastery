#ifndef SMP_SHOOTDOWN_H
#define SMP_SHOOTDOWN_H
#include "locks.h"
#include <stdint.h>

enum
{
    ST_OK,
    ST_INVALID,
    ST_STALE,
    ST_BUSY,
    ST_FULL,
    ST_TIMEOUT,
    ST_PORT,
    ST_SIGNAL,
    ST_INCOMPLETE
};

enum
{
    ST_PAGE,
    ST_GLOBAL,
    ST_CONTEXT,
    ST_CPUS = 4,
    ST_SPACES = 2,
    ST_NO_SPACE = 2,
    ST_MAX_PAGES = 4
};

struct st_request
{
    unsigned space, targets, scope, pages;
    uint64_t identity, generation;
    uintptr_t first;
};

struct st_permit
{
    struct st_request request;
};

struct st_port
{
    /* publish failure guarantees no PTE change. invalidate is local to cpu and
     * may over-invalidate; failure publishes no ack. It never touches a retired
     * frame. notify can fail after partial delivery; the request remains live.
     * Callbacks are stable and nonreentrant except notify may call st_handle. */
    int (*publish)(void *, struct st_request);
    int (*invalidate)(void *, unsigned cpu, struct st_request);
    int (*notify)(void *, struct st_request, unsigned targets);
    void *context;
};

struct st_slot
{
    atomic_uint pending, targets, scope, pages;
    atomic_uintptr_t first;
    atomic_uint_fast64_t generation, ack[ST_CPUS];
};

struct st_world
{
    unsigned cpus, current[ST_CPUS];
    uint64_t identity[ST_SPACES];
    struct sl_lock membership;
    struct st_slot slot[ST_SPACES];
    struct st_port port;
};

/* Stable world, positive distinct space identities, no reinitialization while
 * any request/caller can survive. Fixed online CPU set; no hotplug inference.
 * Entry/leave execute on that cpu before/after any address-space use. Finish
 * returns a reclaim permit only for this request; caller retains frame owner
 * and prevents any other mapping from authorizing reuse of the same frame.
 * Outputs/control objects are disjoint. No raw control-field mutation. */
int st_init(struct st_world *, unsigned cpus, uint64_t first_id,
            uint64_t second_id, struct st_port);
int st_enter(struct st_world *, unsigned cpu, unsigned space,
             struct sl_stats *);
int st_leave(struct st_world *, unsigned cpu, struct sl_stats *);
int st_begin(struct st_world *, unsigned actor, unsigned space, uintptr_t first,
             unsigned pages, unsigned scope, struct st_request *,
             struct sl_stats *);
/* Handler never takes the membership/kernel lock. Its caller runs locally on
 * cpu, including a dedicated interrupt path with declared stack/register ABI. */
int st_handle(struct st_world *, unsigned cpu, struct st_request);
int st_request(const struct st_world *, unsigned space, struct st_request *);
int st_retry(struct st_world *, struct st_request, struct sl_stats *);
int st_finish(struct st_world *, struct st_request, int expired,
              struct st_permit *, struct sl_stats *);
#endif
