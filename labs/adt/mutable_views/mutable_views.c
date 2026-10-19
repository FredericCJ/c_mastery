#include "mutable_views.h"
AdvStatus adv_geometry(size_t a,size_t b,size_t c,size_t d,size_t e,AdvGeometry *o){(void)a;(void)b;(void)c;(void)d;(void)e;(void)o;return ADV_INVALID;}
AdvStatus adv_view(uint32_t *p,size_t a,size_t b,size_t c,size_t d,size_t e,AdvView *o){(void)p;(void)a;(void)b;(void)c;(void)d;(void)e;(void)o;return ADV_INVALID;}
ADView adv_readonly(AdvView v){(void)v;return(ADView){0};}
AdvStatus adv_set(AdvView v,size_t r,size_t c,uint32_t x){(void)v;(void)r;(void)c;(void)x;return ADV_INVALID;}
AdvStatus adv_transpose(AdvView v,AdvView *o){(void)v;(void)o;return ADV_INVALID;}
AdvStatus adv_subview(AdvView v,size_t r,size_t c,size_t n,size_t m,AdvView *o){(void)v;(void)r;(void)c;(void)n;(void)m;(void)o;return ADV_INVALID;}
AdvStatus adv_reduce(ADView v,AdvMode m,AdvReduction *o){(void)v;(void)m;(void)o;return ADV_INVALID;}
