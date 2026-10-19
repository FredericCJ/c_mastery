/* Host-side audit of the explicit original target byte-helper provider. */
#include <stddef.h>
#include <stdio.h>
void *memcpy(void *,const void *,size_t);
void *memset(void *,int,size_t);
void __aeabi_memclr4(void *,size_t);
void __aeabi_memcpy4(void *,const void *,size_t);
int main(void)
{
    unsigned char source[64],out[64];unsigned cases=0;
    for(unsigned i=0;i<64;i++)source[i]=(unsigned char)(i*3+1);
    for(unsigned n=0;n<=64;n++)for(unsigned value=0;value<256;value+=17){
        for(unsigned i=0;i<64;i++)out[i]=0xa5;
        if(memset(out,(int)value,n)!=out)return 1;
        for(unsigned i=0;i<64;i++)if(out[i]!=(i<n?(unsigned char)value:0xa5))return 1;
        if(memcpy(out,source,n)!=out)return 1;
        for(unsigned i=0;i<64;i++)if(out[i]!=(i<n?source[i]:0xa5))return 1;
        __aeabi_memclr4(out,n);
        for(unsigned i=0;i<64;i++)if(out[i]!=(i<n?0:0xa5))return 1;
        __aeabi_memcpy4(out,source,n);
        for(unsigned i=0;i<64;i++)if(out[i]!=(i<n?source[i]:0xa5))return 1;
        cases++;
    }
    printf("MM_HELPERS:%u passed\n",cases);return 0;
}
