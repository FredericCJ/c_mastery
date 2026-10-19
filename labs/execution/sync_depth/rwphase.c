#include "rwphase.h"
/* Required learner implementation; no phase, queue or upgrade solution supplied. */
int cpf_init(CPF *o,cmx_kernel *k){(void)o;(void)k;return CMX_UNIMPLEMENTED;}
int cpf_register(CPF *o,unsigned t,CPFHandle *h){(void)o;(void)t;(void)h;return CMX_UNIMPLEMENTED;}
int cpf_begin(CPFHandle h,unsigned m,cmx_deadline d,CPFTicket *t){(void)h;(void)m;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cpf_upgrade(CPFHandle h,cmx_deadline d,CPFTicket *t){(void)h;(void)d;(void)t;return CMX_UNIMPLEMENTED;}
int cpf_result(CPFTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cpf_cancel(CPFTicket t){(void)t;return CMX_UNIMPLEMENTED;}
int cpf_unlock(CPFHandle h){(void)h;return CMX_UNIMPLEMENTED;}
int cpf_close(CPF *o){(void)o;return CMX_UNIMPLEMENTED;}
int cpf_release(CPFHandle *h){(void)h;return CMX_UNIMPLEMENTED;}
int cpf_destroy(CPF *o){(void)o;return CMX_UNIMPLEMENTED;}
