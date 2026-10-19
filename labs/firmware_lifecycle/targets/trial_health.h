#ifndef B06_TRIAL_HEALTH_H
#define B06_TRIAL_HEALTH_H
#include "trial_intent.h"
/* Finite original application workload, not a security MAC or universal health. */
int th_health_accept(unsigned core,uint32_t generation,uint32_t expected_generation,uint32_t result);
/* Live FWPeer snapshot taken only after other core is parked in bank2 and no
 * outstanding users remain; a matching number alone does not stop hardware. */
enum FHResult th_commit_begin(FMRecycle *,FMIO,const BSSelection *old,const BSSelection *trial,const FWPeer *,uint32_t parked_generation);
#endif
