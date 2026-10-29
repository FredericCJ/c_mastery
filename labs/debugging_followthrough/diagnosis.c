#include "diagnosis.h"
/* Deliberately incomplete learner implementation. */
void dx_log(struct dx_log *p,unsigned e){(void)p;(void)e;}
int dx_reduce(unsigned n,unsigned b,dx_oracle_fn o,void *a,struct dx_reduction *r){(void)n;(void)b;(void)o;(void)a;(void)r;return -1;}
int dx_memory(struct dx_memory *m,unsigned a,unsigned g,unsigned i,unsigned e){(void)m;(void)a;(void)g;(void)i;(void)e;return -1;}
unsigned dx_saturate(unsigned a,unsigned b){(void)a;(void)b;return 0;}
int dx_checked(unsigned a,unsigned b,unsigned *p){(void)a;(void)b;(void)p;return -1;}
int dx_close(struct dx_descriptor *d,unsigned t,unsigned p,int r){(void)d;(void)t;(void)p;(void)r;return -1;}
int dx_write_step(struct dx_writer *w,int r,unsigned b){(void)w;(void)r;(void)b;return -1;}
