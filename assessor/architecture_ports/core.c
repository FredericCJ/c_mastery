#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int po_stack(uint64_t top,uint64_t bytes,uint64_t *out){if(bytes>top)return 0;*out=(top-bytes)&~(uint64_t)(B06_MUTATION==1?7:15);return 1;}
int po_saved(unsigned arch,unsigned reg){if(reg>31||arch>1)return 0;if(arch==0)return reg==2||reg==8||reg==9||(reg>=18&&reg<=27);return (reg>=19&&reg<=29)||(reg==31&&B06_MUTATION!=2);}
int po_arg(unsigned i,unsigned *r,unsigned *o){if(i>=16)return 0;if(i<8){*r=i;*o=0;}else{*r=UINT_MAX;*o=(i-(B06_MUTATION==3?7:8))*8;}return 1;}
int po_save(POContext *d,unsigned a,const uint64_t *r){unsigned i;if(a>1)return 0;for(i=0;i<32;++i)d->saved[i]=po_saved(a,i)?r[i]:0;d->arch=a;return 1;}
int po_restore(const POContext *s,unsigned a,uint64_t *r){unsigned i;if(a>1||(s->arch!=a&&B06_MUTATION!=4))return 0;for(i=0;i<32;++i)if(po_saved(a,i))r[i]=s->saved[i];return 1;}
int po_sign12(uint32_t r){unsigned x=r&4095u;return x>=2048?(int)x-4096:(int)x;}
int po_branch(int64_t d,unsigned bits,unsigned a){int64_t limit;if(bits<2||bits>30||(a!=2&&a!=4))return 0;limit=(int64_t)1<<(bits-1);return d>=-limit&&d<=limit-a&&(d%(int64_t)a==0||B06_MUTATION==5);}
int po_trap(POStatus *s,unsigned from){if(from>1||s->sie>1||s->spie>1||s->spp>1)return 0;s->spp=from;s->spie=s->sie;s->sie=0;return 1;}
int po_return(POStatus *s){if(s->sie>1||s->spie>1||s->spp>1)return 0;s->sie=s->spie;s->spie=1;if(B06_MUTATION!=6)s->spp=0;return 1;}
int po_access(unsigned u,unsigned s,unsigned sum,unsigned mxr,unsigned r,unsigned w,unsigned x,unsigned op){if(u>1||s>1||sum>1||mxr>1||r>1||w>1||x>1||op>2)return 0;if(!s&&!u)return 0;if(s&&u&&((op==2&&B06_MUTATION!=7)||!sum))return 0;return op==0?(r||(mxr&&x)):op==1?w:x;}
int po_closure(unsigned r,unsigned p){if((r|p)>15)return 0;return (r&~p)==0 || (B06_MUTATION==8&&r==3);}
