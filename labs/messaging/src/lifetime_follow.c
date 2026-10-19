#include "lifetime.h"
MsgStatus mfl_make(MflAllocator a,MfoValue*v,MflShared**out){(void)a;(void)v;(void)out;return MSG_FAILED;}
MsgStatus mfl_retain(MflShared*p){(void)p;return MSG_INVALID;}
void mfl_drop(MflShared**p){(void)p;}
const MfoValue*mfl_borrow(const MflShared*p){(void)p;return NULL;}
MsgStatus mfg_init(MfgDeque*q,MflAllocator a,size_t n){(void)q;(void)a;(void)n;return MSG_INVALID;}
MsgStatus mfg_push(MfgDeque*q,MfoValue*v){(void)q;(void)v;return MSG_INVALID;}
MsgStatus mfg_pop(MfgDeque*q,MflShared**out){(void)q;(void)out;return MSG_INVALID;}
MsgStatus mfg_steal(MfgDeque*q,MflShared**out,MfgPause f,void*c){(void)q;(void)out;(void)f;(void)c;return MSG_INVALID;}
MsgStatus mfg_destroy(MfgDeque*q){(void)q;return MSG_INVALID;}
MsgStatus mfp_init(MfpPointer*p,MflShared**v){(void)p;(void)v;return MSG_INVALID;}
MsgStatus mfp_snapshot(MfpPointer*p,MflShared**v){(void)p;(void)v;return MSG_INVALID;}
MsgStatus mfp_replace(MfpPointer*p,MflShared**v){(void)p;(void)v;return MSG_INVALID;}
MsgStatus mfp_destroy(MfpPointer*p){(void)p;return MSG_INVALID;}
