#include "list.h"
int sx_list_init(struct sx_list*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
int sx_register(struct sx_list*a,struct sx_participant*b){(void)a;(void)b;return MB_FAILED;}
int sx_depart(struct sx_list*a,struct sx_participant b){(void)a;(void)b;return MB_FAILED;}
int sx_enter(struct sx_list*a,struct sx_participant b){(void)a;(void)b;return MB_FAILED;}
int sx_leave(struct sx_list*a,struct sx_participant b){(void)a;(void)b;return MB_FAILED;}
int sx_append(struct sx_list*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
int sx_remove(struct sx_list*a,unsigned*b){(void)a;(void)b;return MB_FAILED;}
int sx_observe(struct sx_list*a,struct sx_participant b,unsigned c,struct sx_observed*d){(void)a;(void)b;(void)c;(void)d;return MB_FAILED;}
int sx_protect(struct sx_list*a,struct sx_participant b,unsigned c,struct sx_observed d,struct sx_protected*e){(void)a;(void)b;(void)c;(void)d;(void)e;return MB_FAILED;}
int sx_read(struct sx_list*a,struct sx_participant b,struct sx_protected c,unsigned*d){(void)a;(void)b;(void)c;(void)d;return MB_FAILED;}
int sx_clear(struct sx_list*a,struct sx_participant b,unsigned c){(void)a;(void)b;(void)c;return MB_FAILED;}
int sx_collect(struct sx_list*a,unsigned*b){(void)a;(void)b;return MB_FAILED;}
int sx_advance(struct sx_list*a){(void)a;return MB_FAILED;}
