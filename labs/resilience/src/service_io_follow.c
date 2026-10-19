#include "service_io.h"
int rzv_request_encode(RzjRequest r,unsigned char b[RZV_FRAME_BYTES]){(void)r;(void)b;return RZ_INVALID;}
void rzv_frame_init(RzvFrame *f){if(f)*f=(RzvFrame){0};}
int rzv_frame_feed(RzvFrame *f,const unsigned char *p,size_t n,size_t *c,RzjRequest *o){(void)f;(void)p;(void)n;(void)c;(void)o;return RZ_INVALID;}
int rzv_frame_end(RzvFrame *f){(void)f;return RZ_INVALID;}
int rzv_reply_decode(const unsigned char b[RZV_FRAME_BYTES],RzvReply *o){(void)b;(void)o;return RZ_INVALID;}
int rzv_reply_encode(RzvReply r,unsigned char b[RZV_FRAME_BYTES]){(void)r;(void)b;return RZ_INVALID;}
int rzv_now(uint64_t *o){(void)o;return RZ_INVALID;}
int rzv_init(RzvService *s,RzsOutbox *o,unsigned c,uint64_t n){(void)s;(void)o;(void)c;(void)n;return RZ_INVALID;}
int rzv_serve(RzvService *s,RzfScope *p,int f,int c,size_t n,RzvObservation *o){(void)s;(void)p;(void)f;(void)c;(void)n;(void)o;return RZ_INVALID;}
int rzv_close(RzvService *s){(void)s;return RZ_INVALID;}
int rzv_destroy(RzvService *s){(void)s;return RZ_INVALID;}
int rzv_notify(RzvService *s,unsigned p,unsigned n,RzsEvent *o){(void)s;(void)p;(void)n;(void)o;return RZ_INVALID;}
