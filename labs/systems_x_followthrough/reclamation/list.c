#include "list.h"
int sx_list_init(SxList*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
int sx_register(SxList*a,SxParticipant*b){(void)a;(void)b;return MB_FAILED;}
int sx_depart(SxList*a,SxParticipant b){(void)a;(void)b;return MB_FAILED;}
int sx_enter(SxList*a,SxParticipant b){(void)a;(void)b;return MB_FAILED;}
int sx_leave(SxList*a,SxParticipant b){(void)a;(void)b;return MB_FAILED;}
int sx_append(SxList*a,unsigned b){(void)a;(void)b;return MB_FAILED;}
int sx_remove(SxList*a,unsigned*b){(void)a;(void)b;return MB_FAILED;}
int sx_observe(SxList*a,SxParticipant b,unsigned c,SxObserved*d){(void)a;(void)b;(void)c;(void)d;return MB_FAILED;}
int sx_protect(SxList*a,SxParticipant b,unsigned c,SxObserved d,SxProtected*e){(void)a;(void)b;(void)c;(void)d;(void)e;return MB_FAILED;}
int sx_read(SxList*a,SxParticipant b,SxProtected c,unsigned*d){(void)a;(void)b;(void)c;(void)d;return MB_FAILED;}
int sx_clear(SxList*a,SxParticipant b,unsigned c){(void)a;(void)b;(void)c;return MB_FAILED;}
int sx_collect(SxList*a,unsigned*b){(void)a;(void)b;return MB_FAILED;}
int sx_advance(SxList*a){(void)a;return MB_FAILED;}
