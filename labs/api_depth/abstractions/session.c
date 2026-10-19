/* Intentional learner scaffold: implement SESSION.md. */
#include "session.h"
void as_session_init(AsSession *s,AcLedger *l){(void)s;(void)l;}
AsStatus as_construct(AsSession *s,unsigned f,AsConstructionObserver o,void *c){(void)s;(void)f;(void)o;(void)c;return AS_UNIMPLEMENTED;}
AsStatus as_event(AsSession *s,AsEvent e){(void)s;(void)e;return AS_UNIMPLEMENTED;}
AsStatus as_opaque_create(AcAllocate a,AcLedger *l,AsOpaque **o){(void)a;(void)l;(void)o;return AS_UNIMPLEMENTED;}
AsStatus as_opaque_event(AsOpaque *o,AsEvent e){(void)o;(void)e;return AS_UNIMPLEMENTED;}
AsStatus as_opaque_read(const AsOpaque *o,unsigned *v){(void)o;(void)v;return AS_UNIMPLEMENTED;}
AsStatus as_opaque_destroy(AsOpaque *o){(void)o;return AS_UNIMPLEMENTED;}
const AsOpsV1 as_direct_ops={{0,0,0},0};
const AsOpsV2 as_adapted_ops={{{0,0,0},0},0};
AsStatus as_backend(const void *d,size_t n,void *c,AsBackend *o){(void)d;(void)n;(void)c;(void)o;return AS_UNIMPLEMENTED;}
AsStatus as_backend_run(AsBackend b,AsEvent e){(void)b;(void)e;return AS_UNIMPLEMENTED;}
AsStatus as_backend_flush(AsBackend b){(void)b;return AS_UNIMPLEMENTED;}
