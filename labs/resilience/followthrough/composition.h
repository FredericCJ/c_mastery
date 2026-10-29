#ifndef CMASTERY_RESILIENCE_COMPOSITION_H
#define CMASTERY_RESILIENCE_COMPOSITION_H
#include "service_io.h"
#include "hedge.h"
#include "retry.h"
#include "timers.h"
struct rzc_config {unsigned drops[2],delay[2],deadline,notify_at,cancel_at,capacity,retry_after;bool hedge,reset_before_ack;};
struct rzc_result {
    unsigned started,dispatched,retries,dropped,denied,applied,replays,deliveries,acknowledged,resets;
    unsigned allocated_responses,freed_responses,reply_at,quiet_at,notification_at;
    uint64_t tokens;
    int winner,total;
    unsigned journal_records,ack_records;
    struct rzt_work timer_work;
};
int rzc_run(struct rzc_config,struct rzj_io,struct rzj_io,struct rzf_allocator,struct rzc_result *);
#endif
