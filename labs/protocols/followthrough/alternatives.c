#include "alternatives.h"
int pa_address(PAEmpty a,unsigned b,PAAddress*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_length(PAAddress a,unsigned b,PALength*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_timeout(PALength a,unsigned b,PATimeout*c){(void)a;(void)b;(void)c;return PT_UNIMPLEMENTED;}
int pa_finish(PATimeout a,PTConfig*b){(void)a;(void)b;return PT_UNIMPLEMENTED;}
int pa_thread_sum(PTThread*a,unsigned b,uint32_t c,uint32_t*d){(void)a;(void)b;(void)c;(void)d;return PT_UNIMPLEMENTED;}
int pa_session_pair(unsigned a,unsigned*b){(void)a;(void)b;return PT_UNIMPLEMENTED;}
