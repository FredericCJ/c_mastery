#include "contract_modes.h"
er_code ecm_init(EcmBuffer *b,unsigned char *p,size_t n,size_t r){(void)b;(void)p;(void)n;(void)r;return ER_NOT_IMPLEMENTED;}
er_code ecm_write(EcmBuffer *b,er_mode m,size_t i,unsigned char v){(void)b;(void)m;(void)i;(void)v;return ER_NOT_IMPLEMENTED;}
er_code ecm_shared_init(EcmShared *s,unsigned char *p,size_t n,int c,size_t r){(void)s;(void)p;(void)n;(void)c;(void)r;return ER_NOT_IMPLEMENTED;}
er_code ecm_shared_write(EcmShared *s,er_mode m,size_t i,unsigned char v){(void)s;(void)m;(void)i;(void)v;return ER_NOT_IMPLEMENTED;}
er_code ecm_shared_counter(EcmShared *s,er_mode m,int a){(void)s;(void)m;(void)a;return ER_NOT_IMPLEMENTED;}
er_code ecm_shared_snapshot(EcmShared *s,EcmSnapshot *o){(void)s;(void)o;return ER_NOT_IMPLEMENTED;}
void ecm_shared_destroy(EcmShared *s){(void)s;}
