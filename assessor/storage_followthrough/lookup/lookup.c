#include "lookup.h"
#include <string.h>
#ifndef SL_MUTANT
#define SL_MUTANT 0
#endif
static int order(const uint8_t *a,size_t n,const uint8_t *b,size_t m){size_t z=n<m?n:m;for(size_t i=0;i<z;i++){if(a[i]<b[i])return -1;if(a[i]>b[i])return 1;}return n<m?-1:n>m?1:0;}
int sl_normalize(const uint8_t *p,size_t n,uint8_t out[16],size_t *length){
 if(!p||!out||!length||!n||n>16)return STORE_INVALID;
 uint8_t staged[16]={0};for(size_t i=0;i<n;i++){uint8_t c=p[i];if(c<0x20||c>0x7e)return STORE_INVALID;staged[i]=(uint8_t)(c>='A'&&c<='Z'&&SL_MUTANT!=1?c+('a'-'A'):c);if(SL_MUTANT==2)out[i]=staged[i];}memcpy(out,staged,16);*length=n;return STORE_OK;
}
typedef struct {int invalid;} Bind;
static int canonical(void *ctx,const uint8_t *key,size_t n,const uint8_t *value,size_t z){(void)value;(void)z;Bind *b=ctx;uint8_t normalized[16];size_t length;if(sl_normalize(key,n,normalized,&length)!=STORE_OK||memcmp(key,normalized,n)){b->invalid=1;return 1;}return 0;}
int sl_text_bind(store *s,SlText *out){if(!s||!out)return STORE_INVALID;int r=store_check(s);if(r)return r;Bind b={0};r=store_scan(s,NULL,0,canonical,&b);if(r)return r;if(b.invalid)return STORE_INVALID;out->engine=s;return STORE_OK;}
int sl_text_put(SlText *s,const void *p,size_t n,const void *v,size_t z){if(!s||!s->engine)return STORE_INVALID;uint8_t key[16];size_t length;int r=sl_normalize(p,n,key,&length);return r?r:store_put(s->engine,key,length,v,z);}
int sl_text_get(SlText *s,const void *p,size_t n,void *v,size_t z,size_t *l){if(!s||!s->engine)return STORE_INVALID;uint8_t key[16];size_t length;int r=sl_normalize(p,n,key,&length);return r?r:store_get(s->engine,key,length,v,z,l);}
int sl_text_scan(SlText *s,const void *p,size_t n,store_visit f,void *ctx){if(!s||!s->engine)return STORE_INVALID;uint8_t key[16];size_t length;if(n==0)return store_scan(s->engine,NULL,0,f,ctx);int r=sl_normalize(p,n,key,&length);return r?r:store_scan(s->engine,key,length,f,ctx);}
typedef struct {SlSnapshot *snapshot;int full;} Capture;
static int copy_record(void *ctx,const uint8_t *key,size_t n,const uint8_t *value,size_t z){Capture *c=ctx;SlSnapshot *s=c->snapshot;if(s->count==SL_RECORDS){c->full=1;return 1;}SlRecord *r=&s->records[s->count++];r->klen=(uint8_t)n;r->vlen=(uint8_t)z;memcpy(r->key,key,n);memcpy(r->value,value,z);return 0;}
int sl_snapshot_capture(store *s,SlSnapshot *out){
 if(!s||!out)return STORE_INVALID;
 int r=SL_MUTANT==3?STORE_OK:store_check(s);if(r)return r;
 SlSnapshot staged={0};Capture c={&staged,0};r=store_scan(s,NULL,0,copy_record,&c);if(r)return r;if(c.full)return STORE_FULL;staged.generation=s->generation;staged.from_transaction=s->active!=0;staged.ready=1;*out=staged;return STORE_OK;
}
int sl_snapshot_scan(const SlSnapshot *s,const void *p,size_t n,size_t budget,SlVisit f,void *ctx,size_t *calls){
 if(!s||!s->ready||s->count>SL_RECORDS||n>16||(!p&&n)||!f||!calls)return STORE_INVALID;
 size_t count=0;for(size_t i=0;i<s->count&&count<budget;i++){const SlRecord *r=&s->records[i];if(n&&order(r->key,r->klen,p,n)<(SL_MUTANT==4?1:0))continue;count++;if(f(ctx,r))break;}*calls=count;return STORE_OK;
}
