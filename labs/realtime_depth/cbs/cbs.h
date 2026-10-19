#ifndef CMASTERY_REALTIME_CBS_H
#define CMASTERY_REALTIME_CBS_H
#include "cmx.h"
/* Distinct policies, not aliases. SOFT_BOOK uses the inspected >= equality
 * boundary; SOFT_RETAINED preserves cmx's strict > boundary. HARD_DOCUMENT
 * is an original finite comparison of documented Q,D,P throttling rules. */
enum cb_policy {CB_SOFT_BOOK,CB_SOFT_RETAINED,CB_HARD_DOCUMENT};
typedef struct {
  cmx_cbs core;
  uint32_t relative_deadline,last;
  enum cb_policy policy;
  unsigned queued,throttled;
} CBBudget;
enum cmx_result cb_init(CBBudget *,uint32_t q,uint32_t d,uint32_t p,enum cb_policy,uint32_t now);
enum cmx_result cb_arrive(CBBudget *,uint32_t now);
enum cmx_result cb_idle(CBBudget *);
enum cmx_result cb_update(CBBudget *,uint32_t now);
enum cmx_result cb_charge(CBBudget *,uint32_t now,uint32_t amount);
int cb_invariant(const CBBudget *);
/* Two independent clients; exact sufficient density admission, not necessity. */
enum cmx_result cb_admit(uint32_t q0,uint32_t d0,uint32_t p0,
                         uint32_t q1,uint32_t d1,uint32_t p1,unsigned *accepted);
#define CB_REQUESTS 24u
#define CB_HORIZON 128u
typedef struct {unsigned client;uint32_t release,cost,application_deadline;} CBRequest;
typedef struct {
  uint32_t quota[2],deadline[2],period[2];enum cb_policy policy[2];
  unsigned count,horizon,capacity;CBRequest request[CB_REQUESTS];
} CBWorkload;
typedef struct {
  unsigned admitted[CB_REQUESTS],rejected[CB_REQUESTS],missed[CB_REQUESTS];
  uint32_t finish[CB_REQUESTS],executed[CB_REQUESTS],remaining[CB_REQUESTS];
  int selected[CB_HORIZON];
  uint32_t budget[CB_HORIZON][2],deadline[CB_HORIZON][2],pending[CB_HORIZON][2];
} CBTrace;
enum cmx_result cb_workload(const CBWorkload *,CBTrace *);
typedef struct {
  unsigned owner[16],waiter_blocked[16],charged[16];
  uint32_t budget[16][2],deadline[16][2];
  unsigned ticks,waiter_finish,waiter_missed;
} CBMutexTrace;
/* Actual retained CMX mutex/EDF control operations in a finite model, not
 * a real-thread kernel or a bandwidth-inheritance implementation. */
enum cmx_result cb_mutex(unsigned critical_ticks,CBMutexTrace *);
#endif
