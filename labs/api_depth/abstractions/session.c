/* Intentional learner scaffold: implement SESSION.md. */
#include "session.h"
void as_session_init(struct as_session *s,struct ac_ledger *l){(void)s;(void)l;}
enum as_status as_construct(struct as_session *s,unsigned f,as_construction_observer_fn o,void *c){(void)s;(void)f;(void)o;(void)c;return AS_UNIMPLEMENTED;}
enum as_status as_event(struct as_session *s,enum as_event e){(void)s;(void)e;return AS_UNIMPLEMENTED;}
enum as_status as_opaque_create(struct ac_allocate a,struct ac_ledger *l,struct as_opaque **o){(void)a;(void)l;(void)o;return AS_UNIMPLEMENTED;}
enum as_status as_opaque_event(struct as_opaque *o,enum as_event e){(void)o;(void)e;return AS_UNIMPLEMENTED;}
enum as_status as_opaque_read(const struct as_opaque *o,unsigned *v){(void)o;(void)v;return AS_UNIMPLEMENTED;}
enum as_status as_opaque_destroy(struct as_opaque *o){(void)o;return AS_UNIMPLEMENTED;}
const struct as_ops_v1 as_direct_ops={{0,0,0},0};
const struct as_ops_v2 as_adapted_ops={{{0,0,0},0},0};
enum as_status as_backend(const void *d,size_t n,void *c,struct as_backend *o){(void)d;(void)n;(void)c;(void)o;return AS_UNIMPLEMENTED;}
enum as_status as_backend_run(struct as_backend b,enum as_event e){(void)b;(void)e;return AS_UNIMPLEMENTED;}
enum as_status as_backend_flush(struct as_backend b){(void)b;return AS_UNIMPLEMENTED;}
