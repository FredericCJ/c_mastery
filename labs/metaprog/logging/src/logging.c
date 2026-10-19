#include "logging.h"
MplField mpl_number(uint32_t value) { MplField f={MPL_NUMBER,value,NULL,0};return f; }
MplField mpl_bytes(const unsigned char *bytes,size_t length) { MplField f={MPL_BYTES,0,bytes,length};return f; }
int mpl_write(MplSink *s,const MplSite *p,const MplField *f,size_t n) { (void)s;(void)p;(void)f;(void)n;return MPL_INVALID; }
int mpl_log0(MplSink *s,MplSite p) { (void)s;(void)p;return MPL_INVALID; }
int mpl_log1(MplSink *s,MplSite p,MplField a) { (void)a;return mpl_log0(s,p); }
int mpl_log2(MplSink *s,MplSite p,MplField a,MplField b) { (void)b;return mpl_log1(s,p,a); }
int mpl_log3(MplSink *s,MplSite p,MplField a,MplField b,MplField c) { (void)c;return mpl_log2(s,p,a,b); }
int mpl_log4(MplSink *s,MplSite p,MplField a,MplField b,MplField c,MplField d) { (void)d;return mpl_log3(s,p,a,b,c); }
