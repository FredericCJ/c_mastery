#include "dispatch_laws.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test;
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DS_CHECK:%s:%d:%s\n",test,__LINE__,#x);exit(1);}}while(0)
typedef struct {unsigned char bytes[16];size_t n;unsigned fail,wrong,zero_terminated,length_calls,copy_calls;} Source;
static int source_length(const void *p,size_t *out){Source *s=(Source *)p;++s->length_calls;*out=s->n;if(s->zero_terminated){*out=0;while(*out<s->n && s->bytes[*out])++*out;}return s->fail==1?DS_FAILED:DS_OK;}
static int source_copy(const void *p,unsigned char *out,size_t cap,size_t *n){Source *s=(Source *)p;++s->copy_calls;if(cap<s->n)return DS_FULL;for(size_t i=0;i<s->n;++i)out[i]=s->bytes[i];*n=s->wrong && s->n?s->n-1:s->n;return s->fail==2?DS_FAILED:DS_OK;}
static const DSCopyOps copy_ops={source_length,source_copy};
static int read_int(const void *p,int *out){*out=*(const int *)p;return DS_OK;}
static int read_fail(const void *p,int *out){(void)p;*out=123;return DS_FAILED;}
static const DSOps int_ops={read_int},fail_ops={read_fail};
static unsigned measure_calls;
static int counted_int(const void *p,int *out){++measure_calls;return read_int(p,out);}
static int counted_fail(const void *p,int *out){++measure_calls;return read_fail(p,out);}
static void laws(int emit){
 for(size_t n=0;n<=16;++n)for(unsigned cut=0;cut<3;++cut)for(unsigned wrong=0;wrong<2;++wrong){
  Source s={.n=n,.fail=cut,.wrong=wrong};for(size_t i=0;i<n;++i)s.bytes[i]=(unsigned char)(i%3?i:0);
  unsigned char out[16];memset(out,99,sizeof out);size_t length=91;int rc=ds_copy_checked((DSCopyView){&s,&copy_ops},out,16,&length);
  int expected=cut?DS_FAILED:wrong && n?DS_INVALID:DS_OK;CHECK(rc==expected && s.length_calls==1 && s.copy_calls==(cut==1?0u:1u));CHECK(length==(expected?91:n));
  unsigned sum=0;for(size_t i=0;i<16;++i){CHECK(out[i]==(!expected && i<n?s.bytes[i]:99));sum+=out[i];}
  if(emit)printf("L %zu %u %u %d %zu %u %u %u\n",n,cut,wrong,rc,length,s.length_calls,s.copy_calls,sum);
 }
 Source s={.bytes={1,0,2},.n=3,.zero_terminated=1};unsigned char out[4]={91,92,93,94};size_t n=97;
 CHECK(ds_copy_checked((DSCopyView){&s,&copy_ops},out,4,&n)==DS_FULL && n==97 && out[0]==91 && s.length_calls==1 && s.copy_calls==1);
 s=(Source){0};CHECK(ds_copy_checked((DSCopyView){&s,&copy_ops},NULL,0,&n)==DS_OK && n==0 && s.copy_calls==1);
 s.n=17;s.copy_calls=0;CHECK(ds_copy_checked((DSCopyView){&s,&copy_ops},out,4,&n)==DS_FULL && !s.copy_calls);
 const DSOps measured[2]={{counted_int},{counted_fail}};
 for(int value=-8;value<=8;++value)for(unsigned failure=0;failure<2;++failure){int result=91;measure_calls=0;int rc=ds_measure((DSView){&value,&measured[failure]},&result);CHECK(rc==(failure?DS_FAILED:DS_OK) && result==(failure?91:value) && measure_calls==1);if(emit)printf("V %d %u %d %d %u\n",value,failure,rc,result,measure_calls);}
}
static void subobjects(void){
 unsigned char bytes[16]={0};DSMulti owner={37,{bytes,3},{&int_ops,17}};DSView views[2];CHECK(ds_multi_views(&owner,views)==DS_OK);CHECK(views[0].object==&owner.bytes && views[1].object==&owner.scalar.value);
 for(size_t n=0;n<=16;++n)for(int value=-16;value<=16;++value){owner.bytes.length=n;owner.scalar.value=value;int a=91,b=92;CHECK(ds_measure(views[0],&a)==DS_OK && a==(int)n);CHECK(ds_measure(views[1],&b)==DS_OK && b==value);}
 owner.scalar.ops=&fail_ops;CHECK(ds_multi_views(&owner,views)==DS_OK);int out=91;for(int value=-16;value<=16;++value){owner.scalar.value=value;CHECK(ds_measure(views[1],&out)==DS_FAILED && out==91);}
 owner.bytes.bytes=NULL;owner.bytes.length=1;DSView before=views[0];CHECK(ds_multi_views(&owner,views)==DS_INVALID && views[0].object==before.object && views[0].ops==before.ops);
 struct Marker {unsigned char occupied;};CHECK(sizeof(struct Marker)>=1);
}
static unsigned drops;
static void drop(void *p){++*(unsigned *)p;}
static void admission(void){
 struct Self {int value;int *inside;};struct Self original={17,NULL};original.inside=&original.value;struct Self copied=original;CHECK(copied.inside==&original.value && copied.inside!=&copied.value);copied.inside=&copied.value;CHECK(*copied.inside==17);
 struct Aligned {_Alignas(32) int value;};CHECK(_Alignof(struct Aligned)==32);
 OwAllocator allocator={0};DSErased capture;ds_erased_init(&capture,ds_counted_bytes(&allocator));int32_t value=7;
 DSCaptureSpec specs[3]={{DS_CAPTURE_SELF_REFERENCE,_Alignof(int32_t),0},{DS_CAPTURE_TYPED_OBJECT,32,0},{DS_CAPTURE_I32,_Alignof(int32_t),1}};
 for(unsigned i=0;i<3;++i){CHECK(ds_capture_checked(&capture,specs[i],&value,1,drop,&drops)==DS_UNSUPPORTED && !capture.active && !allocator.attempts && !drops);}
 CHECK(ds_capture_checked(&capture,(DSCaptureSpec){DS_CAPTURE_I32,_Alignof(int32_t),0},&value,1,drop,&drops)==DS_OK);int out=91;CHECK(ds_erased_call(&capture,3,&out)==DS_OK && out==10);ds_erased_drop(&capture);CHECK(drops==1);
}
static void histories(int emit){
 for(unsigned mode=0;mode<3;++mode)for(int initial=-4;initial<=4;++initial)for(unsigned code=0;code<729;++code){
  DSClosure c;CHECK(ds_closure_init(&c,initial,(enum DSMode)mode)==DS_OK);int value=initial,output=91;unsigned active=1,script=code;
  for(unsigned step=0;step<3;++step){int arg=(int)(script%9)-4;script/=9;int expected=active?DS_OK:DS_EMPTY;if(active){output=value+arg;if(mode==DS_MUTABLE)value=output;if(mode==DS_ONCE)active=0;}int actual=91;int rc=ds_closure_call(&c,arg,&actual);CHECK(rc==expected && c.capture==value && c.active==active && actual==(expected?91:output));if(emit)printf("H %u %d %u %u %d %d %d %u\n",mode,initial,code,step,rc,actual,c.capture,c.active);}
 }
 for(unsigned mode=0;mode<3;++mode){DSClosure c;CHECK(ds_closure_init(&c,INT_MAX,(enum DSMode)mode)==DS_OK);int out=91;CHECK(ds_closure_call(&c,1,&out)==DS_RANGE && out==91 && c.capture==INT_MAX && c.active==(mode!=DS_ONCE));CHECK(ds_closure_init(&c,INT_MIN,(enum DSMode)mode)==DS_OK);CHECK(ds_closure_call(&c,-1,&out)==DS_RANGE && out==91 && c.capture==INT_MIN && c.active==(mode!=DS_ONCE));}
 DSClosure original;CHECK(ds_closure_init(&original,1,DS_ONCE)==DS_OK);DSClosure copy=original;int a=0,b=0;CHECK(ds_closure_call(&original,2,&a)==DS_OK && ds_closure_call(&copy,2,&b)==DS_OK && a==3 && b==3);
}
int main(int argc,char **argv){if(argc!=2)return 2;test=argv[1];if(!strcmp(test,"copy_laws"))laws(0);else if(!strcmp(test,"subobjects"))subobjects();else if(!strcmp(test,"capture_admission"))admission();else if(!strcmp(test,"closure_histories"))histories(0);else if(!strcmp(test,"oracle_laws"))laws(1);else if(!strcmp(test,"oracle_closures"))histories(1);else if(!strcmp(test,"layout"))printf("%zu %zu %zu %zu %zu %zu\n",sizeof(DSCopyView),sizeof(DSCopyOps),sizeof(DSMulti),offsetof(DSMulti,bytes),offsetof(DSMulti,scalar)+offsetof(DSBox,value),sizeof(DSCaptureSpec));else return 2;return 0;}
