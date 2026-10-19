#include "checkpoint_vector.h"
int cpv_detect(CPVProbe p){(void)p;return 0;}
int cpv_available(void){return 0;}
CPNStatus cpv_plan(CPVPlan *p,CPVBackend b){(void)p;(void)b;return CPN_INVALID;}
CPNStatus cpv_run(const CPVPlan *p,CPNDouble *s,const double *i,size_t n,double *o,size_t c){
    (void)p;(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;
}
