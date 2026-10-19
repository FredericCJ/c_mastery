#include "overload.h"
enum od_status od_admit(ODSet *s,ODTask t,unsigned h,enum od_decision *d){(void)s;(void)t;(void)h;(void)d;return OD_UNIMPLEMENTED;}
enum od_status od_run(const ODSet *s,const ODRequest *r,unsigned n,unsigned c,unsigned h,enum od_policy p,unsigned sp,uint32_t sw,uint32_t re,ODResult *o){(void)s;(void)r;(void)n;(void)c;(void)h;(void)p;(void)sp;(void)sw;(void)re;(void)o;return OD_UNIMPLEMENTED;}
enum od_status od_backpressure(ODDependency *o){(void)o;return OD_UNIMPLEMENTED;}
