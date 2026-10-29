#include "artifacts.h"
int sa_digest(const unsigned char *b,size_t n,unsigned char d[32]){(void)b;(void)n;(void)d;return -1;}
void sa_manifest(const struct sa_artifact *a,unsigned char m[44]){(void)a;(void)m;}
void sa_transition(const struct sa_trust *t,const unsigned char k[32],uint32_t e,uint32_t v,unsigned char m[49]){(void)t;(void)k;(void)e;(void)v;(void)m;}
int sa_verify(const struct sa_trust *t,const struct sa_artifact *a,const unsigned char *b,size_t n,struct sa_report *r){(void)t;(void)a;(void)b;(void)n;(void)r;return -1;}
int sa_rotate(struct sa_trust *t,const unsigned char k[32],uint32_t e,uint32_t m,const unsigned char s[64]){(void)t;(void)k;(void)e;(void)m;(void)s;return -1;}
void sa_service_init(struct sa_service *s){(void)s;}
enum sc_result sa_submit(struct sa_service *s,const struct sa_report *r,const struct sc_authority *a,const unsigned char *p,size_t n,struct sc_ticket *t){(void)s;(void)r;(void)a;(void)p;(void)n;(void)t;return SC_UNIMPLEMENTED;}
enum sc_result sa_verified_submit(struct sa_service *s,const struct sa_trust *r,const struct sa_artifact *v,const unsigned char *b,size_t z,const struct sc_authority *a,const unsigned char *p,size_t n,struct sc_ticket *t){(void)s;(void)r;(void)v;(void)b;(void)z;(void)a;(void)p;(void)n;(void)t;return SC_UNIMPLEMENTED;}
enum sc_result sa_finish(struct sa_service *s,struct sc_ticket t,int c,const struct sc_authority *a,int f,uint32_t *o,uint32_t *p,char b[4]){(void)s;(void)t;(void)c;(void)a;(void)f;(void)o;(void)p;(void)b;return SC_UNIMPLEMENTED;}
enum sc_result sa_shutdown(struct sa_service *s){(void)s;return SC_UNIMPLEMENTED;}
