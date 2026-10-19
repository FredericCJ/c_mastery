#ifndef SMP_SHOOTDOWN_H
#define SMP_SHOOTDOWN_H
#include "locks.h"
#include <stdint.h>
enum { ST_OK,ST_INVALID,ST_STALE,ST_BUSY,ST_FULL,ST_TIMEOUT,ST_PORT,ST_SIGNAL,ST_INCOMPLETE };
enum { ST_PAGE,ST_GLOBAL,ST_CONTEXT,ST_CPUS=4,ST_SPACES=2,ST_NO_SPACE=2,ST_MAX_PAGES=4 };
typedef struct {unsigned space,targets,scope,pages;uint64_t identity,generation;uintptr_t first;} StRequest;
typedef struct {StRequest request;} StPermit;
typedef struct {
 /* publish failure guarantees no PTE change. invalidate is local to cpu and
  * may over-invalidate; failure publishes no ack. It never touches a retired
  * frame. notify can fail after partial delivery; the request remains live.
  * Callbacks are stable and nonreentrant except notify may call st_handle. */
 int (*publish)(void *,StRequest);
 int (*invalidate)(void *,unsigned cpu,StRequest);
 int (*notify)(void *,StRequest,unsigned targets);
 void *context;
} StPort;
typedef struct {atomic_uint pending,targets,scope,pages;atomic_uintptr_t first;atomic_uint_fast64_t generation,ack[ST_CPUS];} StSlot;
typedef struct {unsigned cpus,current[ST_CPUS];uint64_t identity[ST_SPACES];SlLock membership;StSlot slot[ST_SPACES];StPort port;} StWorld;
/* Stable world, positive distinct space identities, no reinitialization while
 * any request/caller can survive. Fixed online CPU set; no hotplug inference.
 * Entry/leave execute on that cpu before/after any address-space use. Finish
 * returns a reclaim permit only for this request; caller retains frame owner
 * and prevents any other mapping from authorizing reuse of the same frame.
 * Outputs/control objects are disjoint. No raw control-field mutation. */
int st_init(StWorld *,unsigned cpus,uint64_t first_id,uint64_t second_id,StPort);
int st_enter(StWorld *,unsigned cpu,unsigned space,SlStats *);
int st_leave(StWorld *,unsigned cpu,SlStats *);
int st_begin(StWorld *,unsigned actor,unsigned space,uintptr_t first,unsigned pages,unsigned scope,StRequest *,SlStats *);
/* Handler never takes the membership/kernel lock. Its caller runs locally on
 * cpu, including a dedicated interrupt path with declared stack/register ABI. */
int st_handle(StWorld *,unsigned cpu,StRequest);
int st_request(const StWorld *,unsigned space,StRequest *);
int st_retry(StWorld *,StRequest,SlStats *);
int st_finish(StWorld *,StRequest,int expired,StPermit *,SlStats *);
#endif
