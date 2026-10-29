#include "analysis.h"
enum ca_status ca_analyze(const struct ca_model*m,const struct rta_task*t,unsigned n,unsigned i,unsigned l,struct rta_result*r,enum rta_status*s){(void)m;(void)t;(void)n;(void)i;(void)l;(void)r;(void)s;return CA_UNIMPLEMENTED;}
enum ca_status ca_compatible(const struct ca_config*a,const struct ca_config*b){(void)a;(void)b;return CA_UNIMPLEMENTED;}
enum ca_status ca_evidence(const struct ca_config*c,const struct ca_evidence*e,enum ca_kind k,uint64_t*n,uint64_t*d){(void)c;(void)e;(void)k;(void)n;(void)d;return CA_UNIMPLEMENTED;}
