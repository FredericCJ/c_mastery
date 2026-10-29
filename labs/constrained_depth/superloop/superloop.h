#ifndef CMASTERY_CONSTRAINED_SUPERLOOP_H
#define CMASTERY_CONSTRAINED_SUPERLOOP_H
#include <stddef.h>
#include <stdint.h>
enum { SC_OK,SC_INVALID,SC_FULL,SC_EMPTY,SC_RANGE,SC_CLOSED,SC_UNBOUNDED };
enum { SC_READY,SC_DONE,SC_CANCELLED };
enum { SC_COALESCE,SC_REJECT_STALE,SC_LOSSLESS };
enum { SC_ROUND_ROBIN,SC_PRIORITY,SC_BUDGETED };
struct sc_bound { uint64_t round,longest,response; };
struct sc_task { unsigned char input[128],staged[128]; size_t length,cursor; unsigned state,emissions; };
struct sc_event { uint64_t id,value; };
struct sc_events { struct sc_event data[3]; unsigned head,count; uint64_t accepted,rejected; };
struct sc_latest { uint64_t value,updates,superseded; unsigned pending; };
struct sc_releases { uint64_t period,next,last,seen,lost,items[2]; unsigned count,exhausted,policy,observed; };
struct sc_schedule { uint64_t last,epoch; unsigned mode,cursor,high_used,observed; };
/* Caller-owned, externally serialized objects. Init requires no surviving call,
 * borrowed result or old task; no reinitialization of live work. Inputs, outputs
 * and controls are valid/disjoint. Source bytes are copied on task admission.
 * Counts/time are exact uint64, not wrapped clocks. Refusals preserve outputs.
 * No callbacks, heap, implicit asynchronous cleanup or scheduler library. */
int sc_bound(const uint64_t *,size_t,uint64_t,int,struct sc_bound *);
int sc_task_init(struct sc_task *,const unsigned char *,size_t);
int sc_task_step(struct sc_task *,size_t,size_t *);
int sc_task_cancel(struct sc_task *);
int sc_task_result(const struct sc_task *,unsigned char *,size_t,size_t *);
int sc_events_init(struct sc_events *);
int sc_events_push(struct sc_events *,struct sc_event);
int sc_events_pop(struct sc_events *,struct sc_event *);
int sc_latest_init(struct sc_latest *);
int sc_latest_put(struct sc_latest *,uint64_t);
int sc_latest_take(struct sc_latest *,uint64_t *);
int sc_releases_init(struct sc_releases *,uint64_t,uint64_t,unsigned);
int sc_releases_poll(struct sc_releases *,uint64_t);
int sc_releases_pop(struct sc_releases *,uint64_t *);
int sc_schedule_init(struct sc_schedule *,unsigned);
int sc_schedule_next(struct sc_schedule *,uint64_t,unsigned,unsigned *);
#endif
