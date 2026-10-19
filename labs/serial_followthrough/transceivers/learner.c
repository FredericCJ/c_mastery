#include "transceivers.h"
int tr_sample(const TrSample *s,int *d){(void)s;(void)d;return TR_UNIMPLEMENTED;}
int tr_connections(const TrConnector *a,const TrConnector *b,const TrWire *w,size_t n){(void)a;(void)b;(void)w;(void)n;return TR_UNIMPLEMENTED;}
int tr_topology(unsigned a,unsigned b,unsigned c,unsigned d){(void)a;(void)b;(void)c;(void)d;return TR_UNIMPLEMENTED;}
int tr_232_circuit(const Tr232Circuit *c){(void)c;return TR_UNIMPLEMENTED;}
int tr_capture(unsigned d,const TrPoint *p,size_t n,unsigned s,TrResult *r){(void)d;(void)p;(void)n;(void)s;(void)r;return TR_UNIMPLEMENTED;}
int tr_edge(const TrPoint *p,size_t n,unsigned e,unsigned s,TrEdge *r){(void)p;(void)n;(void)e;(void)s;(void)r;return TR_UNIMPLEMENTED;}
int tr_grant(TrBus *b,unsigned p,uint32_t s,uint64_t n){(void)b;(void)p;(void)s;(void)n;return TR_UNIMPLEMENTED;}
int tr_release(TrBus *b,unsigned p,uint32_t s,uint64_t n,unsigned t,uint64_t d,uint64_t g){(void)b;(void)p;(void)s;(void)n;(void)t;(void)d;(void)g;return TR_UNIMPLEMENTED;}
int tr_ready(TrBus *b,uint64_t n){(void)b;(void)n;return TR_UNIMPLEMENTED;}
