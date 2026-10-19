#include "stream.h"
MsgStatus mfq_init(MfqStream*s,unsigned c,MfoValue*v,MsgNext n,MsgTerminal t,bool f){(void)s;(void)c;(void)v;(void)n;(void)t;(void)f;return MSG_INVALID;}
MsgStatus mfq_acquire(MfqStream*s,MfqHandle*h){(void)s;(void)h;return MSG_INVALID;}
void mfq_release(MfqHandle*h){(void)h;}
MsgStatus mfq_request(MfqHandle*h,uint64_t n){(void)h;(void)n;return MSG_INVALID;}
MsgStatus mfq_submit(MfqHandle*h,unsigned v,MfqPolicy p){(void)h;(void)v;(void)p;return MSG_INVALID;}
MsgStatus mfq_end(MfqHandle*h,MsgStatus s){(void)h;(void)s;return MSG_INVALID;}
MsgStatus mfq_cancel(MfqHandle*h){(void)h;return MSG_INVALID;}
MsgStatus mfq_close(MfqStream*s){(void)s;return MSG_INVALID;}
MsgStatus mfq_join(MfqStream*s){(void)s;return MSG_INVALID;}
MsgStatus mfq_idle(MfqStream*s){(void)s;return MSG_INVALID;}
MsgStatus mfq_stats(MfqStream*s,MfqStats*out){(void)s;(void)out;return MSG_INVALID;}
MsgStatus mfq_destroy(MfqStream*s){(void)s;return MSG_INVALID;}
