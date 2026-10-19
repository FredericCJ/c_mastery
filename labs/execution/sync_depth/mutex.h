#ifndef CMASTERY_SYNC_DEPTH_MUTEX_H
#define CMASTERY_SYNC_DEPTH_MUTEX_H
#include "cmx.h"

typedef struct {
  cmx_kernel *kernel;
  cmx_mutex mutex;
  unsigned depth, maximum;
} CDRecursive;
typedef struct {
  int32_t balance;
  uint32_t checksum;
} CDRecord;
typedef void (*CDRecordObserver)(void *);

int cdm_recursive_init(CDRecursive *, cmx_kernel *, unsigned maximum);
int cdm_recursive_lock(CDRecursive *, unsigned task, cmx_deadline);
int cdm_recursive_unlock(CDRecursive *, unsigned task);
int cdm_recursive_close(CDRecursive *);
int cdm_ordered_lock(cmx_kernel *, cmx_mutex *const *locks,
                     const unsigned *ranks, unsigned count, unsigned target,
                     unsigned task, cmx_deadline);
void cdm_record_init(CDRecord *, int32_t balance);
int cdm_record_read(cmx_kernel *, cmx_mutex *, unsigned task, const CDRecord *,
                    CDRecord *out);
int cdm_record_apply(cmx_kernel *, cmx_mutex *, unsigned task, CDRecord *,
                     int32_t delta, CDRecordObserver, void *observer_context);
#endif
