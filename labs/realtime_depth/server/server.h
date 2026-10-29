#ifndef CMASTERY_REALTIME_SERVER_H
#define CMASTERY_REALTIME_SERVER_H
#include "cmx_advanced.h"
#define SS_REQUESTS 32u
#define SS_HORIZON 128u
#define SS_NONE UINT32_MAX

enum ss_merge
{
    SS_REJECT,
    SS_DELAY_LATEST
};

enum ss_policy
{
    SS_BACKGROUND,
    SS_POLLING,
    SS_DISPATCH
};

struct ss_budget
{
    cmx_server core;
    unsigned capacity;
    enum ss_merge merge;
};
/* Fresh, externally serialized storage; no caller mutation. Every non-OK
 * result preserves the entire object. Time moves forward by less than 2^31
 * between calls; period is positive and less than that half range. */
enum cmx_result ss_init(struct ss_budget *, uint32_t q, uint32_t period,
                        uint32_t start, unsigned capacity, enum ss_merge);
enum cmx_result ss_update(struct ss_budget *, uint32_t now);
enum cmx_result ss_charge(struct ss_budget *, uint32_t now, uint32_t amount);
int ss_invariant(const struct ss_budget *);

struct ss_comparison
{
    struct ss_budget dispatch, grouped;
};
/* Bounded comparison only: one complete active interval shorter than P.
 * The active interval includes higher-priority preemption; each event is
 * one actual server-service unit. No claim of a complete classical server. */
enum cmx_result ss_compare(uint32_t q, uint32_t period, uint32_t begin,
                           uint32_t end, const uint32_t *service, unsigned n,
                           struct ss_comparison *);

struct ss_request
{
    uint32_t release, cost;
};

struct ss_workload
{
    unsigned count, capacity, horizon;
    uint32_t q, period;
    struct ss_request request[SS_REQUESTS];
};

struct ss_trace
{
    unsigned admitted[SS_REQUESTS], rejected[SS_REQUESTS];
    uint32_t remaining[SS_REQUESTS], finish[SS_REQUESTS], executed[SS_REQUESTS];
    uint32_t service[SS_HORIZON], budget[SS_HORIZON], backlog[SS_HORIZON];
    uint32_t replenished[SS_HORIZON], supervisor, periodic, periodic_misses;
    uint32_t periodic_dropped, periodic_remaining;
    unsigned records[SS_HORIZON];
};
/* Discrete one-CPU model: supervisor C1/T5, lower periodic C2/T10/D10,
 * server between them except background. FCFS, reject-new capacity1..8;
 * zero-cost requests complete without capacity. All times are nonwrapping.
 * Input/output disjoint and live. Invalid input leaves output unchanged. */
enum cmx_result ss_workload(const struct ss_workload *, enum ss_policy,
                            struct ss_trace *);
#endif
