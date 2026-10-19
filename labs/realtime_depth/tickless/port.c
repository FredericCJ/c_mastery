#include "port.h"
enum tq_status tp_init(TPClock *a,TQTimer *b,uint32_t c){(void)a;(void)b;(void)c;return TQ_UNIMPLEMENTED;}
uint32_t tp_raw(void){return 0;}
uint32_t tp_now(const TPClock *a){(void)a;return 0;}
enum tq_status tp_wait(TPClock *a,unsigned b,uint32_t c,unsigned d){(void)a;(void)b;(void)c;(void)d;return TQ_UNIMPLEMENTED;}
enum tq_status tp_at(TPClock *a,unsigned b,uint32_t c){(void)a;(void)b;(void)c;return TQ_UNIMPLEMENTED;}
enum tq_status tp_arm(TPClock *a,uint32_t b){(void)a;(void)b;return TQ_UNIMPLEMENTED;}
enum tq_status tp_interrupt(TPClock *a){(void)a;return TQ_UNIMPLEMENTED;}
enum tq_status tp_stop(TPClock *a){(void)a;return TQ_UNIMPLEMENTED;}
enum tq_status tp_resume(TPClock *a){(void)a;return TQ_UNIMPLEMENTED;}
void tp_shutdown(TPClock *a){(void)a;}
