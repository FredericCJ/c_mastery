#include "versions.h"
MsgStatus mfv_init(MfvStore*s,MflAllocator a,MfoValue*v){(void)s;(void)a;(void)v;return MSG_INVALID;}
MsgStatus mfv_snapshot(MfvStore*s,MsgSnapshot*p){(void)s;(void)p;return MSG_INVALID;}
MsgStatus mfv_read(MfvStore*s,MsgSnapshot p,MflShared**v){(void)s;(void)p;(void)v;return MSG_INVALID;}
MsgStatus mfv_drop_snapshot(MfvStore*s,MsgSnapshot p){(void)s;(void)p;return MSG_INVALID;}
MsgStatus mfv_publish(MfvStore*s,MfvClone c,void*x,const void*v){(void)s;(void)c;(void)x;(void)v;return MSG_INVALID;}
MsgStatus mfv_destroy(MfvStore*s){(void)s;return MSG_INVALID;}
MsgStatus mfk_init(MfkStore*s,unsigned a,unsigned b){(void)s;(void)a;(void)b;return MSG_INVALID;}
MsgStatus mfk_begin(MfkStore*s,MfkTxn*t){(void)s;(void)t;return MSG_INVALID;}
MsgStatus mfk_read(MfkTxn*t,unsigned k,unsigned*v){(void)t;(void)k;(void)v;return MSG_INVALID;}
MsgStatus mfk_nonzero(MfkTxn*t,unsigned*v){(void)t;(void)v;return MSG_INVALID;}
MsgStatus mfk_write(MfkTxn*t,unsigned k,unsigned v){(void)t;(void)k;(void)v;return MSG_INVALID;}
MsgStatus mfk_commit(MfkTxn*t,bool p){(void)t;(void)p;return MSG_INVALID;}
void mfk_destroy(MfkStore*s){(void)s;}
