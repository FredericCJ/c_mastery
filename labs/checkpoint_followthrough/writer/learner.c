#include "writer.h"
int cwf_byte_step(algo_ring *a,CWFByte *b,FWriter c,void *d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int cwf_byte_recover(CWFByte *a){(void)a;return -1;}
int cwf_span_begin(CWFSpan *a,const unsigned char *b,size_t c,unsigned d){(void)a;(void)b;(void)c;(void)d;return -1;}
int cwf_span_take(algo_ring *a,CWFSpan *b,size_t c,unsigned d){(void)a;(void)b;(void)c;(void)d;return -1;}
CPWStatus cwf_span_step(CWFSpan *a,CPWWrite b,void *c){(void)a;(void)b;(void)c;return CPW_INVALID;}
CPWStatus cwf_span_recover(CWFSpan *a){(void)a;return CPW_INVALID;}
int cwf_error(const CPWTransfer *a,CWFError *b){(void)a;(void)b;return -1;}
