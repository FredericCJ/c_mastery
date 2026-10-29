#include "evolution_record.h"
int ade_read(const unsigned char *p,size_t n,struct ade_record *o){(void)p;(void)n;(void)o;return ADE_INVALID;}
int ade_write(const struct ade_record *r,int c,unsigned char *p,size_t n,size_t *u){(void)r;(void)c;(void)p;(void)n;(void)u;return ADE_INVALID;}
int ade_tag(const struct ade_record *r,const unsigned char k[32],unsigned char t[32]){(void)r;(void)k;(void)t;return ADE_INVALID;}
int ade_check(const struct ade_record *r,const unsigned char k[32],const unsigned char t[32]){(void)r;(void)k;(void)t;return ADE_INVALID;}
int ade_apply(const struct ade_record *r,const unsigned char k[32],const unsigned char t[32],struct ade_service *s){(void)r;(void)k;(void)t;(void)s;return ADE_INVALID;}
