#include "dispatch.h"
#ifdef DS_BORROW_RETURN
static int *call(const unsigned char *p,size_t n,int a,int *out){static int value;(void)p;(void)n;(void)a;(void)out;return &value;}
#else
static int call(const unsigned char *p,size_t n,int a,int *out){(void)p;(void)n;*out=a;return DS_OK;}
#endif
DSThunk callback=call;
