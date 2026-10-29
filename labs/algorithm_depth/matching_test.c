#include "algorithms.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ALGORITHM_ASSERT matching line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static size_t oracle(const unsigned char*p,size_t n,const unsigned char*q,size_t m,uint64_t*out){size_t count=0;for(size_t start=0;start<n;++start){if(m>n-start)continue;unsigned same=1;for(size_t i=0;i<m;++i)if(p[start+i]!=q[i])same=0;if(same)out[count++]=start;}return count;}
static int exercise(const unsigned char*p,size_t n,const unsigned char*q,size_t m){
 uint64_t expected[64],got[64],collisions;size_t need=oracle(p,n,q,m,expected),count;
 CHECK(ad_match_naive(p,n,q,m,got,64,&count)==AD_OK&&count==need&&!memcmp(got,expected,need*sizeof*got));
 CHECK(ad_match_hash(p,n,q,m,got,64,&count,&collisions)==AD_OK&&count==need&&!memcmp(got,expected,need*sizeof*got));
 for(size_t split=0;split<=n;++split){struct ad_matcher state;CHECK(ad_match_init(&state,q,m)==AD_OK);size_t first,second;CHECK(ad_match_feed(&state,p,split,got,64,&first)==AD_OK);CHECK(ad_match_feed(&state,n==split?NULL:p+split,n-split,got+first,64-first,&second)==AD_OK);CHECK(first+second==need&&!memcmp(got,expected,need*sizeof*got));CHECK(state.comparisons<=2*n&&state.fallbacks<=n);}
 if(need){struct ad_matcher state;CHECK(ad_match_init(&state,q,m)==AD_OK);struct ad_matcher old=state;count=123;for(unsigned i=0;i<64;++i)got[i]=987;CHECK(ad_match_feed(&state,p,n,got,need-1,&count)==AD_SPACE&&!memcmp(&old,&state,sizeof state)&&count==123);for(unsigned i=0;i<64;++i)CHECK(got[i]==987);}
 return 0;
}
int main(void){
 unsigned char text[]={1,0,1,0,1},pattern[]={1,0,1};CHECK(!exercise(text,5,pattern,3));
 unsigned char collision_text[]={0,2},collision_pattern[]={1,0};uint64_t output[64],collisions=99;size_t count=99;CHECK(ad_match_hash(collision_text,2,collision_pattern,2,output,64,&count,&collisions)==AD_OK&&!count&&collisions==1);
 struct ad_matcher state;CHECK(ad_match_init(&state,pattern,3)==AD_OK);state.consumed=UINT64_MAX-1;struct ad_matcher old=state;CHECK(ad_match_feed(&state,text,2,output,64,&count)==AD_OVERFLOW&&!memcmp(&old,&state,sizeof state));CHECK(ad_match_init(&state,NULL,0)==AD_INVALID);CHECK(ad_match_naive(NULL,0,NULL,0,output,64,&count)==AD_INVALID);
 unsigned char prefix[64],long_pattern[16];for(unsigned i=0;i<64;++i)prefix[i]='a';for(unsigned i=0;i<16;++i)long_pattern[i]=i==15?'b':'a';CHECK(!exercise(prefix,64,long_pattern,16));
 size_t cases=0;for(size_t n=0;n<=7;++n)for(unsigned text_code=0;text_code<(1u<<n);++text_code){unsigned char p[7];for(size_t i=0;i<n;++i)p[i]=(unsigned char)((text_code>>i)&1u);for(size_t m=1;m<=4;++m)for(unsigned pattern_code=0;pattern_code<(1u<<m);++pattern_code){unsigned char q[4];for(size_t i=0;i<m;++i)q[i]=(unsigned char)((pattern_code>>i)&1u);CHECK(!exercise(p,n,q,m));++cases;}}
 printf("ALGORITHM_MATCHING_PASS cases=%zu all_two_chunk_splits transactional_admission collision_verified\n",cases);return 0;
}
