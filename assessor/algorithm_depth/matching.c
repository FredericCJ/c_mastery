#include "algorithms.h"
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static int spans(const unsigned char*p,size_t n,const unsigned char*q,size_t m){return n<=64&&m>=1&&m<=16&&(!n||p)&&q;}
static enum ad_status publish(const uint64_t*found,size_t used,uint64_t*out,size_t capacity,size_t*count){if(!count)return AD_INVALID;if(capacity<used||(used&&!out))return AD_SPACE;for(size_t i=0;i<used;++i)out[i]=found[i];*count=used;return AD_OK;}
enum ad_status ad_match_naive(const unsigned char*p,size_t n,const unsigned char*q,size_t m,uint64_t*out,size_t capacity,size_t*count){if(!spans(p,n,q,m))return AD_INVALID;uint64_t found[64];size_t used=0;if(m<=n)for(size_t start=0;start<=n-m;++start){size_t i=0;while(i<m&&p[start+i]==q[i])++i;if(i==m)found[used++]=start;}return publish(found,used,out,capacity,count);}
static void tick(uint64_t*value){if(*value!=UINT64_MAX)++*value;}
enum ad_status ad_match_init(struct ad_matcher*out,const unsigned char*p,size_t n){
 if(!out||!p||!n||n>16)return AD_INVALID;
 struct ad_matcher m={0};m.length=n;for(size_t i=0;i<n;++i)m.pattern[i]=p[i];
 for(size_t i=1,j=0;i<n;++i){while(j&&p[i]!=p[j])j=m.failure[j-1];if(p[i]==p[j])++j;m.failure[i]=j;}*out=m;return AD_OK;
}
enum ad_status ad_match_feed(struct ad_matcher*state,const unsigned char*p,size_t n,uint64_t*out,size_t capacity,size_t*count){
 if(!state||!count||!spans(p,n,state->pattern,state->length)||state->matched>=state->length)return AD_INVALID;
 /* The prefix table and pattern are immutable after successful initialization. */
 if(n>UINT64_MAX-state->consumed)return AD_OVERFLOW;
 struct ad_matcher m=*state;uint64_t found[64];size_t used=0;
 for(size_t i=0;i<n;++i){while(m.matched){tick(&m.comparisons);if(p[i]==m.pattern[m.matched])break;m.matched=m.failure[m.matched-1];tick(&m.fallbacks);}if(!m.matched){tick(&m.comparisons);if(p[i]==m.pattern[0])++m.matched;}else ++m.matched;
  ++m.consumed;if(m.matched==m.length){found[used++]=m.consumed-m.length;m.matched=AD_MUTANT==4?0:m.failure[m.length-1];}}
 enum ad_status status=publish(found,used,out,capacity,count);if(status==AD_OK)*state=m;return status;
}
enum ad_status ad_match_hash(const unsigned char*p,size_t n,const unsigned char*q,size_t m,uint64_t*out,size_t capacity,size_t*count,uint64_t*collisions){
 if(!spans(p,n,q,m)||!collisions)return AD_INVALID;
 uint64_t found[64],false_hits=0;size_t used=0;unsigned target=0,window=0,power=1;
 for(size_t i=0;i<m;++i){target=(target*2u+q[i])%17u;if(i)power=power*2u%17u;}
 if(m<=n){for(size_t i=0;i<m;++i)window=(window*2u+p[i])%17u;for(size_t start=0;start<=n-m;++start){if(window==target){size_t i=0;while(i<m&&p[start+i]==q[i])++i;if(i==m||AD_MUTANT==5)found[used++]=start;else ++false_hits;}if(start<n-m){unsigned removed=((unsigned)p[start]%17u)*power%17u;window=((window+17u-removed)*2u+p[start+m])%17u;}}}
 enum ad_status status=publish(found,used,out,capacity,count);if(status==AD_OK)*collisions=false_hits;return status;
}
