#include "facilities.h"
enum lf_status lf_add(int32_t a,int32_t b,int32_t *o){(void)a;(void)b;(void)o;return LF_INVALID;}
enum lf_status lf_mixed(int32_t a,uint32_t b,uint16_t *o){(void)a;(void)b;(void)o;return LF_INVALID;}
enum lf_status lf_bits(uint32_t v,struct lf_bits *o){(void)v;(void)o;return LF_INVALID;}
enum lf_status lf_div(uint64_t a,uint64_t b,uint64_t *o){(void)a;(void)b;(void)o;return LF_INVALID;}
enum lf_status lf_select(unsigned a,unsigned s,unsigned *o){(void)a;(void)s;(void)o;return LF_INVALID;}
