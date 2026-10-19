#include "completion.h"
MsgStatus mfx_create(MfxCompletion **o,MfoReceiver r,void *c,void (*d)(void *),void *x,bool f)
{(void)o;(void)r;(void)c;(void)d;(void)x;(void)f;return MSG_FAILED;}
MsgStatus mfx_retain(MfxCompletion *c,MfxCompletion **o){(void)c;(void)o;return MSG_FAILED;}
void mfx_drop(MfxCompletion **c){(void)c;}
MsgStatus mfx_cancel(MfxCompletion *c){(void)c;return MSG_FAILED;}
MsgStatus mfx_complete(MfxCompletion *c,MsgStatus s,MfoValue *v){(void)c;(void)s;(void)v;return MSG_FAILED;}
