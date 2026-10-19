#include "physical.h"
int ep_master_poll_rx(EpMaster *s,uint32_t n,EduFrame *o){(void)s;(void)n;(void)o;return EP_STUB;}
int ep_slave_poll_rx(EpSlave *s,uint32_t n){(void)s;(void)n;return EP_STUB;}
int ep_slave_receive(EpSlave *s,unsigned t,uint8_t v,uint32_t n){(void)s;(void)t;(void)v;(void)n;return EP_STUB;}
int ep_reply(EpSlave *s,uint32_t t,const EduFrame *f,uint32_t n){(void)s;(void)t;(void)f;(void)n;return EP_STUB;}
int ep_open(EpTx *s,R5Io io,const PuClock *c,uint32_t b,const R5Timing *t,uint32_t now){(void)s;(void)io;(void)c;(void)b;(void)t;(void)now;return EP_STUB;}
int ep_fits(const EpTx *s,unsigned n,uint32_t b,unsigned *out){(void)s;(void)n;(void)b;(void)out;return EP_STUB;}
int ep_send(EpTx *s,uint32_t t,const uint8_t *p,size_t n,uint32_t now,uint32_t b){(void)s;(void)t;(void)p;(void)n;(void)now;(void)b;return EP_STUB;}
int ep_step(EpTx *s,uint32_t t,uint32_t now){(void)s;(void)t;(void)now;return EP_STUB;}
int ep_cancel(EpTx *s,uint32_t t,uint32_t now){(void)s;(void)t;(void)now;return EP_STUB;}
int ep_poll(EpMaster *s,uint32_t t,uint32_t now){(void)s;(void)t;(void)now;return EP_STUB;}
int ep_tick(EpMaster *s,uint32_t now){(void)s;(void)now;return EP_STUB;}
int ep_receive(EpMaster *s,unsigned st,uint8_t v,uint32_t now,EduFrame *o){(void)s;(void)st;(void)v;(void)now;(void)o;return EP_STUB;}
int ep_disconnect(EpMaster *s,uint32_t now){(void)s;(void)now;return EP_STUB;}
