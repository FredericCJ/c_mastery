#ifndef OS_FOLLOWTHROUGH_H
#define OS_FOLLOWTHROUGH_H
#include "oslab.h"
#include <stddef.h>
#include <stdint.h>
typedef enum {
  OF_OK,
  OF_INPUT,
  OF_LIMIT,
  OF_WAIT,
  OF_DONE,
  OF_ERROR,
  OF_UNIMPLEMENTED = 99
} OfStatus;
typedef struct {
  os_schedule schedule;
  unsigned dispatch_charge[OS_MAX_JOBS], queued[OS_MAX_JOBS];
} OfSchedule;
/* -1 idle,-2 reserved dispatch overhead. Selection is fixed through overhead;
   arrivals during it are admitted before the next scheduling decision. */
OfStatus of_schedule(const os_job *, size_t, os_policy, unsigned, unsigned,
                     OfSchedule *);
typedef enum { OF_YIELD, OF_SLEEP, OF_WEIGHT } OfEventKind;
typedef struct {
  unsigned time, job;
  OfEventKind kind;
  unsigned value;
} OfEvent;
typedef struct {
  int timeline[256];
  unsigned level[256], used[256], service[3], boosts, normalizations;
  uint64_t final_pass[3];
} OfFair;
/* Three initially runnable workers; events at boundaries; sleep value=duration,
   weight value=new weight; yield preserves consumed allotment. */
OfStatus of_fair(unsigned, unsigned, const unsigned[3], uint64_t, unsigned,
                 const OfEvent *, size_t, unsigned, OfFair *);
typedef enum { OF_FIFO, OF_LRU, OF_CLOCK, OF_OPT } OfPagePolicy;
typedef struct {
  uint8_t page, write;
} OfReference;
typedef struct {
  int victim;
  unsigned hit, live_mask, dirty_mask, hand;
  uint8_t resident[8];
} OfPageStep;
typedef struct {
  OfPageStep steps[64];
  unsigned faults, writebacks;
} OfPages;
OfStatus of_pages(const OfReference *, size_t, unsigned, OfPagePolicy,
                  OfPages *);
typedef struct {
  size_t wanted, done;
  unsigned terminal;
  int error;
} OfTransfer;
/* raw is signed Linux-style return; record contract forbids partial success. */
OfStatus of_progress(OfTransfer *, ptrdiff_t, unsigned);
#endif
