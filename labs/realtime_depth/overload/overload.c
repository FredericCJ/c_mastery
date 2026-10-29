#include "overload.h"
enum od_status od_admit(struct od_set *s,struct od_task t,unsigned h,enum od_decision *d){(void)s;(void)t;(void)h;(void)d;return OD_UNIMPLEMENTED;}
enum od_status od_run(const struct od_set *s,const struct od_request *r,unsigned n,unsigned c,unsigned h,enum od_policy p,unsigned sp,uint32_t sw,uint32_t re,struct od_result *o){(void)s;(void)r;(void)n;(void)c;(void)h;(void)p;(void)sp;(void)sw;(void)re;(void)o;return OD_UNIMPLEMENTED;}
enum od_status od_backpressure(struct od_dependency *o){(void)o;return OD_UNIMPLEMENTED;}
