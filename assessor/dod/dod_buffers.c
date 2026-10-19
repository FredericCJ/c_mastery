#include "dod_buffers.h"
#ifndef D_BUFFER_MUTANT
#define D_BUFFER_MUTANT 0
#endif
int d_inline_append(DInline *s,const unsigned char *p,size_t n){
 if(!s||(!p&&n))return D_INVALID;
 if(n>16-s->length)return D_BUFFER_MUTANT==1?D_OK:D_FULL;
 for(size_t i=0;i<n;++i)s->bytes[s->length+i]=p[i];
 s->length+=n;return D_OK;
}
int d_inline_truncate(DInline *s,size_t n){if(!s)return D_INVALID;if(n>s->length)return D_RANGE;s->length=n;return D_OK;}
static unsigned char *take(DQuota *q,size_t n){
 if(!n||n>128||q->attempts==SIZE_MAX)return NULL;
 ++q->attempts;
 if(q->attempts==q->fail_at||n>128-q->live)return NULL;
 for(unsigned i=0;i<4;++i)if(!q->used[i]){q->used[i]=n;q->live+=n;if(q->live>q->peak)q->peak=q->live;return q->rows[i];}
 return NULL;
}
static void release(DQuota *q,unsigned char *p){for(unsigned i=0;i<4;++i)if(p==q->rows[i]&&q->used[i]){q->live-=q->used[i];q->used[i]=0;if(q->releases<SIZE_MAX)++q->releases;return;}}
void d_zero_init(DZero *s,DQuota *q){*s=(DZero){q,NULL,0};}
void d_zero_reset(DZero *s){if(s->data)release(s->owner,s->data);s->data=NULL;s->length=0;}
static int prepare(const DZero *s,const unsigned char *p,size_t n,DZero *out){
 if(!s||!s->owner||(!p&&n))return D_INVALID;
 if(n>128-s->length)return D_FULL;
 if(!n){*out=(DZero){s->owner,NULL,0};return D_OK;}
 unsigned char *data=take(s->owner,s->length+n);if(!data)return D_FAILED;
 for(size_t i=0;i<s->length;++i)data[i]=s->data[i];
 for(size_t i=0;i<n;++i)data[s->length+i]=p[i];
 *out=(DZero){s->owner,data,s->length+n};return D_OK;
}
static void publish(DZero *s,DZero fresh){d_zero_reset(s);*s=fresh;}
int d_zero_append(DZero *s,const unsigned char *p,size_t n){DZero fresh;int status=prepare(s,p,n,&fresh);if(status!=D_OK||!n)return status;publish(s,fresh);return D_OK;}
int d_zero_pair_append(DZero *a,const unsigned char *ap,size_t an,DZero *b,const unsigned char *bp,size_t bn){
 if(!a||!b||a==b||!a->owner||a->owner!=b->owner||(!ap&&an)||(!bp&&bn))return D_INVALID;
 if(an>128-a->length||bn>128-b->length)return D_FULL;
 DZero first,second;int status=prepare(a,ap,an,&first);if(status!=D_OK)return status;
 if(D_BUFFER_MUTANT==2&&an){publish(a,first);an=0;}
 status=prepare(b,bp,bn,&second);if(status!=D_OK){if(an)d_zero_reset(&first);return status;}
 if(an)publish(a,first);
 if(bn)publish(b,second);
 return D_OK;
}
int d_zero_move(DZero *to,DZero *from){if(!to||!from||to==from||to->data||to->length||!from->owner)return D_INVALID;*to=*from;from->data=NULL;from->length=0;return D_OK;}
uint32_t d_swar16_signed(uint32_t a,uint32_t b){
 uint32_t high=UINT32_C(0x80008000),low=UINT32_C(0x7fff7fff);
 uint32_t sum=((a&low)+(b&low))^((a^b)&high);
 if(D_BUFFER_MUTANT==4)sum=a+b;
 uint32_t overflow=(~(a^b))&(a^sum)&high;
 if(D_BUFFER_MUTANT==3)overflow=0;
 uint32_t mask=overflow|(overflow-(overflow>>15));
 uint32_t positive=(~a)&high;
 uint32_t saturation=(a&high)|(positive-(positive>>15));
 return (sum&~mask)|(saturation&mask);
}
