#include "once.h"
int sx_once_init(struct sx_once*a,struct sx_journal*b,unsigned c,unsigned d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;return MB_FAILED;}
int sx_once_enter(struct sx_once*a,unsigned b,unsigned c[8],unsigned*d){(void)a;(void)b;(void)c;(void)d;return MB_FAILED;}
int sx_once_step(struct sx_once*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
int sx_once_cancel(struct sx_once*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
