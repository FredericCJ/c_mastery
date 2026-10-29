#include "http_depth.h"
int hd_length(const unsigned char *s,size_t n,uint32_t cap,uint32_t *out){(void)s;(void)n;(void)cap;(void)out;return HD_UNIMPLEMENTED;}
int hd_date(unsigned a,uint32_t d,uint32_t s,char *o,size_t c,size_t *n){(void)a;(void)d;(void)s;(void)o;(void)c;(void)n;return HD_UNIMPLEMENTED;}
int hd_admit(struct hd_pool *p,int f,unsigned a,struct hd_handle *h){(void)p;(void)f;(void)a;(void)h;return HD_UNIMPLEMENTED;}
int hd_interest(struct hd_pool *p,struct hd_handle h,unsigned m){(void)p;(void)h;(void)m;return HD_UNIMPLEMENTED;}
int hd_event(const struct hd_pool *p,struct hd_handle h,int f,unsigned r,unsigned *o){(void)p;(void)h;(void)f;(void)r;(void)o;return HD_UNIMPLEMENTED;}
int hd_close(struct hd_pool *p,struct hd_handle h,int *o){(void)p;(void)h;(void)o;return HD_UNIMPLEMENTED;}
