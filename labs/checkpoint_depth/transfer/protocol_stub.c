#include "protocol.h"
enum cpx_protocol_status cpx_protocol_init(struct cpx_parser *p,size_t n){(void)p;(void)n;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_feed(struct cpx_parser *p,const unsigned char *b,size_t n,size_t *u){(void)p;(void)b;(void)n;(void)u;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_queue(struct cpx_queue *q,size_t n){(void)q;(void)n;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_deliver(struct cpx_parser *p,struct cpx_queue *q){(void)p;(void)q;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_take(struct cpx_queue *q,struct cpx_message *m){(void)q;(void)m;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_encode(const struct cpx_message *m,unsigned char *b,size_t n,size_t *w){(void)m;(void)b;(void)n;(void)w;return CPX_P_NOT_IMPLEMENTED;}
enum cpx_protocol_status cpx_protocol_inplace(unsigned char *b,size_t c,size_t n,unsigned v,unsigned f,uint16_t s,uint32_t t,size_t *w){(void)b;(void)c;(void)n;(void)v;(void)f;(void)s;(void)t;(void)w;return CPX_P_NOT_IMPLEMENTED;}
