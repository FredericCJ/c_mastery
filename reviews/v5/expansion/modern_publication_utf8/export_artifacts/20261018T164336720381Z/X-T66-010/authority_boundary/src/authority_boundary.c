#include "authority_boundary.h"
/* Compiling exercise scaffold: implement the checked boundary and trusted core. */
OafStatus oaf_init(OafRoot *r,OwTenantArena *a,unsigned t,unsigned p){(void)r;(void)a;(void)t;(void)p;return OAF_INVALID;}
OafStatus oaf_view(OafRoot *r,OafView *v){(void)r;(void)v;return OAF_INVALID;}
OafStatus oaf_subview(const OafView *v,uint32_t o,uint32_t n,unsigned p,OafView *out){(void)v;(void)o;(void)n;(void)p;(void)out;return OAF_INVALID;}
OafStatus oaf_revoke(OafRoot *r){(void)r;return OAF_INVALID;}
OafStatus oaf_encode(OafRequest r,unsigned char b[OAF_WIRE_SIZE]){(void)r;(void)b;return OAF_INVALID;}
OafStatus oaf_decode(const unsigned char *b,size_t n,OafRequest *r){(void)b;(void)n;(void)r;return OAF_INVALID;}
OafStatus oaf_write(const OafView *v,OafRequest r,int32_t x){(void)v;(void)r;(void)x;return OAF_INVALID;}
OafStatus oaf_read(const OafView *v,OafRequest r,int32_t out[8]){(void)v;(void)r;(void)out;return OAF_INVALID;}
OafStatus oaf_wire_write(const OafView *v,const unsigned char *b,size_t n,int32_t x){(void)v;(void)b;(void)n;(void)x;return OAF_INVALID;}
OafStatus oaf_wire_read(const OafView *v,const unsigned char *b,size_t n,int32_t out[8]){(void)v;(void)b;(void)n;(void)out;return OAF_INVALID;}
