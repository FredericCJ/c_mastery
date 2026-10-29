#include "alternatives.h"
int pa_address(struct pa_empty a,unsigned b,struct pa_address*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_length(struct pa_address a,unsigned b,struct pa_length*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_timeout(struct pa_length a,unsigned b,struct pa_timeout*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_finish(struct pa_timeout a,struct pt_config*b){(void)a;(void)b;return PT_UNIMPLEMENTED;}
int pa_thread_sum(struct pt_thread*a,unsigned b,uint32_t c,uint32_t*d){(void)a;(void)b;(void)c;(void)d;return PT_UNIMPLEMENTED;}
int pa_session_pair(unsigned a,unsigned*b){(void)a;(void)b;return PT_UNIMPLEMENTED;}
