#include "completion.h"
enum msg_status mfx_create(struct mfx_completion **o,mfo_receiver_fn r,void *c,void (*d)(void *),void *x,bool f)
{(void)o;(void)r;(void)c;(void)d;(void)x;(void)f;return MSG_FAILED;}
enum msg_status mfx_retain(struct mfx_completion *c,struct mfx_completion **o){(void)c;(void)o;return MSG_FAILED;}
void mfx_drop(struct mfx_completion **c){(void)c;}
enum msg_status mfx_cancel(struct mfx_completion *c){(void)c;return MSG_FAILED;}
enum msg_status mfx_complete(struct mfx_completion *c,enum msg_status s,struct mfo_value *v){(void)c;(void)s;(void)v;return MSG_FAILED;}
