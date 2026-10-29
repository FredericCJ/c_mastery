#include "dispatch_modules.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
static const char *test;
#define CHECK(x) do { if(!(x)){fprintf(stderr,"DS_CHECK:%s:%d:%s\n",test,__LINE__,#x);return 1;} }while(0)
struct ds_reads {int values[2];unsigned calls,fail;int trace[2];};
static int read_value(void *p,int *out){struct ds_reads *r=p;unsigned n=r->calls++;if(n>=2)return DS_FAILED;r->trace[n]=r->values[n];*out=999;if(r->fail==n+1)return DS_FAILED;*out=r->values[n];return DS_OK;}
struct ds_io {unsigned clock_calls,sink_calls;int stamp,last;unsigned clock_fail,sink_fail;};
static int now(void *p,int *out){struct ds_io *io=p;++io->clock_calls;*out=io->stamp;return io->clock_fail?DS_FAILED:DS_OK;}
static int sink(void *p,int value){struct ds_io *io=p;++io->sink_calls;io->last=value;return io->sink_fail?DS_FAILED:DS_OK;}
static int raw(int output){const int values[5]={-2,0,7,INT_MIN,INT_MAX};
 for(unsigned a=0;a<5;++a)for(unsigned b=0;b<5;++b)for(unsigned fail=0;fail<3;++fail){struct ds_reads r={{values[a],values[b]},0,fail,{0}};struct ds_module *m=NULL;CHECK(ds_module_open((struct ds_service){read_value,&r},&m)==DS_OK);int out=91;int rc=ds_module_twice(m,&out);int want=fail?DS_FAILED:((values[b]>0 && values[a]>INT_MAX-values[b])||(values[b]<0 && values[a]<INT_MIN-values[b]))?DS_RANGE:DS_OK;CHECK(rc==want);CHECK(out==(want?91:values[a]+values[b]));CHECK(r.calls==(fail==1?1u:2u));CHECK(r.trace[0]==values[a]);if(r.calls==2)CHECK(r.trace[1]==values[b]);ds_module_close(m);
  if(output)printf("M %u %u %u %d %d %u %d %d\n",a,b,fail,rc,out,r.calls,r.trace[0],r.calls==2?r.trace[1]:0);
 }
 struct ds_reads r={{7,7},0,0,{0}};struct ds_module *a=NULL,*b=NULL,*c=NULL;struct ds_service s={read_value,&r};CHECK(ds_module_open(s,&a)==DS_OK);CHECK(ds_module_open(s,&b)==DS_OK);c=a;CHECK(ds_module_open(s,&c)==DS_FULL && c==a);ds_module_close(b);ds_module_close(a);return 0;
}
static unsigned destruction[4],destroy_count,released;
struct ds_resource {unsigned id;};
static void destroy(void *p){struct ds_resource *r=p;if(destroy_count<4)destruction[destroy_count++]=r->id;}
static void release(void *p){(void)p;++released;}
static int leases(void){
 struct ds_resource resource[2]={{1},{2}};struct ow_shared owners[2]={{1,1,destroy,release,&resource[0]},{1,1,destroy,release,&resource[1]}};
 struct ds_reads reads={{7,8},0,0,{0}};struct ds_io io={0,0,23,0,0,0};struct ds_module_services s={{read_value,&reads},{now,&io},{sink,&io}};struct ds_module_registry registry={0};ds_registry_init(&registry);
 for(unsigned cut=1;cut<=4;++cut){struct ds_module_lease out={NULL,19,23};CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],cut,&out)==DS_FAILED);CHECK(out.owner==NULL && out.slot==19 && out.generation==23);CHECK(owners[0].strong==1 && owners[1].strong==1);CHECK(!registry.entries[0].live && registry.entries[0].generation==1);}
 struct ow_shared dead={0};struct ds_module_lease out={NULL,19,23};CHECK(ds_lease_open(&registry,s,&owners[0],&dead,0,&out)==DS_FAILED);CHECK(owners[0].strong==1 && out.owner==NULL);
 owners[1].strong=UINT_MAX;CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&out)==DS_FAILED);CHECK(owners[0].strong==1 && owners[1].strong==UINT_MAX);owners[1].strong=1;
 struct ds_module *raw1=NULL,*raw2=NULL;CHECK(ds_module_open(s.input,&raw1)==DS_OK);CHECK(ds_module_open(s.input,&raw2)==DS_OK);CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&out)==DS_FULL);CHECK(owners[0].strong==1 && owners[1].strong==1);ds_module_close(raw2);ds_module_close(raw1);
 struct ds_module_lease first={0},second={0};CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&first)==DS_OK);CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&second)==DS_OK);CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&out)==DS_FULL && out.owner==NULL);CHECK(owners[0].strong==3 && owners[1].strong==3);
 int value=91;CHECK(ds_lease_stamp(first,&value)==DS_OK && value==23 && io.clock_calls==1 && io.sink_calls==0 && reads.calls==0);CHECK(ds_lease_emit(first,41)==DS_OK && io.last==41 && io.sink_calls==1 && reads.calls==0 && io.clock_calls==1);CHECK(ds_lease_twice(first,&value)==DS_OK && value==15 && reads.calls==2 && io.sink_calls==1 && io.clock_calls==1);
 value=91;io.clock_fail=1;CHECK(ds_lease_stamp(first,&value)==DS_FAILED && value==91);io.sink_fail=1;CHECK(ds_lease_emit(first,17)==DS_FAILED && io.last==17 && io.sink_calls==2);
 CHECK(ds_lease_close(first)==DS_OK && owners[0].strong==2 && owners[1].strong==2);CHECK(ds_lease_twice(first,&value)==DS_STALE && value==91);struct ds_module_lease next={0};CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&next)==DS_OK);CHECK(next.slot==first.slot && next.generation!=first.generation);CHECK(ds_lease_twice(first,&value)==DS_STALE && value==91);CHECK(ds_lease_close(first)==DS_STALE);CHECK(ds_lease_close(next)==DS_OK);CHECK(ds_lease_close(second)==DS_OK);
 /* Exhaust both registry slots without reinitializing identity. */
 for(unsigned i=0;i<3;++i){CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&next)==DS_OK);CHECK(ds_lease_close(next)==DS_OK);}
 CHECK(ds_lease_open(&registry,s,&owners[0],&owners[1],0,&out)==DS_FULL);CHECK(owners[0].strong==1 && owners[1].strong==1);
 struct ds_module_registry alias={0};ds_registry_init(&alias);CHECK(ds_lease_open(&alias,s,&owners[0],&owners[0],0,&next)==DS_OK);CHECK(owners[0].strong==3);CHECK(ds_lease_close(next)==DS_OK && owners[0].strong==1);
 struct ds_module_registry final={0};ds_registry_init(&final);CHECK(ds_lease_open(&final,s,&owners[0],&owners[1],0,&next)==DS_OK);CHECK(ow_shared_drop(&owners[0])==OW_OK && ow_shared_drop(&owners[1])==OW_OK);CHECK(ds_lease_close(next)==DS_OK);CHECK(destroy_count==2 && destruction[0]==2 && destruction[1]==1 && released==2);
 return 0;
}
static int correlated(void){struct ds_user_map users={0};struct ds_metric_map metrics={0};struct ds_user_value user={91};struct ds_metric_value metric={92};CHECK(ds_user_get(&users,(struct ds_user_key){0},&user)==DS_EMPTY && user.quota==91);CHECK(ds_metric_get(&metrics,(struct ds_metric_key){0},&metric)==DS_EMPTY && metric.count==92);
 for(unsigned k=0;k<4;++k)for(int v=-4;v<=4;++v){CHECK(ds_user_put(&users,(struct ds_user_key){k},(struct ds_user_value){v})==DS_OK);CHECK(ds_metric_put(&metrics,(struct ds_metric_key){k},(struct ds_metric_value){-v})==DS_OK);CHECK(ds_user_get(&users,(struct ds_user_key){k},&user)==DS_OK && user.quota==v);CHECK(ds_metric_get(&metrics,(struct ds_metric_key){k},&metric)==DS_OK && metric.count==-v);}
 CHECK(ds_user_put(&users,(struct ds_user_key){4},(struct ds_user_value){9})==DS_INVALID);CHECK(ds_metric_put(&metrics,(struct ds_metric_key){4},(struct ds_metric_value){9})==DS_INVALID);return 0;
}
int main(int argc,char **argv){if(argc<2)return 2;test=argv[1];if(!strcmp(test,"oracle"))return raw(1);if(!strcmp(test,"module_second")||!strcmp(test,"shared_modules")){CHECK(raw(0)==0);CHECK(leases()==0);return correlated();}return 2;}
