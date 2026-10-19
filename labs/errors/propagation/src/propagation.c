#include "propagation.h"
er_code epr_init(EprParser *p,unsigned s){(void)p;(void)s;return ER_NOT_IMPLEMENTED;}
EprStep epr_feed(EprParser *p,const unsigned char *b,size_t n,unsigned f,uint32_t *o){(void)p;(void)b;(void)n;(void)f;(void)o;EprStep r={ER_NOT_IMPLEMENTED,0,0,0};return r;}
er_result epr_pair_explicit(const unsigned char *b,size_t n,uint32_t *o){(void)b;(void)n;(void)o;return er_error(ER_NOT_IMPLEMENTED,0);}
er_result epr_pair_macro(const unsigned char *b,size_t n,uint32_t *o){(void)b;(void)n;(void)o;return er_error(ER_NOT_IMPLEMENTED,0);}
