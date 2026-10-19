#include "move.h"
OwStatus omm_registry_create(OmmMemory m,V4Allocator a,uint32_t g,OmmRegistry **o){(void)m;(void)a;(void)g;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus omm_registry_destroy(OmmRegistry **r){(void)r;return OW_NOT_IMPLEMENTED;}
OwStatus omm_token_open(OmmRegistry *r,const int32_t *p,size_t n,OmmToken *o){(void)r;(void)p;(void)n;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus omm_token_move(OmmRegistry *r,OmmToken *d,OmmToken *s){(void)r;(void)d;(void)s;return OW_NOT_IMPLEMENTED;}
OwStatus omm_token_read(OmmRegistry *r,OmmToken t,size_t i,int32_t *o){(void)r;(void)t;(void)i;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus omm_token_consume(OmmRegistry *r,OmmToken *t){(void)r;(void)t;return OW_NOT_IMPLEMENTED;}
