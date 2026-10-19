#include "transactions.h"
MsgStatus mft_commit_phase(MsgTxn*t,MftHook h,void*c){(void)t;(void)h;(void)c;return MSG_INVALID;}
MsgStatus mft_init(MftStore*s,MflAllocator a){(void)s;(void)a;return MSG_INVALID;}
MsgStatus mft_begin(MftStore*s,MftTxn*t){(void)s;(void)t;return MSG_INVALID;}
MsgStatus mft_read(MftTxn*t,unsigned k,const MfoValue**v){(void)t;(void)k;(void)v;return MSG_INVALID;}
MsgStatus mft_write(MftTxn*t,unsigned k,MfoValue*v){(void)t;(void)k;(void)v;return MSG_INVALID;}
MsgStatus mft_remove(MftTxn*t,unsigned k){(void)t;(void)k;return MSG_INVALID;}
MsgStatus mft_log(MftTxn*t,unsigned e){(void)t;(void)e;return MSG_INVALID;}
void mft_abort(MftTxn*t){(void)t;}
MsgStatus mft_commit(MftTxn*t,MftHook h,void*c){(void)t;(void)h;(void)c;return MSG_INVALID;}
MsgStatus mft_run(MftStore*s,unsigned l,MftBody b,void*c,unsigned*a){(void)s;(void)l;(void)b;(void)c;(void)a;return MSG_INVALID;}
MsgStatus mft_destroy(MftStore*s){(void)s;return MSG_INVALID;}
