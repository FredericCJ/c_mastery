#include "lookup.h"
#include <limits.h>
#define CHECK(x) do{if(!(x))return al_failure(#x,__LINE__);}while(0)
struct al_bank {struct cd_bucket slots[16];unsigned allocated;};
static struct cd_bucket*take(void*c,unsigned n){struct al_bank*b=c;if(b->allocated||n>16)return 0;b->allocated=1;return b->slots;}
static void release(void*c,struct cd_bucket*p){struct al_bank*b=c;if(p==b->slots)b->allocated=0;}
static int equal(const void*a,const void*b,size_t n){const unsigned char*x=a,*y=b;for(size_t i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static int lookup(void){
 /* This fixture is a valid typed table even before the prerequisite is linked. */
 struct al_bank b={{{16,160,1},{9,90,2},{2,20,1},{0,0,0},{0,0,0},{0,0,0},{6,60,1},{14,140,1}},1};
 struct cd_hash h={b.slots,8,4,5,0,{&b,take,release}};struct al_probe p;int value=77;unsigned path[]={6,7,0,1,2,3};
 CHECK(al_lookup(&h,22,&value,&p)==AL_MISSING&&value==77&&p.index==-1&&p.used==6);
 for(unsigned i=0;i<6;++i)CHECK(p.visits[i]==path[i]);
 al_emit_lookup(0,22,&p);
 CHECK(al_lookup(&h,16,&value,&p)==AL_OK&&value==160&&p.index==0&&p.used==1);al_emit_lookup(0,16,&p);
 CHECK(al_lookup(&h,2,&value,&p)==AL_OK&&value==20&&p.index==2&&p.used==1);al_emit_lookup(0,2,&p);
 struct al_request r={22,6};struct {unsigned before;struct al_probe p;unsigned after;} guard={0x1234,{-7,0,{0}},0x5678};struct al_bank saved=b;
 CHECK(al_lookup_abi(b.slots,8,&r,&guard.p)==1&&guard.p.used==6&&guard.p.index==-1);
 CHECK(guard.before==0x1234&&guard.after==0x5678&&equal(&b,&saved,sizeof b));
 struct al_probe savedp=p;h.capacity=0;CHECK(al_lookup(&h,0,&value,&p)==AL_INVALID&&equal(&p,&savedp,sizeof p));h.capacity=8;
 h.slots[1].state=3;CHECK(al_lookup(&h,0,&value,&p)==AL_INVALID&&equal(&p,&savedp,sizeof p));h.slots[1].state=2;
 h.live=3;CHECK(al_lookup(&h,0,&value,&p)==AL_INVALID&&equal(&p,&savedp,sizeof p));h.live=4;
 h.slots[6].state=0;h.live=3;--h.occupied;CHECK(al_lookup(&h,14,&value,&p)==AL_INVALID);
 CHECK(al_quadratic(8,0,&p)==AL_OK&&p.used==8);unsigned seen=0;for(unsigned i=0;i<p.used;++i)seen|=1u<<p.visits[i];CHECK(seen==0x13);al_emit_lookup(2,0,&p);
 savedp=p;CHECK(al_quadratic(0,0,&p)==AL_INVALID&&equal(&savedp,&p,sizeof p));return 0;
}
static int prior(void){
 for(unsigned cluster=0;cluster<2;++cluster)for(unsigned cap=1;cap<=16;++cap){struct al_bank bank={0};struct cd_hash h;unsigned probes;CHECK(cd_hash_init(&h,cap,(struct cd_hash_allocator){&bank,take,release},cluster)==CD_OK);
  for(unsigned i=0;i<cap;++i)CHECK(cd_hash_put(&h,i*16u+3u,(int)(100+i),&probes)==CD_OK);
  for(unsigned i=0;i<=cap;++i){uint32_t key=i==cap?UINT32_MAX:i*16u+3u;int a=77,b=77;struct al_probe p;enum cd_status expected=cd_hash_get(&h,key,&a,&probes);enum al_status got=al_lookup(&h,key,&b,&p);CHECK((expected==CD_OK&&got==AL_OK)||(expected==CD_MISSING&&got==AL_MISSING));CHECK(a==b&&probes==p.used&&p.used<=cap);if(i==cap)CHECK(p.used==cap);}
  for(unsigned i=0;i<cap;++i)CHECK(cd_hash_delete(&h,i*16u+3u)==CD_OK);
  int value=77;struct al_probe p;CHECK(al_lookup(&h,99,&value,&p)==AL_MISSING&&value==77&&p.used==cap);if(cap==8)al_emit_lookup(1,99,&p);
  CHECK(cd_hash_destroy(&h)==CD_OK&&!bank.allocated);
 }
 return 0;
}
static size_t independent(const unsigned char*t,size_t n,const unsigned char*p,size_t m,uint64_t*out,uint64_t*cost){size_t count=0;*cost=0;if(m<=n)for(size_t i=0;i<=n-m;++i){int found=1;for(size_t j=0;j<m;++j){++*cost;if(t[i+j]!=p[j]){found=0;break;}}if(found)out[count++]=i;}return count;}
static int matching(void){
 const unsigned char t[]={0,97,0,97,0},p[]={0,97,0};uint64_t out[67];for(unsigned i=0;i<67;++i)out[i]=0xdead;size_t count=77;struct al_match_work w={77,77};
 CHECK(al_match(t,5,p,3,out+1,1,&count,&w)==AL_SPACE&&count==2&&w.comparisons==7&&w.required==2);
 for(unsigned i=0;i<67;++i)CHECK(out[i]==0xdead);
 CHECK(al_match(t,5,p,3,out+1,65,&count,&w)==AL_OK&&count==2&&out[1]==0&&out[2]==2&&w.comparisons==7&&out[0]==0xdead&&out[3]==0xdead);
 CHECK(al_match(t,5,p+1,2,out+1,65,&count,&w)==AL_OK&&count==2&&out[1]==1&&out[2]==3&&w.comparisons==6);
 for(size_t n=0;n<=5;++n)for(unsigned bits=0;bits<(1u<<n);++bits)for(size_t m=0;m<=3;++m)for(unsigned pattern=0;pattern<(1u<<m);++pattern){unsigned char text[64]={0},needle[16]={0};for(size_t i=0;i<n;++i)text[i]=(unsigned char)((bits>>i)&1u);for(size_t i=0;i<m;++i)needle[i]=(unsigned char)((pattern>>i)&1u);uint64_t expected[65],cost;size_t used=independent(text,n,needle,m,expected,&cost);count=77;for(unsigned i=0;i<67;++i)out[i]=0xdead;
  CHECK(al_match(text,n,needle,m,out+1,65,&count,&w)==AL_OK&&count==used&&w.comparisons==cost&&w.required==used&&out[0]==0xdead&&out[used+1]==0xdead);for(size_t i=0;i<used;++i)CHECK(out[i+1]==expected[i]);al_emit_match(bits,n,pattern,m,out+1,count,w.comparisons);
  for(size_t capacity=0;capacity<used;++capacity){for(unsigned i=0;i<67;++i)out[i]=0xdead;count=77;CHECK(al_match(text,n,needle,m,out+1,capacity,&count,&w)==AL_SPACE&&count==used&&w.comparisons==cost);for(unsigned i=0;i<67;++i)CHECK(out[i]==0xdead);}
 }
 unsigned char zeros[64]={0};count=77;CHECK(al_match(zeros,64,0,0,out+1,65,&count,&w)==AL_OK&&count==65&&out[65]==64&&w.comparisons==0&&out[66]==0xdead);
 CHECK(al_match(0,0,0,0,0,0,&count,&w)==AL_SPACE&&count==1);
 CHECK(al_match(0,0,p,3,0,0,&count,&w)==AL_OK&&count==0);
 unsigned char high[]={255,0,255};CHECK(al_match(high,3,high,1,out,65,&count,&w)==AL_OK&&count==2&&out[0]==0&&out[1]==2&&w.comparisons==3);
 count=77;w=(struct al_match_work){77,77};CHECK(al_match(0,1,p,3,out,67,&count,&w)==AL_INVALID&&count==77&&w.comparisons==77);return 0;
}
struct al_callback {uint64_t offsets[65];size_t accepted,calls,stop;unsigned result;};
static int callback(void*context,uint64_t at){struct al_callback*c=context;size_t call=c->calls++;if(call==c->stop)return(int)c->result;c->offsets[c->accepted++]=at;return 0;}
static int callbacks(void){
 const unsigned char text[]={0,97,0,97,0};struct al_publication p;
 for(unsigned result=1;result<=2;++result)for(size_t stop=0;stop<=6;++stop){struct al_callback c={{0},0,0,stop,result};enum al_status status=al_publish(text,5,0,0,callback,&c,&p);CHECK(status==(stop<6?(result==2?AL_CANCELLED:AL_FAILED):AL_OK));CHECK(c.accepted==(stop<6?stop:6)&&c.calls==(stop<6?stop+1:6)&&p.accepted==c.accepted&&p.attempted==c.calls&&p.comparisons==0);for(size_t i=0;i<c.accepted;++i)CHECK(c.offsets[i]==i);}
 struct al_callback c={{0},0,0,1,2};CHECK(al_publish(text,5,text,3,callback,&c,&p)==AL_CANCELLED&&c.calls==2&&c.accepted==1&&c.offsets[0]==0&&p.comparisons==7);
 struct al_publication before=p;CHECK(al_publish(0,1,0,0,callback,&c,&p)==AL_INVALID&&equal(&p,&before,sizeof p)&&c.calls==2);return 0;
}
static int stream(void){
 for(size_t n=0;n<=5;++n)for(unsigned bits=0;bits<(1u<<n);++bits)for(size_t m=0;m<=3;++m)for(unsigned pattern=0;pattern<(1u<<m);++pattern){unsigned char text[6]={0},needle[4]={0};for(size_t i=0;i<n;++i)text[i]=(unsigned char)((bits>>i)&1u);for(size_t i=0;i<m;++i)needle[i]=(unsigned char)((pattern>>i)&1u);uint64_t expected[65],cost;size_t used=independent(text,n,needle,m,expected,&cost);
  for(size_t split=0;split<=n;++split){struct al_stream s;uint64_t out[67];for(unsigned i=0;i<67;++i)out[i]=0xdead;CHECK(al_stream_init(&s,needle,m)==AL_OK);size_t a=77,b=77;CHECK(al_stream_feed(&s,text,split,out+1,65,&a)==AL_OK);CHECK(al_stream_feed(&s,text+split,n-split,out+1+a,65-a,&b)==AL_OK&&a+b==used&&s.consumed==n);CHECK(out[0]==0xdead&&out[used+1]==0xdead);for(size_t i=0;i<used;++i)CHECK(out[i+1]==expected[i]);al_emit_stream(bits,n,pattern,m,split,out+1,used);
   size_t z=77;CHECK(al_stream_feed(&s,0,0,0,0,&z)==AL_OK&&z==0);
  }
 }
 unsigned char text[]={0,97,0,97,0};struct al_stream s;CHECK(al_stream_init(&s,text,3)==AL_OK);struct al_stream before=s;size_t n=77;uint64_t out[4]={99,99,99,99};CHECK(al_stream_feed(&s,text,5,out+1,1,&n)==AL_SPACE&&n==2&&equal(&s,&before,sizeof s)&&out[1]==99&&out[2]==99);
 struct ad_matcher prior;CHECK(ad_match_init(&prior,text,3)==AD_OK);struct ad_matcher saved=prior;n=77;CHECK(ad_match_feed(&prior,text,5,out+1,1,&n)==AD_SPACE&&n==77&&equal(&prior,&saved,sizeof prior));CHECK(ad_match_init(&prior,0,0)==AD_INVALID);
 n=77;CHECK(al_stream_feed(&s,text,5,out+1,2,&n)==AL_OK&&n==2&&out[1]==0&&out[2]==2&&out[0]==99&&out[3]==99);
 CHECK(al_stream_init(&s,0,0)==AL_OK);s.consumed=UINT64_MAX;s.started=1;before=s;n=77;CHECK(al_stream_feed(&s,text,1,out,4,&n)==AL_OVERFLOW&&n==77&&equal(&s,&before,sizeof s));return 0;
}
int al_test(unsigned which){switch(which){case 0:return lookup();case 1:return prior();case 2:return matching();case 3:return callbacks();case 4:return stream();default:return 2;}}
