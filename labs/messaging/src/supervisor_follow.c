#include "supervisor.h"
MsgStatus mfs_init(MfsTree*t,size_t p,size_t c,uint64_t w,MfsOpen o,MfsClose x,void*r,MfaHandler h,void*a[4][4],MfaDeadLetter d,void*z){(void)t;(void)p;(void)c;(void)w;(void)o;(void)x;(void)r;(void)h;(void)a;(void)d;(void)z;return MSG_FAILED;}
MsgStatus mfs_fault(MfsTree*t,unsigned g,unsigned c,uint64_t n,MsgStrategy s,unsigned f,MfsReport*r){(void)t;(void)g;(void)c;(void)n;(void)s;(void)f;(void)r;return MSG_FAILED;}
MsgStatus mfs_destroy(MfsTree*t){(void)t;return MSG_FAILED;}
