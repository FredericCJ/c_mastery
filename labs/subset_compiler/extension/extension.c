#include "extension.h"
/* Learner implementation: rejection stubs compile; they cannot pass semantic gates. */
int ce_compile(const char*s,size_t n,CEProgram*p,CEDiagnostic*d){(void)s;(void)n;(void)p;(void)d;return 0;}
int ce_verify(const CEProgram*p,unsigned h[CE_CODE]){(void)p;(void)h;return 0;}
int ce_run(const CEProgram*p,uint32_t x,unsigned*f,uint32_t*out){(void)p;(void)x;(void)f;(void)out;return 0;}
int ce_emit(const CEProgram*p,unsigned t,char*out,size_t n){(void)p;(void)t;(void)out;(void)n;return 0;}
