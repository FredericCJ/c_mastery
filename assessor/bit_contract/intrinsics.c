#include "bits.h"
#include <limits.h>
#include <stdio.h>
#if !defined(__has_builtin)
#error builtin_capability_missing
#endif
#if !__has_builtin(__builtin_popcount) || !__has_builtin(__builtin_clz)
#error bit_builtin_capability_missing
#endif
_Static_assert(UINT_MAX==4294967295u,"recorded32bit unsigned int intrinsic carrier");
static FbStatus intrinsic(unsigned width,FbWord input,FbStats *out){
 if(!out || !width || width>16)return FB_INVALID;
 unsigned v=(unsigned)(input&(((FbWord)1<<width)-1));
 unsigned leading=v?(unsigned)__builtin_clz(v)-(32-width):width;
#ifdef FB_INTRINSIC_ZERO
 if(!v)leading=0; /* Defined wrong zero policy, never call undefined clz(0). */
#endif
 *out=(FbStats){(unsigned)__builtin_popcount(v),width-leading,leading};return FB_OK;
}
int main(void){
 unsigned long count=0;
 for(unsigned width=1;width<=16;++width)for(unsigned n=0;n<=65535;++n){
  FbStats a,b;if(fb_stats(width,n,&a)!=FB_OK||intrinsic(width,n,&b)!=FB_OK||a.population!=b.population||a.width!=b.width||a.leading!=b.leading){fprintf(stderr,"FB_INTRINSIC_FAIL width=%u value=%u\n",width,n);return 1;}++count;
 }
 printf("FB_INTRINSIC_PASS cases=%lu zero_guard logical_width\n",count);return 0;
}
