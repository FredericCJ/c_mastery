#include "depth.h"
#include <limits.h>
int b2_parse(const unsigned char *p,size_t n,int32_t *out){
 size_t i=0;int negative=0;uint32_t v=0,limit;
 if(!out||(!p&&n))return B2_INVALID;
 if(!n)return B2_INVALID;
 if(p[0]==45){negative=1;i=1;}
 if(i==n)return B2_INVALID;
 /* Validate complete grammar before range admission. ASCII wire octets. */
#ifndef B2_MUTATE_PARSE
 if(p[i]==48&&(n-i!=1||negative))return B2_INVALID;
#endif
 for(size_t j=i;j<n;j++)if(p[j]<48||p[j]>57)return B2_INVALID;
 limit=negative?UINT32_C(2147483648):UINT32_C(2147483647);
 for(;i<n;i++){unsigned digit=p[i]-48u;if(v>(limit-digit)/10u)return B2_RANGE;v=v*10u+digit;}
 if(negative)*out=v==UINT32_C(2147483648)?INT32_MIN:-(int32_t)v;else *out=(int32_t)v;
 return B2_OK;
}
void b2_map_init(B2Map *m){size_t i;for(i=0;i<7;i++)m->slots[i]=(B2Entry){0,0,0};m->count=0;}
int b2_map_get(const B2Map *m,uint16_t k,int *out){
 size_t i,j;if(!m||!out)return B2_INVALID;i=k%7u;
 for(j=0;j<7;j++,i=(i+1)%7){if(m->slots[i].state==0)return B2_ABSENT;if(m->slots[i].state==1&&m->slots[i].key==k){*out=m->slots[i].value;return B2_OK;}}return B2_ABSENT;
}
int b2_map_put(B2Map *m,uint16_t k,int v){
 size_t i,j,free_slot=7;if(!m)return B2_INVALID;i=k%7u;
 for(j=0;j<7;j++,i=(i+1)%7){if(m->slots[i].state==1&&m->slots[i].key==k){m->slots[i].value=v;return B2_OK;}if(m->slots[i].state!=1&&free_slot==7)free_slot=i;if(m->slots[i].state==0)break;
#ifdef B2_MUTATE_MAP
 if(m->slots[i].state==2)break;
#endif
 }
 if(free_slot==7)return B2_FULL;
 m->slots[free_slot]=(B2Entry){1,k,v};m->count++;return B2_OK;
}
int b2_map_remove(B2Map *m,uint16_t k){size_t i,j;if(!m)return B2_INVALID;i=k%7u;for(j=0;j<7;j++,i=(i+1)%7){if(m->slots[i].state==0)return B2_ABSENT;if(m->slots[i].state==1&&m->slots[i].key==k){m->slots[i].state=2;m->count--;return B2_OK;}}return B2_ABSENT;}
#ifdef B2_MUTATE_SORT
#define LEFT_WINS(a,b) ((a)<(b))
#else
#define LEFT_WINS(a,b) ((a)<=(b))
#endif
static void merge(B2Item *a,B2Item *s,size_t n){size_t mid=n/2,i=0,j=mid,k=0;if(n<2)return;merge(a,s,mid);merge(a+mid,s+mid,n-mid);while(i<mid&&j<n)s[k++]=LEFT_WINS(a[i].key,a[j].key)?a[i++]:a[j++];while(i<mid)s[k++]=a[i++];while(j<n)s[k++]=a[j++];for(i=0;i<n;i++)a[i]=s[i];}
int b2_sort(B2Item *a,size_t n,B2Item *s,size_t c){if(n&&!a)return B2_INVALID;if(n<2)return B2_OK;if(!s)return B2_INVALID;if(c<n)return B2_FULL;merge(a,s,n);return B2_OK;}
void b2_arena_init(B2Arena *a){a->cursor=0;a->depth=0;a->next_id=1;}
int b2_reserve(B2Arena *a,size_t n,size_t l,size_t *o){size_t pad;if(!a||!o||!l||l>8||(l&(l-1)))return B2_INVALID;pad=(l-a->cursor%l)%l;if(a->cursor>64||pad>64-a->cursor||n>64-a->cursor-pad)return B2_FULL;*o=a->cursor+pad;a->cursor+=pad+n;return B2_OK;}
int b2_mark(B2Arena *a,B2Mark *o){if(!a||!o)return B2_INVALID;if(a->depth==4||a->next_id>15)return B2_FULL;a->saved[a->depth]=a->cursor;a->ids[a->depth]=a->next_id;*o=(B2Mark){a,a->depth+1,a->next_id};a->next_id++;a->depth++;return B2_OK;}
int b2_rollback(B2Arena *a,B2Mark m){if(!a)return B2_INVALID;
#ifdef B2_MUTATE_ARENA
 if(m.owner==a&&m.depth&&m.depth<=a->depth){a->cursor=a->saved[m.depth-1];a->depth=m.depth-1;return B2_OK;}
#endif
 if(m.owner!=a||m.depth!=a->depth||!a->depth||m.id!=a->ids[a->depth-1])return B2_STALE;
 a->cursor=a->saved[--a->depth];return B2_OK;}
int b2_q7(const int8_t *a,const int8_t *b,size_t n,int8_t *out){int32_t sum=0,r;size_t i;if(!out||((!a||!b)&&n))return B2_INVALID;if(n>1024)return B2_RANGE;for(i=0;i<n;i++){
#ifdef B2_MUTATE_Q7
 int32_t p=(int32_t)a[i]*(int32_t)b[i];sum+=p>=0?(p+64)/128:-((-p+64)/128);
#else
 sum+=(int32_t)a[i]*(int32_t)b[i];
#endif
 }
#ifdef B2_MUTATE_Q7
 r=sum;
#else
 r=sum>=0?(sum+64)/128:-((-sum+64)/128);
#endif
 if(r>127)r=127;
 if(r< -128)r= -128;
 *out=(int8_t)r;return B2_OK;}
int b2_transaction(B2Backend b,unsigned *p){unsigned i;if(!p||!b.acquire||!b.release)return B2_INVALID;for(i=0;i<3;i++)if(b.acquire(b.context,i)!=0){
#ifdef B2_MUTATE_TRANSACTION
 b.release(b.context,i);
#endif
 while(i)b.release(b.context,--i);
 return B2_FAILED;}*p=3;return B2_OK;}
int b2_render(const B2Result *r,unsigned char *o,size_t c,size_t *w){unsigned char temp[16];size_t n=0,i;unsigned magnitude;if(!r||!w||(!o&&c))return B2_INVALID;
#ifdef B2_MUTATE_VARIANT
 if(c)o[0]=0;
#endif
if(r->tag==1){if(!r->value.text.data&&r->value.text.length)return B2_INVALID;n=r->value.text.length;if(c<n)return B2_FULL;for(i=0;i<n;i++)o[i]=r->value.text.data[i];*w=n;return B2_OK;}if(r->tag!=0&&r->tag!=2)return B2_INVALID;{int x=r->tag==0?r->value.number:r->value.error;if(x< -9999||x>9999)return B2_RANGE;magnitude=x<0?(unsigned)(-x):(unsigned)x;do{temp[n++]=(unsigned char)(48+magnitude%10);magnitude/=10;}while(magnitude);if(x<0)temp[n++]=45;}if(c<n)return B2_FULL;for(i=0;i<n;i++)o[i]=temp[n-i-1];*w=n;return B2_OK;}
int b2_identity_int(int x){return x;}
unsigned b2_identity_unsigned(unsigned x){return x;}
