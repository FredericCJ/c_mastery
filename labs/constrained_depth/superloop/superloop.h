#ifndef CMASTERY_CONSTRAINED_SUPERLOOP_H
#define CMASTERY_CONSTRAINED_SUPERLOOP_H
#include <stddef.h>
#include <stdint.h>
enum { SC_OK,SC_INVALID,SC_FULL,SC_EMPTY,SC_RANGE,SC_CLOSED,SC_UNBOUNDED };
enum { SC_READY,SC_DONE,SC_CANCELLED };
enum { SC_COALESCE,SC_REJECT_STALE,SC_LOSSLESS };
enum { SC_ROUND_ROBIN,SC_PRIORITY,SC_BUDGETED };
typedef struct { uint64_t round,longest,response; } ScBound;
typedef struct { unsigned char input[128],staged[128]; size_t length,cursor; unsigned state,emissions; } ScTask;
typedef struct { uint64_t id,value; } ScEvent;
typedef struct { ScEvent data[3]; unsigned head,count; uint64_t accepted,rejected; } ScEvents;
typedef struct { uint64_t value,updates,superseded; unsigned pending; } ScLatest;
typedef struct { uint64_t period,next,last,seen,lost,items[2]; unsigned count,exhausted,policy,observed; } ScReleases;
typedef struct { uint64_t last,epoch; unsigned mode,cursor,high_used,observed; } ScSchedule;
/* Caller-owned, externally serialized objects. Init requires no surviving call,
 * borrowed result or old task; no reinitialization of live work. Inputs, outputs
 * and controls are valid/disjoint. Source bytes are copied on task admission.
 * Counts/time are exact uint64, not wrapped clocks. Refusals preserve outputs.
 * No callbacks, heap, implicit asynchronous cleanup or scheduler library. */
int sc_bound(const uint64_t *,size_t,uint64_t,int,ScBound *);
int sc_task_init(ScTask *,const unsigned char *,size_t);
int sc_task_step(ScTask *,size_t,size_t *);
int sc_task_cancel(ScTask *);
int sc_task_result(const ScTask *,unsigned char *,size_t,size_t *);
int sc_events_init(ScEvents *);
int sc_events_push(ScEvents *,ScEvent);
int sc_events_pop(ScEvents *,ScEvent *);
int sc_latest_init(ScLatest *);
int sc_latest_put(ScLatest *,uint64_t);
int sc_latest_take(ScLatest *,uint64_t *);
int sc_releases_init(ScReleases *,uint64_t,uint64_t,unsigned);
int sc_releases_poll(ScReleases *,uint64_t);
int sc_releases_pop(ScReleases *,uint64_t *);
int sc_schedule_init(ScSchedule *,unsigned);
int sc_schedule_next(ScSchedule *,uint64_t,unsigned,unsigned *);
#endif
