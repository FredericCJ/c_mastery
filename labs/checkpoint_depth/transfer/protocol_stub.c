#include "protocol.h"
CPXProtocolStatus cpx_protocol_init(CPXParser *p,size_t n){(void)p;(void)n;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_feed(CPXParser *p,const unsigned char *b,size_t n,size_t *u){(void)p;(void)b;(void)n;(void)u;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_queue(CPXQueue *q,size_t n){(void)q;(void)n;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_deliver(CPXParser *p,CPXQueue *q){(void)p;(void)q;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_take(CPXQueue *q,CPXMessage *m){(void)q;(void)m;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_encode(const CPXMessage *m,unsigned char *b,size_t n,size_t *w){(void)m;(void)b;(void)n;(void)w;return CPX_P_NOT_IMPLEMENTED;}
CPXProtocolStatus cpx_protocol_inplace(unsigned char *b,size_t c,size_t n,unsigned v,unsigned f,uint16_t s,uint32_t t,size_t *w){(void)b;(void)c;(void)n;(void)v;(void)f;(void)s;(void)t;(void)w;return CPX_P_NOT_IMPLEMENTED;}
