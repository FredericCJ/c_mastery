#include "linux.h"
void csl_init(CsLinux *a,CsService *s,int e){(void)a;(void)s;(void)e;}
uint64_t csl_token(const CsLinux *a,unsigned s){(void)a;(void)s;return 0;}
int csl_open(CsLinux *a,unsigned s,int f,int e){(void)a;(void)s;(void)f;(void)e;return -1;}
int csl_close(CsLinux *a,unsigned s){(void)a;(void)s;return -1;}
CsStatus csl_event(CsLinux *a,uint64_t t){(void)a;(void)t;return CS_INVALID;}
CsTurn csl_turn(CsLinux *a,unsigned s,size_t b,int u){(void)a;(void)s;(void)b;(void)u;return(CsTurn){0,CS_INVALID,0};}
CsIO csl_write(void *c,const unsigned char *b,size_t n){(void)c;(void)b;(void)n;return(CsIO){0,CS_IO_ERROR,4096};}
