#ifndef CMASTERY_SYNC_DEPTH_MUTEX_H
#define CMASTERY_SYNC_DEPTH_MUTEX_H
#include "cmx.h"

struct cd_recursive {
  cmx_kernel *kernel;
  cmx_mutex mutex;
  unsigned depth, maximum;
};
struct cd_record {
  int32_t balance;
  uint32_t checksum;
};
typedef void (*cd_record_observer_fn)(void *);

int cdm_recursive_init(struct cd_recursive *, cmx_kernel *, unsigned maximum);
int cdm_recursive_lock(struct cd_recursive *, unsigned task, cmx_deadline);
int cdm_recursive_unlock(struct cd_recursive *, unsigned task);
int cdm_recursive_close(struct cd_recursive *);
int cdm_ordered_lock(cmx_kernel *, cmx_mutex *const *locks,
                     const unsigned *ranks, unsigned count, unsigned target,
                     unsigned task, cmx_deadline);
void cdm_record_init(struct cd_record *, int32_t balance);
int cdm_record_read(cmx_kernel *, cmx_mutex *, unsigned task, const struct cd_record *,
                    struct cd_record *out);
int cdm_record_apply(cmx_kernel *, cmx_mutex *, unsigned task, struct cd_record *,
                     int32_t delta, cd_record_observer_fn, void *observer_context);
#endif
