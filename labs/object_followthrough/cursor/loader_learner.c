#include "loader.h"
int oc_open(OcModule *m,const char *p){(void)m;(void)p;return OC_LOAD_BAD;}
int oc_borrow(OcModule *m,const OcPlugin **p){(void)m;(void)p;return OC_LOAD_BAD;}
int oc_return(OcModule *m,const OcPlugin **p){(void)m;(void)p;return OC_LOAD_BAD;}
int oc_close(OcModule *m){(void)m;return OC_LOAD_BAD;}
int oc_function_lookup(OcModule *p,int *v){(void)p;(void)v;return OC_LOAD_BAD;}
