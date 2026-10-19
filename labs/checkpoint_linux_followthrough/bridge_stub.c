#include "bridge.h"
/* Intentionally unfinished learner adapter. */
int p5_bridge_init(P5Bridge *p,int fd,int h){(void)p;(void)fd;(void)h;return -1;}
int p5_bridge_offer(P5Bridge *p,int32_t v){(void)p;(void)v;return -1;}
int p5_bridge_begin(P5Bridge *p,uint32_t *t){(void)p;(void)t;return -1;}
int p5_bridge_cancel(P5Bridge *p,uint32_t t){(void)p;(void)t;return -1;}
int p5_bridge_observe(P5Bridge *p,uint32_t t){(void)p;(void)t;return -1;}
int p5_bridge_wait(P5Bridge *p,const struct timespec *t){(void)p;(void)t;return -1;}
int p5_bridge_release_gate(P5Bridge *p){(void)p;return -1;}
int p5_bridge_reap(P5Bridge *p){(void)p;return -1;}
int p5_bridge_destroy(P5Bridge *p){(void)p;return -1;}
