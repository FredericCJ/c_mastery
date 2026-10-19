#include "security_extension.h"
int mde_deliver(MDEObject *o,MDEEvent e){(void)o;(void)e;return 0;}
int mde_retire(MDEObject *o){(void)o;return 0;}
int mde_recreate(MDEObject *o,unsigned a,unsigned b){(void)o;(void)a;(void)b;return 0;}
int mde_log(const unsigned char *p,size_t n,char *o,size_t c){(void)p;(void)n;(void)o;(void)c;return 0;}
int mde_destroy(MDEOwner *o){(void)o;return 0;}
