#include "checkpoint_writer.h"
CPWResult cpw_raw(long n){(void)n;return (CPWResult){CPW_ERROR,0,1};}
CPWResult cpw_libc(long n,int e){(void)n;(void)e;return (CPWResult){CPW_ERROR,0,1};}
CPWResult cpw_script(long n){(void)n;return (CPWResult){CPW_ERROR,0,1};}
CPWStatus cpw_begin(CPWTransfer *t,const unsigned char *d,size_t n,unsigned o){(void)t;(void)d;(void)n;(void)o;return CPW_INVALID;}
CPWStatus cpw_step(CPWTransfer *t,CPWWrite w,void *c){(void)t;(void)w;(void)c;return CPW_INVALID;}
CPWStatus cpw_recover(CPWTransfer *t){(void)t;return CPW_INVALID;}
void cpw_cleanup(CPWTransfer *t,int c){(void)t;(void)c;}
