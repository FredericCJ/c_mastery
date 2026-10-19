#include "facilities.h"
#ifndef LF_MUTANT
#define LF_MUTANT 0
#endif
#ifdef LF_ENHANCED
#include <stdckdint.h>
#include <stdbit.h>
#endif
LfStatus lf_add(int32_t a,int32_t b,int32_t *out){
 if(!out)return LF_INVALID;
#if LF_MUTANT==1
 *out=0;
#endif
#ifdef LF_ENHANCED
 int32_t v;if(ckd_add(&v,a,b))return LF_INVALID;
#else
 if((b>0 && a>INT32_MAX-b)||(b<0 && a<INT32_MIN-b))return LF_INVALID;
 int32_t v=a+b;
#endif
 *out=v;return LF_OK;
}
LfStatus lf_mixed(int32_t a,uint32_t b,uint16_t *out){
 if(!out)return LF_INVALID;
#ifdef LF_ENHANCED
 uint16_t v;if(ckd_add(&v,a,b))return LF_INVALID;*out=v;
#else
 uint32_t v;
 if(a<0){uint32_t magnitude=(uint32_t)(-(a+1))+1u;if(b<magnitude)return LF_INVALID;v=b-magnitude;}
 else {if((uint32_t)a>UINT16_MAX || b>UINT16_MAX-(uint32_t)a)return LF_INVALID;v=(uint32_t)a+b;}
#if LF_MUTANT!=2
 if(v>UINT16_MAX)return LF_INVALID;
#endif
 *out=(uint16_t)v;
#endif
 return LF_OK;
}
LfStatus lf_bits(uint32_t v,LfBits *out){
 if(!out)return LF_INVALID;
#ifdef LF_ENHANCED
 LfBits r={(unsigned)stdc_leading_zeros(v),(unsigned)stdc_trailing_zeros(v),(unsigned)stdc_bit_width(v)};
#else
 LfBits r={32,32,0};uint32_t t=v;while(t){r.width++;t>>=1;}r.leading=32-r.width;
 if(v){r.trailing=0;t=v;while((t&1u)==0){r.trailing++;t>>=1;}}
#endif
#if LF_MUTANT==3
 if(v==0)r.trailing=0;
#endif
 *out=r;return LF_OK;
}
LfStatus lf_div(uint64_t a,uint64_t b,uint64_t *out){
 if(!out)return LF_INVALID;
#if LF_MUTANT==4
 *out=0;
#endif
 if(!b)return LF_INVALID;
#ifdef LF_ENHANCED
 *out=a/b;
#else
 uint64_t q=0,r=0;
 for(unsigned i=64;i>0;i--){unsigned carry=(unsigned)(r>>63);r=(r<<1)|((a>>(i-1u))&1u);q<<=1;if(carry || r>=b){r-=b;q|=1u;}}
 *out=q;
#endif
 return LF_OK;
}
LfStatus lf_select(unsigned observed,unsigned strict,unsigned *out){
 if(!out || observed>31 || strict>1)return LF_INVALID;
#if LF_MUTANT==5
 *out=(observed&1u)!=0;
#else
 unsigned required=strict?31u:15u;
 *out=(observed&required)==required;
#endif
 return LF_OK;
}
