#include "protocols.h"
/* Implement the contracts; these buildable stubs deliberately fail semantic checks. */
void pt_device_init(struct pt_device *d,struct pt_closed *out){(void)d;(void)out;}
int pt_configure(struct pt_closed *old,unsigned fail,struct pt_ready *out){(void)old;(void)fail;(void)out;return PT_UNIMPLEMENTED;}
int pt_recover(struct pt_device *d,unsigned quiet,struct pt_closed *out){(void)d;(void)quiet;(void)out;return PT_UNIMPLEMENTED;}
int pt_close(struct pt_ready *old,struct pt_closed *out){(void)old;(void)out;return PT_UNIMPLEMENTED;}
int pt_builder_set(struct pt_builder *b,unsigned field,unsigned value){(void)b;(void)field;(void)value;return PT_UNIMPLEMENTED;}
int pt_builder_finish(const struct pt_builder *b,struct pt_config *out){(void)b;(void)out;return PT_UNIMPLEMENTED;}
int pt_session_step(struct pt_session *s,unsigned sending,unsigned tag){(void)s;(void)sending;(void)tag;return PT_UNIMPLEMENTED;}
int pt_dispatch(struct pt_machine *m,unsigned event,unsigned char *trace,size_t capacity,size_t *used){(void)m;(void)event;(void)trace;(void)capacity;(void)used;return PT_UNIMPLEMENTED;}
int pt_post(struct pt_active *a,unsigned event){(void)a;(void)event;return PT_UNIMPLEMENTED;}
int pt_active_step(struct pt_active *a,unsigned char *trace,size_t capacity,size_t *used){(void)a;(void)trace;(void)capacity;(void)used;return PT_UNIMPLEMENTED;}
int pt_thread_sum(struct pt_thread *p,unsigned available,uint32_t value,uint32_t *out){(void)p;(void)available;(void)value;(void)out;return PT_UNIMPLEMENTED;}
int pt_behavior_tick(struct pt_behavior *b,const struct pt_node *p,size_t n,pt_tick_fn tick,void *ctx,unsigned *out){(void)b;(void)p;(void)n;(void)tick;(void)ctx;(void)out;return PT_UNIMPLEMENTED;}
int pt_behavior_cancel(struct pt_behavior *b,pt_abort_fn abort,void *ctx){(void)b;(void)abort;(void)ctx;return PT_UNIMPLEMENTED;}
int pt_driver_start(struct pt_driver *d,const unsigned char *p,size_t n){(void)d;(void)p;(void)n;return PT_UNIMPLEMENTED;}
int pt_driver_complete(struct pt_driver *d,unsigned error,unsigned quiet){(void)d;(void)error;(void)quiet;return PT_UNIMPLEMENTED;}
int pt_driver_recover(struct pt_driver *d){(void)d;return PT_UNIMPLEMENTED;}
