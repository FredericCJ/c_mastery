#ifndef CMASTERY_REALTIME_PORT_H
#define CMASTERY_REALTIME_PORT_H
#include "server.h"
/* Serialized, bounded single-core adapter. The caller owns the kernel and
 * controls every transition of the bound task. See CONTRACTS.md before use. */
typedef struct {
  cmx_kernel *kernel;
  SSBudget budget;
  unsigned task, generation, bound, closed, sleeping;
  uint32_t last, wait_sequence, wake, charged, throttles;
} SRPort;
enum cmx_result sr_init(SRPort *, cmx_kernel *, unsigned task,
                        uint32_t quota, uint32_t period);
/* Exactly once before cmx_tick(k, boundary), with boundary == k->now+1.
 * A running server is charged one conservative quantum. A non-OK result
 * requires immediate containment; the caller must not continue dispatch. */
enum cmx_result sr_tick(SRPort *, uint32_t boundary);
/* Close may cancel only this adapter's exact outstanding sleep. Payload
 * work must stop before the server returns; close itself does not reap. */
enum cmx_result sr_close(SRPort *);
enum cmx_result sr_detach(SRPort *);
#endif
