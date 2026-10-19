#include "artifacts.h"
int sa_digest(const unsigned char *b,size_t n,unsigned char d[32]){(void)b;(void)n;(void)d;return -1;}
void sa_manifest(const SAArtifact *a,unsigned char m[44]){(void)a;(void)m;}
void sa_transition(const SATrust *t,const unsigned char k[32],uint32_t e,uint32_t v,unsigned char m[49]){(void)t;(void)k;(void)e;(void)v;(void)m;}
int sa_verify(const SATrust *t,const SAArtifact *a,const unsigned char *b,size_t n,SAReport *r){(void)t;(void)a;(void)b;(void)n;(void)r;return -1;}
int sa_rotate(SATrust *t,const unsigned char k[32],uint32_t e,uint32_t m,const unsigned char s[64]){(void)t;(void)k;(void)e;(void)m;(void)s;return -1;}
void sa_service_init(SAService *s){(void)s;}
enum sc_result sa_submit(SAService *s,const SAReport *r,const SCAuthority *a,const unsigned char *p,size_t n,SCTicket *t){(void)s;(void)r;(void)a;(void)p;(void)n;(void)t;return SC_UNIMPLEMENTED;}
enum sc_result sa_verified_submit(SAService *s,const SATrust *r,const SAArtifact *v,const unsigned char *b,size_t z,const SCAuthority *a,const unsigned char *p,size_t n,SCTicket *t){(void)s;(void)r;(void)v;(void)b;(void)z;(void)a;(void)p;(void)n;(void)t;return SC_UNIMPLEMENTED;}
enum sc_result sa_finish(SAService *s,SCTicket t,int c,const SCAuthority *a,int f,uint32_t *o,uint32_t *p,char b[4]){(void)s;(void)t;(void)c;(void)a;(void)f;(void)o;(void)p;(void)b;return SC_UNIMPLEMENTED;}
enum sc_result sa_shutdown(SAService *s){(void)s;return SC_UNIMPLEMENTED;}
