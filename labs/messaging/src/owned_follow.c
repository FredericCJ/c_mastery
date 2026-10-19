#include "owned.h"
void mfo_drop(MfoValue*v){(void)v;}
MsgStatus mfc_init(MfcHub*h,size_t a,size_t b){(void)h;(void)a;(void)b;return MSG_FAILED;}
MsgStatus mfc_send(MfcHub*h,unsigned c,MfoValue*v,bool w,MfcAfterSubmit f,void*x){(void)h;(void)c;(void)v;(void)w;(void)f;(void)x;return MSG_FAILED;}
MsgStatus mfc_select(MfcHub*h,unsigned m,bool w,unsigned*c,MfoValue*v){(void)h;(void)m;(void)w;(void)c;(void)v;return MSG_FAILED;}
MsgStatus mfc_close(MfcHub*h,unsigned c){(void)h;(void)c;return MSG_FAILED;}
void mfc_cancel(MfcHub*h){(void)h;}
MsgStatus mfc_destroy(MfcHub*h){(void)h;return MSG_FAILED;}
MsgStatus mfa_init(MfaActor*a,MfaHandler f,void*c,MfaDeadLetter d,void*x){(void)a;(void)f;(void)c;(void)d;(void)x;return MSG_FAILED;}
MsgStatus mfa_send(MfaActor*a,uint64_t g,MfoValue*v){(void)a;(void)g;(void)v;return MSG_FAILED;}
MsgStatus mfa_step(MfaActor*a){(void)a;return MSG_FAILED;}
MsgStatus mfa_restart(MfaActor*a){(void)a;return MSG_FAILED;}
MsgStatus mfa_destroy(MfaActor*a){(void)a;return MSG_FAILED;}
