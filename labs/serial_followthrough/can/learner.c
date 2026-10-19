#include "can.h"
int cn_feed(CnStream *a,const uint8_t *b,size_t c,size_t *d,CnDecoded *e){(void)a;(void)b;(void)c;(void)d;(void)e;return CN_UNIMPLEMENTED;}
int cn_search(uint32_t a,uint32_t b,unsigned c,unsigned d,uint32_t e,CnSearch *f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return CN_UNIMPLEMENTED;}
int cn_fault(CnFault *a,const CnEvent *b,CnEffect *c){(void)a;(void)b;(void)c;return CN_UNIMPLEMENTED;}
int cn_begin(CnController *a,CnIO b,const SDTiming *c,unsigned d,unsigned e,unsigned f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return CN_UNIMPLEMENTED;}
int cn_control(CnController *a){(void)a;return CN_UNIMPLEMENTED;}
int cn_recover(CnController *a,unsigned b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
int cn_send(CnController *a,const SDCan *b,uint32_t *c){(void)a;(void)b;(void)c;return CN_UNIMPLEMENTED;}
int cn_tx_poll(CnController *a,uint32_t b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
int cn_cancel(CnController *a,uint32_t b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
int cn_take(CnController *a,uint32_t b,unsigned *c){(void)a;(void)b;(void)c;return CN_UNIMPLEMENTED;}
int cn_receive(CnController *a,CnRx *b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
int cn_ack_overrun(CnController *a){(void)a;return CN_UNIMPLEMENTED;}
int cn_queue_put(CnQueue *a,const CnRx *b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
int cn_queue_get(CnQueue *a,CnRx *b){(void)a;(void)b;return CN_UNIMPLEMENTED;}
CnIO cn_f439_io(void){CnIO io={0};return io;}
