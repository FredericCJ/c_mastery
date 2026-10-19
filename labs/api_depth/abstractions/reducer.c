#include "reducer.h"
AsStatus am_switch(AmMachine m,AmEvent e,AmTransition *o){(void)m;(void)e;(void)o;return AS_UNIMPLEMENTED;}
AsStatus am_table(AmMachine m,AmEvent e,AmTransition *o){(void)m;(void)e;(void)o;return AS_UNIMPLEMENTED;}
AsStatus am_functions(AmMachine m,AmEvent e,AmTransition *o){(void)m;(void)e;(void)o;return AS_UNIMPLEMENTED;}
AsStatus am_sparse(unsigned s,unsigned q,AmEvent e,AmTransition *o){(void)s;(void)q;(void)e;(void)o;return AS_UNIMPLEMENTED;}
AsStatus am_execute(AmBackend *b,AmEffect e){(void)b;(void)e;return AS_UNIMPLEMENTED;}
AsStatus am_hot_acquire(AmHot *h,AmLease *l){(void)h;(void)l;return AS_UNIMPLEMENTED;}
AsStatus am_hot_replace(AmHot *h,AmStrategy s){(void)h;(void)s;return AS_UNIMPLEMENTED;}
AsStatus am_hot_release(AmHot *h,AmLease *l){(void)h;(void)l;return AS_UNIMPLEMENTED;}
AsStatus am_hot_close(AmHot *h){(void)h;return AS_UNIMPLEMENTED;}
