#include "encoding.h"
int pe_branch(int32_t d,unsigned a,unsigned b,uint32_t*out){if(!out||a>31||b>31||d<-4096||d>4094||(d&1))return 0;uint32_t u=(uint32_t)d;*out=((u>>12)&1u)<<31|((u>>5)&63u)<<25|b<<20|a<<15|((u>>1)&15u)<<8|((u>>11)&1u)<<7|0x63u;return 1;}
int pe_jump(int32_t d,unsigned rd,uint32_t*out){if(!out||rd>31||d<-1048576||d>1048574||(d&1))return 0;uint32_t u=(uint32_t)d;*out=((u>>20)&1u)<<31|((u>>1)&1023u)<<21|((u>>11)&1u)<<20|((u>>12)&255u)<<12|rd<<7|0x6fu;return 1;}
