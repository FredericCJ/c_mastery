#include "transfer.h"
int it_progress(const struct iovec*v,unsigned n,size_t*p,ITWrite f,void*c){(void)v;(void)n;(void)p;(void)f;(void)c;return -1;}
int it_queue_init(ITQueue*q){(void)q;return -1;}
int it_push(ITQueue*q,uint64_t v){(void)q;(void)v;return 0;}
int it_pop(ITQueue*q,uint64_t*v){(void)q;(void)v;return 0;}
void it_queue_close(ITQueue*q){(void)q;}
void it_queue_destroy(ITQueue*q){(void)q;}
int it_time(ITLoop*q,uint64_t n,int p){(void)q;(void)n;(void)p;return -1;}
int it_choose(ITLoop*q){(void)q;return -1;}
void it_drained(ITLoop*q,unsigned p){(void)q;(void)p;}
int it_direct_ok(uintptr_t p,size_t n,uint64_t o,unsigned m,unsigned a){(void)p;(void)n;(void)o;(void)m;(void)a;return 0;}
