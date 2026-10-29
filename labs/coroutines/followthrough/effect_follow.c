#include "effect_follow.h"
int ch_create(struct co_allocator a,const struct ch_handler*b,unsigned c,const unsigned char d[4],struct ch_effect**e){(void)a;(void)b;(void)c;(void)d;(void)e;return CO_UNIMPLEMENTED;}
int ch_step(struct ch_effect*a,const struct ch_handler*b,uint32_t*c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int ch_clone(const struct ch_effect*a,struct ch_effect**b){(void)a;(void)b;return CO_UNIMPLEMENTED;}
int ch_abort(struct ch_effect*a){(void)a;return CO_UNIMPLEMENTED;}
int ch_destroy(struct ch_effect**a){(void)a;return CO_UNIMPLEMENTED;}
