#include "policy_effects.h"
#include "metaprog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
static void *live_owner;
static void cleanup(void){free(live_owner);live_owner=NULL;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MPO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);cleanup();return 1;}}while(0)
struct mp_context {uint32_t values[32];size_t count;unsigned acquisition_failure,project_failure,acquires,projects,releases,live;uint64_t trace;unsigned multiply;size_t current;};
static void event(struct mp_context*c,unsigned kind,unsigned index){c->trace=c->trace*257+kind*32+index;}
static int acquire(void*context,size_t index,struct mpp_item*out){struct mp_context*c=context;c->acquires++;c->current=index;event(c,1,(unsigned)index);if(c->acquisition_failure==2*index+1)return 1;live_owner=malloc(1);if(!live_owner)return 1;c->live++;*out=(struct mpp_item){live_owner,c->values[index]};return c->acquisition_failure==2*index+2;}
static int project(void*context,const struct mpp_item*item,uint32_t*out){struct mp_context*c=context;c->projects++;event(c,2,(unsigned)c->current);if(c->project_failure==c->current+1)return 1;*out=c->multiply?item->raw*UINT32_C(3):item->raw;return 0;}
static void release(void*context,struct mpp_item*item){struct mp_context*c=context;c->releases++;event(c,3,(unsigned)c->current);if(item->owner){free(item->owner);live_owner=NULL;c->live--;}*item=(struct mpp_item){0};}
static struct mpp_ops operations(struct mp_context*c){return (struct mpp_ops){c,acquire,project,release};}
static int apply(unsigned variant,unsigned policy,struct mp_context*c,uint32_t limit,uint32_t*out){return variant?policy?mpp_saturate(c->count,limit,operations(c),out):mpp_reject(c->count,limit,operations(c),out):mpp_dynamic(c->count,limit,policy,operations(c),out);}
static int order(void){for(unsigned v=0;v<2;v++){struct mp_context c={{1,2,3},3,0,0,0,0,0,0,0,1,0};uint32_t out=99;CHECK(apply(v,0,&c,100,&out)==MPP_OK&&out==18&&c.acquires==3&&c.projects==3&&c.releases==3&&c.live==0);uint64_t trace=0;for(unsigned i=0;i<3;i++)for(unsigned k=1;k<=3;k++)trace=trace*257+k*32+i;CHECK(c.trace==trace);}
 struct mp_context c={0};uint32_t out=99;struct mpp_ops ops=operations(&c);CHECK(mpp_dynamic(0,0,2,ops,&out)==MPP_INVALID&&out==99&&c.acquires==0);CHECK(mpp_reject(33,100,ops,&out)==MPP_INVALID&&out==99);ops.project=NULL;CHECK(mpp_saturate(0,0,ops,&out)==MPP_INVALID&&out==99);CHECK(mpp_saturate(0,0,operations(&c),&out)==MPP_OK&&out==0&&c.acquires==0);
 puts("MPP_ORDER acquire-project-release left-to-right once");return 0;}
static int failure(void){
 for(unsigned v=0;v<2;v++){
  for(unsigned policy=0;policy<2;policy++){
 for(unsigned point=1;point<=6;point++){struct mp_context c={{1,2,3},3,0,0,0,0,0,0,0,0,0};c.acquisition_failure=point;uint32_t out=99;CHECK(apply(v,policy,&c,100,&out)==MPP_ACQUIRE&&out==99&&c.live==0&&c.releases==(point+1)/2&&c.projects==(point-1)/2);}
 for(unsigned point=1;point<=3;point++){struct mp_context c={{1,2,3},3,0,0,0,0,0,0,0,0,0};c.project_failure=point;uint32_t out=99;CHECK(apply(v,policy,&c,100,&out)==MPP_PROJECT&&out==99&&c.live==0&&c.releases==point&&c.projects==point);}
  }
 }
 puts("MPP_FAILURE partial-acquisition release output-rollback effects-retained");return 0;}
static int saturated(void){for(unsigned v=0;v<2;v++){
 struct mp_context c={{7,5,9},3,0,0,0,0,0,0,0,0,0};uint32_t out=99;CHECK(apply(v,1,&c,10,&out)==MPP_OK&&out==10&&c.acquires==3&&c.projects==3&&c.releases==3);
 c=(struct mp_context){{7,5,9},3,0,3,0,0,0,0,0,0,0};out=99;CHECK(apply(v,1,&c,10,&out)==MPP_PROJECT&&out==99&&c.projects==3&&c.releases==3);
 c=(struct mp_context){{7,5,9},3,0,0,0,0,0,0,0,0,0};CHECK(apply(v,0,&c,10,&out)==MPP_RANGE&&out==99&&c.acquires==2&&c.projects==2&&c.releases==2);
 c=(struct mp_context){{7,5,9},3,0,0,0,0,0,0,0,0,0};CHECK(apply(v,1,&c,0,&out)==MPP_OK&&out==0&&c.projects==3);
 }puts("MPP_SATURATION mandatory-effects late-failure rejection-prefix");return 0;}
static int base(void){uint32_t values[]={7,5},out=99;CHECK(!mp_reduce(values,2,(struct mp_policy){10,0},&out)&&out==99);CHECK(mp_reduce_saturating(values,2,10,&out)&&out==10);CHECK(mp_reduce_reject(NULL,0,10,&out)&&out==0);puts("MPP_BASE actual-reject-saturate empty-identity");return 0;}
static int driver(void){unsigned variant,policy,n,af,pf,multiply;uint32_t limit;while(scanf("%u %u %u %" SCNu32 " %u %u %u",&variant,&policy,&n,&limit,&af,&pf,&multiply)==7){CHECK(n<=32);struct mp_context c={0};c.count=n;c.acquisition_failure=af;c.project_failure=pf;c.multiply=multiply;for(unsigned i=0;i<n;i++)CHECK(scanf("%" SCNu32,&c.values[i])==1);uint32_t out=99;int status=apply(variant,policy,&c,limit,&out);CHECK(c.live==0);uint32_t projected[32],base_out=99;for(unsigned i=0;i<n;i++)projected[i]=multiply?c.values[i]*UINT32_C(3):c.values[i];int base_ok=mp_reduce(projected,n,(struct mp_policy){limit,policy},&base_out);printf("%d %u %u %u %u %" PRIu64 " %d %u\n",status,out,c.acquires,c.projects,c.releases,c.trace,base_ok,base_out);}return ferror(stdin)?1:0;}
static int metrics(void){double times[2][2][7];for(unsigned policy=0;policy<2;policy++)for(unsigned variant=0;variant<2;variant++)for(unsigned sample=0;sample<7;sample++){struct mp_context c={{1,2,3,4},4,0,0,0,0,0,0,0,0,0};uint64_t sum=0;clock_t start=clock();for(unsigned i=0;i<10000;i++){uint32_t out;CHECK(apply(variant,policy,&c,policy?5:100,&out)==MPP_OK);sum+=out;}times[policy][variant][sample]=(double)(clock()-start)/CLOCKS_PER_SEC;CHECK(sum==(policy?50000:100000)&&c.acquires==40000&&c.projects==40000&&c.releases==40000&&c.live==0);}printf("{\"iterations\":10000,\"reject_checksum\":100000,\"saturate_checksum\":50000,\"callback_calls_each\":40000,\"item_size\":%zu,\"ops_size\":%zu",sizeof(struct mpp_item),sizeof(struct mpp_ops));for(unsigned p=0;p<2;p++)for(unsigned v=0;v<2;v++){printf(",\"%s_%s_seconds\":[",p?"saturate":"reject",v?"specialized":"dynamic");for(unsigned s=0;s<7;s++)printf("%s%.9f",s?",":"",times[p][v][s]);printf("]");}puts("}");return 0;}
int main(int argc,char**argv){if(argc!=2)return 2;if(!strcmp(argv[1],"policy_order"))return order();if(!strcmp(argv[1],"policy_failure"))return failure();if(!strcmp(argv[1],"policy_saturation"))return saturated();if(!strcmp(argv[1],"policy_base"))return base();if(!strcmp(argv[1],"driver"))return driver();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
