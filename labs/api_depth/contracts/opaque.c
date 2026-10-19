#include "session_v1.h"
/* Intentional learner scaffold, including failure and ownership behavior. */
int ac_opaque_create(uint32_t i,uint32_t n,AcAllocate a,AcOpaque **o){(void)i;(void)n;(void)a;(void)o;return 2;}
int ac_opaque_get(const AcOpaque *s,AcExposedV1 *o){(void)s;(void)o;return 1;}
void ac_opaque_destroy(AcOpaque *s){(void)s;}
size_t ac_opaque_size(void){return 0;}
size_t ac_opaque_alignment(void){return 0;}
