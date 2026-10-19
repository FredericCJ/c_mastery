#include "channel.h"
int r5_delays(const R5Timing *a,R5Delays *b){(void)a;(void)b;return R5_UNIMPLEMENTED;}
int r5_load(unsigned a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_open(R5Channel *a,R5Io b,const PuClock *c,uint32_t d,const R5Timing *e,unsigned f,unsigned g,uint32_t h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return R5_UNIMPLEMENTED;}
int r5_send(R5Channel *a,uint32_t b,const uint8_t *c,size_t d,unsigned e,uint32_t f,uint32_t g,size_t *h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return R5_UNIMPLEMENTED;}
int r5_step(R5Channel *a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_cancel(R5Channel *a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_receive(R5Channel *a,unsigned b,uint8_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return R5_UNIMPLEMENTED;}
int r5_receive_poll(R5Channel *a){(void)a;return R5_UNIMPLEMENTED;}
int r5_take(R5Channel *a,R5Event *b,size_t *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_loss(R5Channel *a){(void)a;return R5_UNIMPLEMENTED;}
int r5_link(R5Link *a,const R5Event *b,EduFrame *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
