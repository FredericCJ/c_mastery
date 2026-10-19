#include "dispatch_laws.h"
int ds_copy_checked(DSCopyView v,unsigned char *p,size_t n,size_t *out){(void)v;(void)p;(void)n;(void)out;return DS_FAILED;}
int ds_multi_views(const DSMulti *p,DSView out[2]){(void)p;(void)out;return DS_FAILED;}
int ds_capture_checked(DSErased *p,DSCaptureSpec s,const int32_t *v,size_t n,void (*d)(void *),void *c){(void)p;(void)s;(void)v;(void)n;(void)d;(void)c;return DS_FAILED;}
