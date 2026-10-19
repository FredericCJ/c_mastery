#include "fault_adapter.h"
#include "metaprog.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MPO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
typedef struct {unsigned calls,fail,live,drops;} Alloc;
static void *get(void *p,size_t n){Alloc *a=p;++a->calls;if(a->calls==a->fail)return NULL;void *q=malloc(n);if(q)++a->live;return q;}
static void put(void *p,void *q){Alloc *a=p;--a->live;++a->drops;free(q);}
static int owner(void){
    Alloc a={0};MptOwner o={0};unsigned char word[]={'A',0,'B'};MptAllocator al={get,put,&a};int status;unsigned okay;
    CHECK(mpt_assign(&o,(MptView){word,3,99},al)==MPT_OK&&a.live==1&&o.bytes!=word&&o.length==3&&!memcmp(o.bytes,word,3));
    unsigned char *old=o.bytes;a.fail=2;status=mpt_assign(&o,(MptView){(const unsigned char *)"x",1,7},al);okay=status==MPT_NOMEM&&o.bytes==old&&o.length==3&&o.cache==99&&a.live==1;
    if(okay)okay=!memcmp(o.bytes,word,3);
    mpt_drop(&o);CHECK(okay&&a.live==0&&!o.bytes&&!o.length&&!o.cache);mpt_drop(&o);CHECK(a.drops==1);
    a.fail=0;CHECK(mpt_assign(&o,(MptView){NULL,0,1},al)==MPT_OK&&o.bytes&&o.length==0&&a.live==1);mpt_drop(&o);CHECK(!a.live);puts("MPT_OWNER copied-storage failed-replacement-unchanged exact-cleanup");return 0;
}
static int canonical(void){
    MptView a={(const unsigned char *)" A  B ",6,1},b={(const unsigned char *)"a b",3,99};unsigned equal=99;uint64_t x=0,y=0;
    CHECK(mpt_equal(a,b,MPT_BYTES,&equal)==MPT_OK&&!equal);CHECK(mpt_equal(a,b,MPT_ASCII_TEXT,&equal)==MPT_OK&&equal);CHECK(mpt_hash(a,MPT_ASCII_TEXT,&x)==MPT_OK&&mpt_hash(b,MPT_ASCII_TEXT,&y)==MPT_OK&&x==y);
    a=(MptView){(const unsigned char *)"a",1,0};b=a;b.cache=77;CHECK(mpt_hash(a,MPT_BYTES,&x)==MPT_OK&&mpt_hash(b,MPT_BYTES,&y)==MPT_OK&&x==y);
    unsigned char bytes[]={0,1};a=(MptView){bytes,2,0};CHECK(mpt_hash(a,MPT_BYTES,&x)==MPT_OK&&x==UINT64_C(16906520802787011418));
    x=99;equal=99;CHECK(mpt_hash(a,MPT_ASCII_TEXT,&x)==MPT_RANGE&&x==99&&mpt_equal(a,b,MPT_ASCII_TEXT,&equal)==MPT_RANGE&&equal==99);
    const char *texts[]={"","A","a"," A ","B"," b  ","ab","a b"};unsigned i,j,k;
    for(i=0;i<8;++i)for(j=0;j<8;++j)for(k=0;k<8;++k){unsigned ab,ba,bc,ac;MptView u={(const unsigned char *)texts[i],strlen(texts[i]),i},v={(const unsigned char *)texts[j],strlen(texts[j]),j},w={(const unsigned char *)texts[k],strlen(texts[k]),k};CHECK(mpt_equal(u,u,MPT_ASCII_TEXT,&equal)==MPT_OK&&equal);CHECK(mpt_equal(u,v,MPT_ASCII_TEXT,&ab)==MPT_OK&&mpt_equal(v,u,MPT_ASCII_TEXT,&ba)==MPT_OK&&ab==ba);CHECK(mpt_equal(v,w,MPT_ASCII_TEXT,&bc)==MPT_OK&&mpt_equal(u,w,MPT_ASCII_TEXT,&ac)==MPT_OK);if(ab&&bc)CHECK(ac);if(ab){CHECK(mpt_hash(u,MPT_ASCII_TEXT,&x)==MPT_OK&&mpt_hash(v,MPT_ASCII_TEXT,&y)==MPT_OK&&x==y);}}
    puts("MPT_CANONICAL explicit-ascii-equivalence prefix-length cache-exclusion");return 0;
}
static int eq(void *p,MptView a,MptView b){unsigned variant=*(unsigned *)p,same=0;if(variant==3)return 1;if(variant==2&&(!a.length||!b.length))return -1;return mpt_equal(a,b,MPT_ASCII_TEXT,&same)==MPT_OK?(int)same:-1;}
static uint64_t hash(void *p,MptView a){unsigned variant=*(unsigned *)p;uint64_t value=0;if(variant==3)return a.length==1&&(a.bytes[0]=='A'||a.bytes[0]=='B');(void)mpt_hash(a,variant?MPT_BYTES:MPT_ASCII_TEXT,&value);return value;}
static MptTrace seed(void){MptTrace t={{{MPT_SET,0,4},{MPT_SET,1,4},{MPT_DROP,1,0},{MPT_SET,0,3},{MPT_SET,1,2},{MPT_CHECK,0,0}},6,0};return t;}
static int shrinking(void){
    unsigned variant=2,attempts=99;MptLaws laws={eq,hash,&variant};MptTrace t=seed(),out={0},old=out;MptOutcome o={0};
    CHECK(mpt_replay(&t,laws,&o)==MPT_OK&&o.failure==MPT_HASH_LAW&&o.first_step==5&&!o.live&&o.allocations==4&&o.releases==4);
    CHECK(mpt_shrink(&t,laws,&out,&attempts)==MPT_OK&&out.count==3&&out.steps[0].operation==MPT_SET&&out.steps[0].value==2&&out.steps[1].operation==MPT_SET&&out.steps[1].value==1&&out.steps[2].operation==MPT_CHECK&&attempts>0);
    CHECK(mpt_replay(&out,laws,&o)==MPT_OK&&o.failure==MPT_HASH_LAW&&!o.live);variant=0;attempts=99;out=old;CHECK(mpt_shrink(&t,laws,&out,&attempts)==MPT_NO_FAILURE&&attempts==99&&out.count==old.count);
    t=(MptTrace){{{MPT_SET,0,1},{MPT_SET,1,2},{MPT_SET,0,4},{MPT_CHECK,0,0}},4,3};CHECK(mpt_replay(&t,laws,&o)==MPT_OK&&o.failure==MPT_NONE&&o.refusals==1&&o.releases==2&&!o.live);
    t.fail_at=1;o.first_step=99;CHECK(mpt_replay(&t,laws,&o)==MPT_OK);t.steps[2]=(MptStep){MPT_DROP,0,0};o.first_step=99;CHECK(mpt_replay(&t,laws,&o)==MPT_INVALID&&o.first_step==99);
    variant=3;t=(MptTrace){{{MPT_SET,0,4},{MPT_SET,1,2},{MPT_CHECK,0,0}},3,0};CHECK(mpt_shrink(&t,laws,&out,&attempts)==MPT_OK&&out.steps[0].value==4&&out.steps[1].value==2);
    t.steps[0].value=1;t.steps[1].value=0;CHECK(mpt_replay(&t,laws,&o)==MPT_OK&&o.failure==MPT_HASH_LAW);
    puts("MPT_SHRINK admitted-history same-failure-class local-deletion-minimum");return 0;
}
static int prop(MPPair p,void *c){(void)c;return p.x+p.y<5;}
static int base(void){
    unsigned a,b,c;MPPair witness={99,99};CHECK(!mp_equal((MPKey){1,2,0},(MPKey){1,3,0}));CHECK(mp_hash((MPKey){1,2,0})==mp_hash((MPKey){1,2,99}));
    for(a=0;a<8;++a)for(b=0;b<8;++b)for(c=0;c<8;++c){MPKey x={a/2,a%2,a},y={b/2,b%2,b},z={c/2,c%2,c};CHECK(mp_equal(x,x));CHECK(mp_equal(x,y)==mp_equal(y,x));if(mp_equal(x,y)&&mp_equal(y,z))CHECK(mp_equal(x,z));if(mp_equal(x,y))CHECK(mp_hash(x)==mp_hash(y));}
    CHECK(mp_find_counterexample(prop,NULL,8,&witness)==1&&witness.x+witness.y==5);puts("MPT_BASE actual-key-laws finite-triples unsigned-fnv");return 0;
}
static int oracle(void){char op;while(scanf(" %c",&op)==1){
    if(op=='H'){char hex[19];unsigned mode,cache;unsigned char bytes[9];size_t n,i;uint64_t h=99;int status;if(scanf("%u %u %18s",&mode,&cache,hex)!=3)return 2;n=!strcmp(hex,"-")?0:strlen(hex)/2;for(i=0;i<n;++i){unsigned b;if(sscanf(hex+i*2,"%2x",&b)!=1)return 2;bytes[i]=(unsigned char)b;}status=mpt_hash((MptView){bytes,n,cache},mode,&h);printf("%d %" PRIu64 "\n",status,h);}
    else if(op=='T'){MptTrace t={0},out={0};MptOutcome o={0};unsigned variant,shrink,attempts=99;size_t i;int status;if(scanf("%zu %u %u %u",&t.count,&t.fail_at,&variant,&shrink)!=4||t.count>12)return 2;for(i=0;i<t.count;++i)if(scanf("%u %u %u",&t.steps[i].operation,&t.steps[i].slot,&t.steps[i].value)!=3)return 2;MptLaws laws={eq,hash,&variant};status=shrink?mpt_shrink(&t,laws,&out,&attempts):mpt_replay(&t,laws,&o);if(shrink){printf("%d %u %zu %u",status,attempts,out.count,out.fail_at);for(i=0;i<out.count;++i)printf(" %u %u %u",out.steps[i].operation,out.steps[i].slot,out.steps[i].value);puts("");}else printf("%d %u %zu %u %u %u %u\n",status,o.failure,o.first_step,o.allocations,o.refusals,o.releases,o.live);}
    else return 2;
}return 0;}
static int metrics(void){double seconds[7];unsigned sample,i,variant=2,attempts;uint64_t checksum=0;MptTrace in=seed(),out;MptLaws laws={eq,hash,&variant};for(sample=0;sample<7;++sample){clock_t start=clock();for(i=0;i<1000;++i){CHECK(mpt_shrink(&in,laws,&out,&attempts)==MPT_OK);checksum+=out.count;}seconds[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":1000,\"checksum\":%" PRIu64 ",\"owner_size\":%zu,\"trace_size\":%zu,\"seconds\":[",checksum,sizeof(MptOwner),sizeof(MptTrace));for(i=0;i<7;++i)printf("%s%.9f",i?",":"",seconds[i]);puts("]}");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"law_owner"))return owner();if(!strcmp(argv[1],"law_canonical"))return canonical();if(!strcmp(argv[1],"law_shrink"))return shrinking();if(!strcmp(argv[1],"law_base"))return base();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
