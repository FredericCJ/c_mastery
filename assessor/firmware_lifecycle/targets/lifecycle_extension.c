#include "lifecycle_extension.h"
#include <string.h>
#ifndef LCE_MUTANT
#define LCE_MUTANT 0
#endif
static uint32_t get(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void put(unsigned char *p,uint32_t x){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(x>>(8*i));}
static uint32_t crc(const unsigned char *p,size_t n){uint32_t c=UINT32_MAX;for(size_t i=0;i<n;i++){c^=p[i];for(unsigned b=0;b<8;b++)c=c&1?(c>>1)^UINT32_C(0xedb88320):c>>1;}return ~c;}
int lce_admit(const unsigned char *p,size_t n,const unsigned char *s,size_t z,unsigned trusted,unsigned revoked,uint32_t floor,lce_verify_fn verify,void *ctx,uint32_t *version){
 if(!p||n<6||!s||!z||!verify||!version)return 0;unsigned key=p[4];if(key<1||key>2||!(trusted&(1u<<key))||(LCE_MUTANT!=1&&(revoked&(1u<<key))))return 0;uint32_t v=get(p);if(v<floor)return 0;
 if(verify(ctx,key,p,n,s,z)!=1)return 0;*version=v;return 1;
}
int lce_record(unsigned char *p,uint32_t seq,uint32_t value){if(!p||seq==0)return 0;put(p,seq);put(p+4,value);put(p+8,~seq);put(p+12,~value);put(p+16,crc(p,16));p[20]=0xA5;p[21]=0x5A;p[22]=0x3C;p[23]=0xC3;return 1;}
int lce_valid(const unsigned char *p,uint32_t *seq,uint32_t *value){if(!p||!seq||!value||p[20]!=0xA5||(LCE_MUTANT!=2&&(p[21]!=0x5A||p[22]!=0x3C||p[23]!=0xC3)))return 0;uint32_t s=get(p),v=get(p+4);if(!s||get(p+8)!=~s||get(p+12)!=~v||get(p+16)!=crc(p,16))return 0;*seq=s;*value=v;return 1;}
int lce_choose(const unsigned char *a,const unsigned char *b,unsigned *o){uint32_t as=0,av=0,bs=0,bv=0;int x=lce_valid(a,&as,&av),y=lce_valid(b,&bs,&bv);if(!o||(!x&&!y)||(x&&y&&as==bs&&av!=bv))return 0;*o=!x?1:!y?0:bs>as;return 1;}
int lce_next(uint32_t c,uint32_t max,uint32_t *o){if(!o||!max||c>=max)return 0;*o=c+1;return 1;}
int lce_joint_reset(struct lce_joint *s){uint32_t n;if(!s||!lce_next(s->generation,s->limit,&n))return 0;s->generation=n;s->healthy=0;s->outstanding[0]=s->outstanding[1]=0;return 1;}
int lce_joint_ack(struct lce_joint *s,unsigned core,uint32_t generation,unsigned pending){if(!s||core>1||!s->generation||(LCE_MUTANT!=3&&generation!=s->generation))return 0;s->outstanding[core]=pending;s->healthy|=1u<<core;return 1;}
int lce_joint_confirm(const struct lce_joint *s){return s&&s->healthy==3&&!s->outstanding[0]&&!s->outstanding[1];}
int lce_fault_begin(struct lce_fault *s){if(!s)return 0;if(s->busy||s->valid){s->nested=1;return 0;}s->busy=1;return 1;}
int lce_fault_commit(struct lce_fault *s,uint32_t pc,uint32_t status){if(!s||!s->busy||s->valid)return 0;s->words[0]=pc;s->words[1]=status;s->words[2]=pc^status^UINT32_C(0x9e3779b9);s->words[3]=0xc0de;s->valid=1;s->busy=0;return 1;}
