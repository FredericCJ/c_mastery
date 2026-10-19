#ifndef CMASTERY_RESILIENCE_COMPOSITION_H
#define CMASTERY_RESILIENCE_COMPOSITION_H
#include "service_io.h"
#include "hedge.h"
#include "retry.h"
#include "timers.h"
typedef struct {unsigned drops[2],delay[2],deadline,notify_at,cancel_at,capacity,retry_after;bool hedge,reset_before_ack;} RzcConfig;
typedef struct {
    unsigned started,dispatched,retries,dropped,denied,applied,replays,deliveries,acknowledged,resets;
    unsigned allocated_responses,freed_responses,reply_at,quiet_at,notification_at;
    uint64_t tokens;
    int winner,total;
    unsigned journal_records,ack_records;
    RztWork timer_work;
} RzcResult;
int rzc_run(RzcConfig,RzjIO,RzjIO,RzfAllocator,RzcResult *);
#endif
