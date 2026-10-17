#include "protocols.h"
/* Implement the contracts; these buildable stubs deliberately fail semantic checks. */
void pt_device_init(PTDevice *d,PTClosed *out){(void)d;(void)out;}
int pt_configure(PTClosed *old,unsigned fail,PTReady *out){(void)old;(void)fail;(void)out;return PT_UNIMPLEMENTED;}
int pt_recover(PTDevice *d,unsigned quiet,PTClosed *out){(void)d;(void)quiet;(void)out;return PT_UNIMPLEMENTED;}
int pt_close(PTReady *old,PTClosed *out){(void)old;(void)out;return PT_UNIMPLEMENTED;}
int pt_builder_set(PTBuilder *b,unsigned field,unsigned value){(void)b;(void)field;(void)value;return PT_UNIMPLEMENTED;}
int pt_builder_finish(const PTBuilder *b,PTConfig *out){(void)b;(void)out;return PT_UNIMPLEMENTED;}
int pt_session_step(PTSession *s,unsigned sending,unsigned tag){(void)s;(void)sending;(void)tag;return PT_UNIMPLEMENTED;}
int pt_dispatch(PTMachine *m,unsigned event,unsigned char *trace,size_t capacity,size_t *used){(void)m;(void)event;(void)trace;(void)capacity;(void)used;return PT_UNIMPLEMENTED;}
int pt_post(PTActive *a,unsigned event){(void)a;(void)event;return PT_UNIMPLEMENTED;}
int pt_active_step(PTActive *a,unsigned char *trace,size_t capacity,size_t *used){(void)a;(void)trace;(void)capacity;(void)used;return PT_UNIMPLEMENTED;}
int pt_thread_sum(PTThread *p,unsigned available,uint32_t value,uint32_t *out){(void)p;(void)available;(void)value;(void)out;return PT_UNIMPLEMENTED;}
int pt_behavior_tick(PTBehavior *b,const PTNode *p,size_t n,PTTick tick,void *ctx,unsigned *out){(void)b;(void)p;(void)n;(void)tick;(void)ctx;(void)out;return PT_UNIMPLEMENTED;}
int pt_behavior_cancel(PTBehavior *b,PTAbort abort,void *ctx){(void)b;(void)abort;(void)ctx;return PT_UNIMPLEMENTED;}
int pt_driver_start(PTDriver *d,const unsigned char *p,size_t n){(void)d;(void)p;(void)n;return PT_UNIMPLEMENTED;}
int pt_driver_complete(PTDriver *d,unsigned error,unsigned quiet){(void)d;(void)error;(void)quiet;return PT_UNIMPLEMENTED;}
int pt_driver_recover(PTDriver *d){(void)d;return PT_UNIMPLEMENTED;}
