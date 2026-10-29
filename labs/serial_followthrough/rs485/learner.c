#include "channel.h"
int r5_delays(const struct r5_timing *a,struct r5_delays *b){(void)a;(void)b;return R5_UNIMPLEMENTED;}
int r5_load(unsigned a,unsigned b,unsigned *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_open(struct r5_channel *a,struct r5_io b,const struct pu_clock *c,uint32_t d,const struct r5_timing *e,unsigned f,unsigned g,uint32_t h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return R5_UNIMPLEMENTED;}
int r5_send(struct r5_channel *a,uint32_t b,const uint8_t *c,size_t d,unsigned e,uint32_t f,uint32_t g,size_t *h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return R5_UNIMPLEMENTED;}
int r5_step(struct r5_channel *a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_cancel(struct r5_channel *a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_receive(struct r5_channel *a,unsigned b,uint8_t c,size_t *d){(void)a;(void)b;(void)c;(void)d;return R5_UNIMPLEMENTED;}
int r5_receive_poll(struct r5_channel *a){(void)a;return R5_UNIMPLEMENTED;}
int r5_take(struct r5_channel *a,struct r5_event *b,size_t *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
int r5_loss(struct r5_channel *a){(void)a;return R5_UNIMPLEMENTED;}
int r5_link(struct r5_link *a,const struct r5_event *b,struct edu_frame *c){(void)a;(void)b;(void)c;return R5_UNIMPLEMENTED;}
