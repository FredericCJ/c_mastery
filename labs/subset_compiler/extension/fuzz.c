#include "extension.h"
#include <stdint.h>
#include <stddef.h>
int LLVMFuzzerTestOneInput(const uint8_t*d,size_t n){struct ce_program p;struct ce_diagnostic e;uint32_t out;unsigned fuel=1000,h[CE_CODE];unsigned char b[4252];size_t z;if(n>4252)return 0;if(ce_compile((const char*)d,n,&p,&e)||ce_unpack(d,n,&p)){if(!ce_verify(&p,h))__builtin_trap();if(!ce_pack(&p,b,sizeof b,&z))__builtin_trap();(void)ce_run(&p,17,&fuel,&out);}return 0;}
