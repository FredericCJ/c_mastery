#include "rwphase.h"
/* Required learner implementation; no phase, queue or upgrade solution supplied. */
int cpf_init(struct cmx_cpf *o,cmx_kernel *k){(void)o;(void)k;return CMX_UNIMPLEMENTED;}
int cpf_register(struct cmx_cpf *o,unsigned t,struct cpf_handle *h){(void)o;(void)t;(void)h;return CMX_UNIMPLEMENTED;}
int cpf_begin(struct cpf_handle h,unsigned m,cmx_deadline d,struct cpf_ticket *t){(void)h;(void)m;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cpf_upgrade(struct cpf_handle h,cmx_deadline d,struct cpf_ticket *t){(void)h;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cpf_result(struct cpf_ticket t){(void)t;return CMX_UNIMPLEMENTED;}
int cpf_cancel(struct cpf_ticket t){(void)t;return CMX_UNIMPLEMENTED;}
int cpf_unlock(struct cpf_handle h){(void)h;return CMX_UNIMPLEMENTED;}
int cpf_close(struct cmx_cpf *o){(void)o;return CMX_UNIMPLEMENTED;}
int cpf_release(struct cpf_handle *h){(void)h;return CMX_UNIMPLEMENTED;}
int cpf_destroy(struct cmx_cpf *o){(void)o;return CMX_UNIMPLEMENTED;}
