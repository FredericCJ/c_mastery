#include "extension.h"
#include <string.h>
int LLVMFuzzerTestOneInput(const uint8_t*d,size_t n){
 struct ce_program p,q,before;unsigned h[CE_CODE],fuel=1024;uint32_t out=91;unsigned char bytes[4252];size_t written=0;
 if(n>4252)return 0;memset(&p,0xa5,sizeof p);before=p;
 if(!ce_unpack(d,n,&p)){if(memcmp(&p,&before,sizeof p))__builtin_trap();return 0;}
 if(!ce_verify(&p,h)||!ce_pack(&p,bytes,sizeof bytes,&written)||written!=n||memcmp(bytes,d,n)||!ce_unpack(bytes,written,&q))__builtin_trap();
 if(!ce_run(&p,17,&fuel,&out)&&out!=91)__builtin_trap();return 0;
}
