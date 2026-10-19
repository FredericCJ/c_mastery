#include "analysis.h"
enum ca_status ca_analyze(const CAModel*m,const RTATask*t,unsigned n,unsigned i,unsigned l,RTAResult*r,enum rta_status*s){(void)m;(void)t;(void)n;(void)i;(void)l;(void)r;(void)s;return CA_UNIMPLEMENTED;}
enum ca_status ca_compatible(const CAConfig*a,const CAConfig*b){(void)a;(void)b;return CA_UNIMPLEMENTED;}
enum ca_status ca_evidence(const CAConfig*c,const CAEvidence*e,enum ca_kind k,uint64_t*n,uint64_t*d){(void)c;(void)e;(void)k;(void)n;(void)d;return CA_UNIMPLEMENTED;}
