#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int md_write(unsigned char *a,size_t cap,size_t off,const unsigned char *p,size_t n){size_t i;if(cap>31||off>cap||(n>cap-off&&B06_MUTATION!=1))return 0;if(off>32||n>32-off)return 0;for(i=0;i<n;++i)a[off+i]=p[i];return 1;}
int md_extent(uint32_t c,uint32_t s,uint32_t lim,uint32_t *o){uint64_t n=(uint64_t)c*s;if(B06_MUTATION==2)n=(uint16_t)n;if(n>lim||n>UINT32_MAX)return 0;*o=(uint32_t)n;return 1;}
int md_read(const struct md_arena *a,uint32_t g,unsigned i,unsigned char *o){if(a->live!=1||(a->generation!=g&&B06_MUTATION!=3)||i>=16)return 0;*o=a->bytes[i];return 1;}
int md_authorize(const struct md_arena *a,unsigned who,uint32_t g,unsigned begin,unsigned n){return a->live==1&&(a->owner==who||B06_MUTATION==4)&&a->generation==g&&begin<=16&&n<=16-begin;}
int md_dispatch(unsigned op,unsigned grant,unsigned *s){if(op>1||grant>1)return 0;if(op==1){if(!grant&&B06_MUTATION!=5)return 0;*s=77;}return 1;}
int md_log_copy(const char *p,size_t n,char *o,size_t cap){size_t i;if(n>=cap)return 0;for(i=0;i<n;++i)o[i]=(B06_MUTATION==6&&p[i]=='%')?'?':p[i];o[n]=0;return 1;}
int md_release(struct md_owner *o){if((o->owned!=1&&B06_MUTATION!=7)||o->releases==UINT_MAX)return 0;o->owned=0;++o->releases;return 1;}
int md_equal(const unsigned char *a,const unsigned char *b,size_t n,size_t *steps){size_t i;unsigned diff=0;*steps=0;for(i=0;i<n;++i){diff|=a[i]^b[i];++*steps;if(B06_MUTATION==8&&diff)return 0;}return diff==0;}
