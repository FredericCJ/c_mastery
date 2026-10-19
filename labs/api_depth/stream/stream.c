#include "stream.h"
void sf_init(SFStream*s){if(s)*s=(SFStream){0};}
int sf_start(SFStream*a,const unsigned char*b,size_t c,SFHandle*d){(void)a;(void)b;(void)c;(void)d;return SF_UNIMPLEMENTED;}
int sf_pump(SFHandle a,SFWrite b,void*c,unsigned d,size_t*e){(void)a;(void)b;(void)c;(void)d;(void)e;return SF_UNIMPLEMENTED;}
int sf_cancel(SFHandle a){(void)a;return SF_UNIMPLEMENTED;}
