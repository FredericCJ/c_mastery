#include "scope.h"
OwStatus ocs_run(const OcsPlan *p,unsigned *e,OcsReport *r){(void)p;(void)e;(void)r;return OW_NOT_IMPLEMENTED;}
OwStatus ocs_begin(OcsRecovery *r,OcsTicket *t){(void)r;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus ocs_push(OcsTicket t,OcsCloser c,void *p,unsigned i){(void)t;(void)c;(void)p;(void)i;return OW_NOT_IMPLEMENTED;}
OwStatus ocs_recover(OcsTicket t,OcsRecovered *r){(void)t;(void)r;return OW_NOT_IMPLEMENTED;}
