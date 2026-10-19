#include "medium.h"
void dm_init(DMedium *m){(void)m;}
int dm_arm(DMedium *m,unsigned c,unsigned o,unsigned l,uint64_t a,uint64_t b){(void)m;(void)c;(void)o;(void)l;(void)a;(void)b;return 1;}
void dm_reboot(DMedium *m){(void)m;}
int dm_read(void *m,uint32_t p,void *b){(void)m;(void)p;(void)b;return 1;}
int dm_write(void *m,uint32_t p,const void *b){(void)m;(void)p;(void)b;return 1;}
int dm_flush(void *m){(void)m;return 1;}
store_io dm_io(DMedium *m){return (store_io){m,dm_read,dm_write,dm_flush,DM_PAGES};}
