#include "lookup.h"
#include <limits.h>
_Static_assert(CHAR_BIT==8&&sizeof(int)==4&&sizeof(unsigned)==4,"selected ABI integers");
_Static_assert(sizeof(CDBucket)==12&&offsetof(CDBucket,key)==0&&offsetof(CDBucket,value)==4&&offsetof(CDBucket,state)==8,"actual typed table layout");
_Static_assert(sizeof(ALRequest)==8&&offsetof(ALRequest,start)==4&&offsetof(ALProbe,used)==4&&offsetof(ALProbe,visits)==8,"assembly trace layout");
static int valid(const CDHash*h){
 if(!h||!h->slots||!h->capacity||h->capacity>16||h->clustered>1||!h->allocator.take||!h->allocator.release)return 0;
 unsigned live=0,occupied=0;
 for(unsigned i=0;i<h->capacity;++i){const CDBucket*b=h->slots+i;if(b->state>2)return 0;occupied+=b->state!=0;live+=b->state==1;
  if(b->state==1){unsigned at=h->clustered?0:b->key%h->capacity;for(unsigned j=0;j<h->capacity;++j){if(at==i)break;if(!h->slots[at].state||(h->slots[at].state==1&&h->slots[at].key==b->key))return 0;if(++at==h->capacity)at=0;}}}
 return live==h->live&&occupied==h->occupied;
}
enum ALStatus al_lookup(const CDHash*h,uint32_t key,int*out,ALProbe*trace){
 if(!out||!trace||!valid(h))return AL_INVALID;
 ALRequest request={key,h->clustered?0:key%h->capacity};ALProbe result={-1,0,{0}};al_lookup_kernel(h->slots,h->capacity,&request,&result);*trace=result;
 if(result.index<0)return AL_MISSING;
 *out=h->slots[result.index].value;return AL_OK;
}
enum ALStatus al_quadratic(unsigned cap,unsigned start,ALProbe*out){
 if(!out||!cap||cap>16||start>=cap)return AL_INVALID;
 ALProbe p={-1,0,{0}};for(unsigned i=0;i<cap;++i)p.visits[p.used++]=(start+i*i)%cap;*out=p;return AL_OK;
}
static int spans(const unsigned char*p,size_t n,const unsigned char*q,size_t m){return n<=64&&m<=16&&(!n||p)&&(!m||q);}
static size_t enumerate(const unsigned char*p,size_t n,const unsigned char*q,size_t m,uint64_t*found,uint64_t*comparisons){
 size_t used=0;*comparisons=0;if(m>n)return 0;
 for(size_t start=0;start<=n-m;++start){size_t j=0;for(;j<m;++j){++*comparisons;if(p[start+j]!=q[j])break;}if(j==m){found[used++]=start;
#ifdef AL_MUTANT_OVERLAP
   if(m)start+=m-1;
#endif
  }}return used;
}
enum ALStatus al_match(const unsigned char*p,size_t n,const unsigned char*q,size_t m,uint64_t*out,size_t cap,size_t*count,ALMatchWork*work){
 if(!count||!work||!spans(p,n,q,m))return AL_INVALID;
 uint64_t found[65],comparisons;size_t used=enumerate(p,n,q,m,found,&comparisons);ALMatchWork next={comparisons,used};
 if(cap<used||(used&&!out)){
#ifndef AL_MUTANT_COUNT
  *count=used;
#endif
  *work=next;return AL_SPACE;}
 for(size_t i=0;i<used;++i)out[i]=found[i];
 *count=used;*work=next;return AL_OK;
}
enum ALStatus al_publish(const unsigned char*p,size_t n,const unsigned char*q,size_t m,ALPublish publish,void*context,ALPublication*out){
 if(!out||!publish||!spans(p,n,q,m))return AL_INVALID;
 uint64_t found[65],comparisons;size_t used=enumerate(p,n,q,m,found,&comparisons);ALPublication next={0,0,comparisons};
 for(size_t i=0;i<used;++i){++next.attempted;int result=publish(context,found[i]);if(result){
#ifndef AL_MUTANT_CANCEL
   *out=next;return result==2?AL_CANCELLED:AL_FAILED;
#endif
  }else ++next.accepted;}*out=next;return AL_OK;
}
enum ALStatus al_stream_init(ALStream*out,const unsigned char*p,size_t n){
 if(!out||n>16||(n&&!p))return AL_INVALID;
 ALStream s={0};s.empty=n==0;if(n&&ad_match_init(&s.prefix,p,n)!=AD_OK)return AL_INVALID;*out=s;return AL_OK;
}
enum ALStatus al_stream_feed(ALStream*state,const unsigned char*p,size_t n,uint64_t*out,size_t cap,size_t*count){
 if(!state||!count||n>64||(n&&!p)||state->empty>1||state->started>1||(!state->empty&&state->consumed!=state->prefix.consumed))return AL_INVALID;
 if(n>UINT64_MAX-state->consumed)return AL_OVERFLOW;
 ALStream next=*state;uint64_t found[65];size_t used=0;
 if(next.empty){if(!next.started)found[used++]=next.consumed;for(size_t i=0;i<n;++i)found[used++]=next.consumed+i+1;}
 else {enum ADStatus s=ad_match_feed(&next.prefix,p,n,found,65,&used);if(s!=AD_OK)return s==AD_OVERFLOW?AL_OVERFLOW:AL_INVALID;}
 next.consumed+=n;next.started=1;
 if(cap<used||(used&&!out)){*count=used;return AL_SPACE;}
 for(size_t i=0;i<used;++i)out[i]=found[i];
 *count=used;*state=next;return AL_OK;
}
