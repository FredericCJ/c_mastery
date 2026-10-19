#include "pipeline.h"
int ew_init(EwNode *a,unsigned b,unsigned c,uint16_t d,const R5Io *e,const PuClock *f,uint32_t g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return EW_STUB;}
int ew_queue(EwNode *a,unsigned b,const EduFrame *c){(void)a;(void)b;(void)c;return EW_STUB;}
int ew_tick(EwNode *a,uint32_t b){(void)a;(void)b;return EW_STUB;}
int ew_take(EwNode *a,EsEvent *b){(void)a;(void)b;return EW_STUB;}
int ew_pause(EwNode *a){(void)a;return EW_STUB;}
int ew_drained(const EwNode *a){(void)a;return 0;}
int ew_rebind(EwNode *a,uint16_t b){(void)a;(void)b;return EW_STUB;}
int ew_run(EwNode *a){(void)a;return EW_STUB;}
int ew_physical_verdict(unsigned a,unsigned b){(void)a;(void)b;return EW_STUB;}
