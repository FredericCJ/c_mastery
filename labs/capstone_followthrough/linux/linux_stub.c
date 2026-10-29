#include "linux.h"
void csl_init(struct cs_linux *a,struct cs_service *s,int e){(void)a;(void)s;(void)e;}
uint64_t csl_token(const struct cs_linux *a,unsigned s){(void)a;(void)s;return 0;}
int csl_open(struct cs_linux *a,unsigned s,int f,int e){(void)a;(void)s;(void)f;(void)e;return -1;}
int csl_close(struct cs_linux *a,unsigned s){(void)a;(void)s;return -1;}
enum cs_status csl_event(struct cs_linux *a,uint64_t t){(void)a;(void)t;return CS_INVALID;}
struct cs_turn csl_turn(struct cs_linux *a,unsigned s,size_t b,int u){(void)a;(void)s;(void)b;(void)u;return(struct cs_turn){0,CS_INVALID,0};}
struct cs_io csl_write(void *c,const unsigned char *b,size_t n){(void)c;(void)b;(void)n;return(struct cs_io){0,CS_IO_ERROR,4096};}
