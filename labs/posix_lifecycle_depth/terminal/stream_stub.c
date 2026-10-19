#include "stream.h"
void pt_session_init(PTSession *s){(void)s;}
CPStatus pt_session_open(PTSession *s){(void)s;return CP_INVALID;}
void pt_session_close(PTSession *s){(void)s;}
CPStatus pt_session_feed(PTSession *s,const unsigned char *b,size_t n,size_t *u){(void)s;(void)b;(void)n;(void)u;return CP_INVALID;}
CPStatus pt_session_take(PTSession *s,CPMessage *m){(void)s;(void)m;return CP_INVALID;}
