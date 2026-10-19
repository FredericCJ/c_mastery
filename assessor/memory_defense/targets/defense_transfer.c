#include "defense_transfer.h"
#ifndef DT_MUTANT
#define DT_MUTANT 0
#endif
int dt_extent(uint64_t count,uint64_t width,uint64_t header,uint64_t align,uint64_t tail,uint64_t limit,uint64_t*out){if(!out||!width||!align||align>4096||(align&(align-1))||header>limit)return 0;uint64_t pad=(0-header)&(align-1);if(pad>limit-header)return 0;uint64_t prefix=header+pad;if(tail>limit-prefix)return 0;uint64_t available=limit-prefix-tail;if(count>available/width&&DT_MUTANT!=1)return 0;*out=prefix+count*width+tail;return 1;}
int dt_read(const DTOwner*owner,DTView view,size_t i,unsigned char*out){if(!owner||!out||!owner->p||view.tag!=owner->tag||view.n>owner->n||i>=view.n||(DT_MUTANT!=2&&view.p!=owner->p))return 0;*out=view.p[i];return 1;}
/* Public lengths; full equal-length byte scan. No ISO C wall-clock guarantee. */
int dt_equal(const unsigned char*a,size_t na,const unsigned char*b,size_t nb,size_t*steps){if(!steps||na>256||nb>256||(na&&!a)||(nb&&!b))return -1;*steps=0;if(na!=nb)return 0;unsigned diff=0;for(size_t i=0;i<na;i++){diff|=(unsigned)(a[i]^b[i]);++*steps;if(DT_MUTANT==3&&diff)return 0;}return diff==0;}
