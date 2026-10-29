#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int lc_program(unsigned char *d,const unsigned char *s,size_t n){size_t i;if(B06_MUTATION!=1)for(i=0;i<n;++i)if((d[i]&s[i])!=s[i])return 0;for(i=0;i<n;++i)d[i]&=s[i];return 1;}
uint32_t lc_crc(const unsigned char *p,size_t n){uint32_t c=UINT32_MAX;size_t i;unsigned b;for(i=0;i<n;++i){c^=p[i];for(b=0;b<8;++b)c=(c>>1)^((c&1)?UINT32_C(0xedb88320):0);}return B06_MUTATION==2?c:~c;}
int lc_admit(uint32_t n,uint32_t cap,uint32_t v,uint32_t floor,unsigned sig){return n>0&&n<=cap&&v>=floor&&(sig==1||B06_MUTATION==3);}
int lc_select(struct lc_boot *s,unsigned *slot){if(s->active>1||s->trial>1||s->confirmed>1||s->attempted>1)return 0;if(s->confirmed)*slot=s->active;else if(!s->attempted){*slot=s->trial;s->attempted=1;}else *slot=B06_MUTATION==4?s->trial:s->active;return 1;}
int lc_journal_select(const struct lc_journal *a,const struct lc_journal *b,unsigned *idx){int av=a->commit==0xa5&&a->check==(a->seq^a->value^UINT32_C(0x5a5a5a5a));int bv=(b->commit==0xa5||B06_MUTATION==5)&&b->check==(b->seq^b->value^UINT32_C(0x5a5a5a5a));if(!av&&!bv)return 0;if(av&&bv&&a->seq==b->seq&&a->value!=b->value)return 0;*idx=!av?1:!bv?0:b->seq>a->seq;return 1;}
int lc_raise_floor(uint32_t c,uint32_t n,unsigned ok,uint32_t *o){if(ok!=1||(n<c&&B06_MUTATION!=6))return 0;*o=n;return 1;}
int lc_peer_ready(uint32_t local,uint32_t ack,unsigned outstanding){return (local==ack||B06_MUTATION==7)&&outstanding==0;}
int lc_capture_valid(const uint32_t *p,size_t n){return n==4&&p[3]==0xc0de&&(p[2]==(p[0]^p[1]^UINT32_C(0x9e3779b9))||B06_MUTATION==8);}
