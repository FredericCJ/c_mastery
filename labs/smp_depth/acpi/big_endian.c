#include "root_checks.h"
#include "oracle_cases.h"
#include "capture_cases.h"
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ || _CALL_ELF != 2
#error The transfer requires an actual big-endian ELFv2 translation.
#endif
/* Explicit course providers for compiler-lowered aggregate copies. */
void *memcpy(void *d,const void *s,size_t n){volatile unsigned char *out=d;const volatile unsigned char *in=s;for(size_t i=0;i<n;++i)out[i]=in[i];return d;}
void *memset(void *d,int v,size_t n){volatile unsigned char *out=d;for(size_t i=0;i<n;++i)out[i]=(unsigned char)v;return d;}
extern long sa_raw_write(const char *,size_t);
int main(void){
 int result=sa_checks();if(!result)result=sa_root_checks();
 if(!result)result=sa_oracle_cases();
 if(!result)result=sa_capture_cases();
 const char pass[]="SA_BE_PASS:checks-and-independent-wire-oracle\n",fail[]="SA_BE_CHECK:semantic-mismatch\n";
 const char *text=result?fail:pass;size_t n=result?sizeof fail-1:sizeof pass-1;
 if(sa_raw_write(text,n)!=(long)n)return 2;
 return result?1:0;
}
