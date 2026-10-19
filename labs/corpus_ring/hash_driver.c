#include "ring.h"
#include <stdio.h>
int cr_sha_provider(void *,const unsigned char *,size_t,unsigned char [32]);
int main(void)
{
    CrToken t[16];for(unsigned i=0;i<16;i++)t[i]=(CrToken){(uint16_t)(4096*i),(uint8_t)(i+1),0};
    CrRing r;if(cr_build(t,16,1,&r)!=CR_OK)return 2;
    for(unsigned n=0;n<=512;n++){
        unsigned char bytes[512];for(unsigned i=0;i<n;i++)bytes[i]=(unsigned char)(i^n);
        unsigned out;CrCost c;if(cr_route(&r,1,bytes,n,cr_sha_provider,NULL,&out,&c)!=CR_OK)return 3;
        printf("%u %u\n",n,out);
    }
    return 0;
}
