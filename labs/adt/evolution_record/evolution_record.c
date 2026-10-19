#include "evolution_record.h"
int ade_read(const unsigned char *p,size_t n,AdeRecord *o){(void)p;(void)n;(void)o;return ADE_INVALID;}
int ade_write(const AdeRecord *r,int c,unsigned char *p,size_t n,size_t *u){(void)r;(void)c;(void)p;(void)n;(void)u;return ADE_INVALID;}
int ade_tag(const AdeRecord *r,const unsigned char k[32],unsigned char t[32]){(void)r;(void)k;(void)t;return ADE_INVALID;}
int ade_check(const AdeRecord *r,const unsigned char k[32],const unsigned char t[32]){(void)r;(void)k;(void)t;return ADE_INVALID;}
int ade_apply(const AdeRecord *r,const unsigned char k[32],const unsigned char t[32],AdeService *s){(void)r;(void)k;(void)t;(void)s;return ADE_INVALID;}
