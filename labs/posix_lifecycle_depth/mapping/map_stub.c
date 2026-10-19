#include "map.h"
#include <errno.h>
int pm_map(int f,off_t o,size_t n,size_t l,PMMap *m){(void)f;(void)o;(void)n;(void)l;(void)m;return ENOSYS;}
int pm_remap(PMMap *m,int f,size_t n,size_t l){(void)m;(void)f;(void)n;(void)l;return ENOSYS;}
int pm_copy(const PMMap *m,PMBorrow b,void *p,size_t n){(void)m;(void)b;(void)p;(void)n;return ENOSYS;}
int pm_unmap(PMMap *m){(void)m;return ENOSYS;}
int pm_encode(unsigned char b[128],const PMRecord *r,unsigned e){(void)b;(void)r;(void)e;return ENOSYS;}
int pm_decode(const unsigned char *b,size_t n,PMRecord *r){(void)b;(void)n;(void)r;return ENOSYS;}
int pm_publish(int f,uint64_t s,const void *p,size_t n,PMCut c,void *a){(void)f;(void)s;(void)p;(void)n;(void)c;(void)a;return ENOSYS;}
int pm_read_record(int f,uint64_t s,PMRecord *r){(void)f;(void)s;(void)r;return ENOSYS;}
int pm_replace(int f,uint64_t a,uint64_t b,const void *p,size_t n){(void)f;(void)a;(void)b;(void)p;(void)n;return ENOSYS;}
