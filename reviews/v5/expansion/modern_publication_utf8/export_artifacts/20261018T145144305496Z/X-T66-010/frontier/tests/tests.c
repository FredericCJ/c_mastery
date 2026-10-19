#include "frontier.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OW_CHECK OCF line%d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {
 OwAllocator pool;V4Allocator upstream;OcfPlan plan;OcfOwner *published;
 unsigned events[32],n,fail_init,bad,constructing;int32_t *by_id[4];
} Harness;
static void event(Harness *h,unsigned value){if(h->n<32)h->events[h->n++]=value;else h->bad=1;}
static int32_t *take(void *p,size_t count){
 Harness *h=p;unsigned attempt=(unsigned)h->pool.attempts;
 if(attempt>=4){h->bad=1;return NULL;}
 unsigned id=h->plan.order[attempt];if(id>=4){h->bad=1;return NULL;}
 event(h,10+id);if(h->constructing&&h->published->count)h->bad=1;
 int32_t *r=h->upstream.take(h->upstream.context,count);h->by_id[id]=r;return r;
}
static void release(void *p,int32_t *block){
 Harness *h=p;unsigned id=4;for(unsigned i=0;i<4;++i)if(h->by_id[i]==block)id=i;
 if(id==4){h->bad=1;return;}
 event(h,40+id);if(h->constructing&&h->published->count)h->bad=1;
 h->upstream.release(h->upstream.context,block);h->by_id[id]=NULL;
}
static int initialize(void *p,unsigned id,int32_t *block,size_t length,const OcfView *v){
 Harness *h=p;event(h,20+id);unsigned deps=id==0?0:id==3?2:1;
 if((v->initialized&deps)!=deps||(v->initialized&(1u<<id))||h->published->count)h->bad=1;
 for(unsigned i=0;i<4;++i){if(((v->initialized>>i)&1u)!=(unsigned)(v->block[i]!=NULL))h->bad=1;}
 for(size_t i=0;i<length;++i)block[i]=(int32_t)(id+1);
 if(h->fail_init==id+1){block[0]=-99;return 1;}
 if(deps&&v->block[id==3?1:0]&&v->block[id==3?1:0][0]<=0)h->bad=1;
 return 0;
}
static void destroy(void *p,unsigned id,int32_t *block,size_t length,const OcfView *v){
 Harness *h=p;event(h,30+id);unsigned deps=id==0?0:id==3?2:1;
 if(!length||block[0]!=(int32_t)(id+1)||(v->initialized&deps)!=deps||(v->initialized&(1u<<id))||v->block[id]!=NULL)h->bad=1;
 block[0]=-7;
}
static void setup(Harness *h,OcfOwner *o,OcfPlan p,unsigned fail_take,unsigned fail_init){
 memset(h,0,sizeof(*h));h->plan=p;h->pool.fail_at=fail_take;h->fail_init=fail_init;h->published=o;h->constructing=1;h->upstream=ow_allocator(&h->pool);
}
static OcfStatus construct(Harness *h,OcfOwner *o){return ocf_construct(o,(V4Allocator){h,take,release},(OcfCallbacks){h,initialize,destroy},&h->plan);}
static int baseline(void){
 for(unsigned fail=1;fail<=4;++fail){OcfOwner dummy={0};Harness h;OcfPlan p={{0,1,2,3},{1,2,3,4}};setup(&h,&dummy,p,fail,0);OwOwner o={0};size_t lengths[3]={1,2,3};
  OwStatus status=ow_construct(&o,(V4Allocator){&h,take,release},lengths);
  CHECK(status==(fail<=3?OW_NOMEM:OW_OK));CHECK(o.count==(fail<=3?0u:3u));CHECK(ow_destroy(&o)==OW_OK&&ow_destroy(&o)==OW_OK);CHECK(h.pool.live==0&&h.pool.invalid_releases==0);
  unsigned acquired=fail<=3?fail-1:3;CHECK(h.pool.releases==acquired);for(unsigned i=0;i<acquired;++i)CHECK(h.events[h.n-acquired+i]==40+acquired-1-i);
 }
 OcfOwner dummy={0};Harness h;OcfPlan p={{0,1,2,3},{1,2,3,4}};setup(&h,&dummy,p,0,0);OwOwner o={0};size_t invalid[3]={1,0,3};CHECK(ow_construct(&o,(V4Allocator){&h,take,release},invalid)==OW_INVALID&&h.pool.attempts==0);invalid[1]=SIZE_MAX/sizeof(int32_t)+1;CHECK(ow_construct(&o,(V4Allocator){&h,take,release},invalid)==OW_INVALID&&h.pool.attempts==0);
 puts("OCF_BASE actual-construction allocator reverse-release late-size-preflight");return 0;
}
static int dag(void){
 const unsigned orders[3][4]={{0,1,2,3},{0,1,3,2},{0,2,1,3}};
 for(unsigned k=0;k<3;++k){OcfOwner o={0};Harness h;OcfPlan p={{0},{1,2,3,4}};for(unsigned i=0;i<4;++i)p.order[i]=orders[k][i];setup(&h,&o,p,0,0);
  CHECK(construct(&h,&o)==OCF_OK&&o.count==4&&o.acquired==15&&o.initialized==15);h.constructing=0;
  for(unsigned id=0;id<4;++id)CHECK(o.block[id][o.length[id]-1]==(int32_t)id+1);
  CHECK(ocf_destroy(&o)==OCF_OK&&ocf_destroy(&o)==OCF_OK);CHECK(o.count==0&&o.acquired==0&&o.initialized==0&&!h.bad&&!h.pool.live&&h.pool.releases==4);
  for(unsigned i=0;i<4;++i)CHECK(h.events[8+2*i]==30+orders[k][3-i]&&h.events[9+2*i]==40+orders[k][3-i]);
 }
 puts("OCF_DAG three-topological-orders initialized-set dependency-preserving-cleanup");return 0;
}
static int partial(void){
 OcfOwner o={0};Harness h;OcfPlan p={{0,1,3,2},{1,2,3,4}};setup(&h,&o,p,0,3);CHECK(construct(&h,&o)==OCF_INIT_FAILED);const unsigned expected[]={10,20,11,21,13,23,12,22,42,33,43,31,41,30,40};
 CHECK(o.count==0&&o.acquired==0&&o.initialized==0&&h.pool.live==0&&h.pool.releases==4&&!h.bad&&h.n==sizeof(expected)/sizeof(expected[0]));for(unsigned i=0;i<h.n;++i)CHECK(h.events[i]==expected[i]);
 for(unsigned fail=1;fail<=4;++fail){setup(&h,&o,p,fail,0);CHECK(construct(&h,&o)==OCF_NOMEM&&!o.count&&!o.acquired&&!o.initialized&&!h.pool.live&&h.pool.releases==fail-1&&!h.bad);}
 puts("OCF_PARTIAL C-storage-acquired-but-uninitialized no-C-destructor unpublished-failure");return 0;
}
static int admission(void){
 OcfOwner o={0};Harness h;OcfPlan p={{0,2,3,1},{1,2,3,4}};setup(&h,&o,p,0,0);CHECK(construct(&h,&o)==OCF_INVALID&&h.pool.attempts==0);
 p=(OcfPlan){{0,1,2,3},{1,2,3,0}};setup(&h,&o,p,0,0);CHECK(construct(&h,&o)==OCF_INVALID&&h.pool.attempts==0);h.plan.length[3]=SIZE_MAX/sizeof(int32_t)+1;CHECK(construct(&h,&o)==OCF_INVALID&&h.pool.attempts==0);h.plan.length[3]=4;h.plan.order[3]=2;CHECK(construct(&h,&o)==OCF_INVALID&&h.pool.attempts==0);
 CHECK(ocf_construct(NULL,(V4Allocator){&h,take,release},(OcfCallbacks){&h,initialize,destroy},&p)==OCF_INVALID&&ocf_destroy(NULL)==OCF_INVALID);
 puts("OCF_ADMISSION all-lengths-and-graph-before-any-callback");return 0;
}
static int callbacks(void){OcfOwner o={0};Harness h;OcfPlan p={{0,1,2,3},{1,2,3,4}};setup(&h,&o,p,0,0);CHECK(construct(&h,&o)==OCF_OK&&!h.bad);h.constructing=0;CHECK(ocf_destroy(&o)==OCF_OK&&!h.bad);for(unsigned i=0;i<4;++i)CHECK(h.pool.storage[i][0]==INT32_MIN);puts("OCF_CALLBACKS clear-initialized-before-destroy dependencies-still-live matching-release");return 0;}
static int oracle(void){OcfPlan p;unsigned ft,fi;while(scanf("%u %u %u %u %u %u %zu %zu %zu %zu",&p.order[0],&p.order[1],&p.order[2],&p.order[3],&ft,&fi,&p.length[0],&p.length[1],&p.length[2],&p.length[3])==10){OcfOwner o={0};Harness h;setup(&h,&o,p,ft,fi);OcfStatus s=construct(&h,&o);unsigned count=o.count,acquired=o.acquired,initialized=o.initialized;if(s==OCF_OK){h.constructing=0;if(ocf_destroy(&o)!=OCF_OK)h.bad=1;}printf("%d %u %u %u %zu %zu %zu %u %u",s,count,acquired,initialized,h.pool.live,h.pool.attempts,h.pool.releases,h.bad,h.n);for(unsigned i=0;i<h.n;++i)printf(" %u",h.events[i]);putchar('\n');}return ferror(stdin)?2:0;}
static int metrics(void){double seconds[7];unsigned checksum=0;size_t takes=0,drops=0;for(unsigned s=0;s<7;++s){clock_t start=clock();for(unsigned i=0;i<1000;++i){OcfOwner o={0};Harness h;OcfPlan p={{0,1,3,2},{1,2,3,4}};setup(&h,&o,p,0,0);CHECK(construct(&h,&o)==OCF_OK);checksum+=o.initialized;h.constructing=0;CHECK(ocf_destroy(&o)==OCF_OK&&!h.bad);takes+=h.pool.attempts;drops+=h.pool.releases;}seconds[s]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":1000,\"checksum\":%u,\"takes\":%zu,\"releases\":%zu,\"owner_size\":%zu,\"seconds\":[",checksum,takes,drops,sizeof(OcfOwner));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",seconds[i]);puts("]}");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"frontier_base"))return baseline();if(!strcmp(argv[1],"frontier_dag"))return dag();if(!strcmp(argv[1],"frontier_partial"))return partial();if(!strcmp(argv[1],"frontier_admission"))return admission();if(!strcmp(argv[1],"frontier_callbacks"))return callbacks();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
