#ifndef OS_FOLLOWTHROUGH_H
#define OS_FOLLOWTHROUGH_H
#include "oslab.h"
#include <stddef.h>
#include <stdint.h>

enum of_status
{
    OF_OK,
    OF_INPUT,
    OF_LIMIT,
    OF_WAIT,
    OF_DONE,
    OF_ERROR,
    OF_UNIMPLEMENTED = 99
};

struct of_schedule
{
    os_schedule schedule;
    unsigned dispatch_charge[OS_MAX_JOBS], queued[OS_MAX_JOBS];
};
/* -1 idle,-2 reserved dispatch overhead. Selection is fixed through overhead;
   arrivals during it are admitted before the next scheduling decision. */
enum of_status of_schedule(const os_job *, size_t, os_policy, unsigned,
                           unsigned, struct of_schedule *);

enum of_event_kind
{
    OF_YIELD,
    OF_SLEEP,
    OF_WEIGHT
};

struct of_event
{
    unsigned time, job;
    enum of_event_kind kind;
    unsigned value;
};

struct of_fair
{
    int timeline[256];
    unsigned level[256], used[256], service[3], boosts, normalizations;
    uint64_t final_pass[3];
};
/* Three initially runnable workers; events at boundaries; sleep value=duration,
   weight value=new weight; yield preserves consumed allotment. */
enum of_status of_fair(unsigned, unsigned, const unsigned[3], uint64_t,
                       unsigned, const struct of_event *, size_t, unsigned,
                       struct of_fair *);

enum of_page_policy
{
    OF_FIFO,
    OF_LRU,
    OF_CLOCK,
    OF_OPT
};

struct of_reference
{
    uint8_t page, write;
};

struct of_page_step
{
    int victim;
    unsigned hit, live_mask, dirty_mask, hand;
    uint8_t resident[8];
};

struct of_pages
{
    struct of_page_step steps[64];
    unsigned faults, writebacks;
};
enum of_status of_pages(const struct of_reference *, size_t, unsigned,
                        enum of_page_policy, struct of_pages *);

struct of_transfer
{
    size_t wanted, done;
    unsigned terminal;
    int error;
};
/* raw is signed Linux-style return; record contract forbids partial success. */
enum of_status of_progress(struct of_transfer *, ptrdiff_t, unsigned);
#endif
