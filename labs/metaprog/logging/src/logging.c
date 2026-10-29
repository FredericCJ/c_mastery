#include "logging.h"
struct mpl_field mpl_number(uint32_t value) { struct mpl_field f={MPL_NUMBER,value,NULL,0};return f; }
struct mpl_field mpl_bytes(const unsigned char *bytes,size_t length) { struct mpl_field f={MPL_BYTES,0,bytes,length};return f; }
int mpl_write(struct mpl_sink *s,const struct mpl_site *p,const struct mpl_field *f,size_t n) { (void)s;(void)p;(void)f;(void)n;return MPL_INVALID; }
int mpl_log0(struct mpl_sink *s,struct mpl_site p) { (void)s;(void)p;return MPL_INVALID; }
int mpl_log1(struct mpl_sink *s,struct mpl_site p,struct mpl_field a) { (void)a;return mpl_log0(s,p); }
int mpl_log2(struct mpl_sink *s,struct mpl_site p,struct mpl_field a,struct mpl_field b) { (void)b;return mpl_log1(s,p,a); }
int mpl_log3(struct mpl_sink *s,struct mpl_site p,struct mpl_field a,struct mpl_field b,struct mpl_field c) { (void)c;return mpl_log2(s,p,a,b); }
int mpl_log4(struct mpl_sink *s,struct mpl_site p,struct mpl_field a,struct mpl_field b,struct mpl_field c,struct mpl_field d) { (void)d;return mpl_log3(s,p,a,b,c); }
