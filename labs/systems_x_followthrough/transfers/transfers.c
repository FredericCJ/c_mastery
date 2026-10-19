#include "transfers.h"
/* Intentionally unimplemented: explicit completed baseline providers required. */
int sx_remap(SxMap*a,unsigned b,unsigned c,unsigned d,unsigned e,unsigned char f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return B5_UNIMPLEMENTED;}
int sx_copy(SxMap*a,size_t b,const unsigned char*c,size_t d,SxBetween e,void*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return B5_UNIMPLEMENTED;}
int sx_partial(SxMap*a,size_t b,const unsigned char*c,size_t d,SxCopyResult*e){(void)a;(void)b;(void)c;(void)d;(void)e;return B5_UNIMPLEMENTED;}
int sx_allocate(SxLedger*a,uint32_t b,size_t c,unsigned d,unsigned e,SxAlias*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return B4_UNIMPLEMENTED;}
int sx_resize(SxLedger*a,SxAlias b,unsigned c,size_t d,int e,SxAlias*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return B4_UNIMPLEMENTED;}
int sx_transfer(SxLedger*a,SxAlias b,unsigned c,unsigned d){(void)a;(void)b;(void)c;(void)d;return B4_UNIMPLEMENTED;}
int sx_release(SxLedger*a,SxAlias b,unsigned c,unsigned d){(void)a;(void)b;(void)c;(void)d;return B4_UNIMPLEMENTED;}
int sx_lookup(const SxLedger*a,SxAlias b,unsigned c,size_t*d){(void)a;(void)b;(void)c;(void)d;return B4_UNIMPLEMENTED;}
int sx_online(SxHotplug*a,unsigned b){(void)a;(void)b;return S_UNIMPLEMENTED;}
int sx_begin(SxHotplug*a,unsigned b){(void)a;(void)b;return S_UNIMPLEMENTED;}
int sx_offline_request(SxHotplug*a,unsigned b,uint32_t c){(void)a;(void)b;(void)c;return S_UNIMPLEMENTED;}
int sx_offline_quiescent(SxHotplug*a,unsigned b,uint32_t c){(void)a;(void)b;(void)c;return S_UNIMPLEMENTED;}
int sx_ack(SxHotplug*a,unsigned b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return S_UNIMPLEMENTED;}
int sx_reclaim(SxHotplug*a){(void)a;return S_UNIMPLEMENTED;}
