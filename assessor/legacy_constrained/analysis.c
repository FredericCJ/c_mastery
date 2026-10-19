#include "analysis.h"
#ifndef CA_MUTANT
#define CA_MUTANT 0
#endif
enum ca_status ca_analyze(const CAModel*m,const RTATask*t,unsigned n,unsigned i,unsigned l,RTAResult*r,enum rta_status*s){
 if(!m||!t||!r||!s||!n||n>RTA_TASKS||i>=n||!l||l>RTA_STEPS)return CA_INVALID;
 if((CA_MUTANT!=1&&m->premises!=CA_PREMISES)||
    (m->arrivals!=CA_PERIODIC&&m->arrivals!=CA_SPORADIC_MINIMUM))return CA_UNSUPPORTED;
 for(unsigned j=0;j<=i;++j){
  if(!t[j].period||!t[j].deadline)return CA_INVALID;
  if(t[j].jitter||t[j].deadline>t[j].period)return CA_UNSUPPORTED;
 }
 RTAResult staged;enum rta_status status=rta_response(t,n,i,l,&staged);
 if(status==RTA_INVALID||status==RTA_OVERFLOW||status==RTA_UNIMPLEMENTED)return CA_INVALID;
 if(CA_MUTANT==2&&status==RTA_LIMIT)status=RTA_CONVERGED;
 *r=staged;*s=status;return CA_OK;
}
enum ca_status ca_compatible(const CAConfig*a,const CAConfig*b){
 if(!a||!b||!a->clock_hz||!b->clock_hz||!a->resolution_ns||!b->resolution_ns)return CA_INVALID;
 for(unsigned i=0;i<32;++i)if(a->image_sha256[i]!=b->image_sha256[i])return CA_DIFFERENT;
 if(a->clock_hz!=b->clock_hz||a->interrupt_policy!=b->interrupt_policy||
    (CA_MUTANT!=3&&a->cache_policy!=b->cache_policy)||
    (CA_MUTANT!=4&&a->probe_policy!=b->probe_policy)||a->resolution_ns!=b->resolution_ns)return CA_DIFFERENT;
 return CA_OK;
}
enum ca_status ca_evidence(const CAConfig*c,const CAEvidence*e,enum ca_kind k,uint64_t*n,uint64_t*d){
 if(!c||!e||!n||!d||k<CA_REQUIREMENT||k>CA_OBSERVED_MAX||!e->denominator)return CA_INVALID;
 if(CA_MUTANT!=5&&e->kind!=k)return CA_INVALID;
 enum ca_status status=ca_compatible(c,&e->config);if(status!=CA_OK)return status;
 *n=e->numerator;*d=e->denominator;return CA_OK;
}
