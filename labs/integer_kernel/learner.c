#include "integer.h"
enum fi_status fi_binary(struct fi_range r,enum fi_operation o,int a,int b,int *v){(void)r;(void)o;(void)a;(void)b;(void)v;return FI_UNFINISHED;}
enum fi_status fi_saturate(struct fi_range r,enum fi_operation o,int a,int b,int *v,unsigned *c){(void)r;(void)o;(void)a;(void)b;(void)v;(void)c;return FI_UNFINISHED;}
enum fi_status fi_divide(struct fi_range r,enum fi_division o,int a,int b,int *q,int *m){(void)r;(void)o;(void)a;(void)b;(void)q;(void)m;return FI_UNFINISHED;}
enum fi_status fi_mod_positive(int a,int m,int *v){(void)a;(void)m;(void)v;return FI_UNFINISHED;}
enum fi_status fi_to_size(intmax_t v,size_t c,size_t *o){(void)v;(void)c;(void)o;return FI_UNFINISHED;}
enum fi_status fi_extent(size_t p,size_t c,size_t w,size_t l,size_t *o){(void)p;(void)c;(void)w;(void)l;(void)o;return FI_UNFINISHED;}
enum fi_status fi_layout(size_t p,intmax_t c,size_t w,size_t l,size_t *o){(void)p;(void)c;(void)w;(void)l;(void)o;return FI_UNFINISHED;}
enum fi_status fi_window(uintmax_t b,uintmax_t p,size_t n,size_t *o){(void)b;(void)p;(void)n;(void)o;return FI_UNFINISHED;}
enum fi_status fi_encode_cells(uint_least32_t w,unsigned int o[4]){(void)w;(void)o;return FI_UNFINISHED;}
enum fi_status fi_decode_cells(const unsigned int c[4],uint_least32_t *o){(void)c;(void)o;return FI_UNFINISHED;}
enum fi_status fi_encode_octets(uint_least32_t w,unsigned char o[4]){(void)w;(void)o;return FI_UNFINISHED;}
