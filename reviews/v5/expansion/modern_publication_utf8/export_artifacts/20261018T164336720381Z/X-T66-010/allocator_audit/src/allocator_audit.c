#include "allocator_audit.h"
OwStatus oaa_init(OaaAudit *a,OaaUpstream b){(void)a;(void)b;return OW_NOT_IMPLEMENTED;}
OwStatus oaa_set_report(OaaAudit *a,OaaReporter b,void *c){(void)a;(void)b;(void)c;return OW_NOT_IMPLEMENTED;}
V4Allocator oaa_allocator(OaaAudit *a){(void)a;return (V4Allocator){0};}
OwStatus oaa_resize(OaaAudit *a,int32_t *b,size_t c,int32_t **d){(void)a;(void)b;(void)c;(void)d;return OW_NOT_IMPLEMENTED;}
