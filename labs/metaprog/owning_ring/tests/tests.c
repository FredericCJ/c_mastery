#include "owning_ring.h"
#include "metaprog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
static void *allocated[32];
static void cleanup(void){for(unsigned i=0;i<32;i++)if(allocated[i]){free(allocated[i]);allocated[i]=NULL;}}
static void *allocate(size_t n){void *p=malloc(n?n:1);if(!p)return NULL;for(unsigned i=0;i<32;i++)if(!allocated[i]){allocated[i]=p;return p;}free(p);return NULL;}
static void release(void *p){if(!p)return;for(unsigned i=0;i<32;i++)if(allocated[i]==p){free(p);allocated[i]=NULL;return;}abort();}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MPO_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);cleanup();return 1;}}while(0)
typedef struct { unsigned step,fail,live,drop_calls,n;uint32_t trace[128]; } Context;
typedef struct { unsigned char *first,*second; } Twin;
typedef struct { void *context;int (*clone)(void *,const Twin *,Twin *);void (*drop)(void *,Twin *); } TwinOps;
typedef struct { Twin data[4];size_t head,count;TwinOps ops; } TwinRing;
#ifndef MPO_MUTATION
#define MPO_MUTATION 0
#endif
#define MPO_PREFIX twin
#define MPO_RING TwinRing
#define MPO_ELEMENT Twin
#define MPO_OPS TwinOps
#include "ring_template.inc"
static int twin_clone(void *context,const Twin *in,Twin *out){
 Context *c=context;
 if(++c->step==c->fail)return 1;
 out->first=allocate(1);if(!out->first)return 1;c->live++;*out->first=*in->first;
 if(++c->step==c->fail)return 1;
 if(++c->step==c->fail)return 1;
 out->second=allocate(1);if(!out->second)return 1;c->live++;*out->second=*in->second;
 return ++c->step==c->fail;
}
static void twin_drop(void *context,Twin *value){Context *c=context;c->drop_calls++;if(value->second){release(value->second);c->live--;}if(value->first){release(value->first);c->live--;}*value=(Twin){0};}
static int clone_blob(void *context,const MpoBlob *in,MpoBlob *out){Context *c=context;if(++c->step==c->fail)return 1;unsigned char *p=allocate(in->length);if(!p)return 1;for(size_t i=0;i<in->length;i++)p[i]=in->bytes[i];*out=(MpoBlob){p,in->length};c->live++;return ++c->step==c->fail;}
static void drop_blob(void *context,MpoBlob *value){Context *c=context;c->drop_calls++;if(value->bytes){c->trace[c->n++%128]=value->length?value->bytes[0]:0;c->live--;release(value->bytes);}*value=(MpoBlob){0};}
static int clone_words(void *context,const MpoWords *in,MpoWords *out){Context *c=context;if(++c->step==c->fail)return 1;uint32_t *p=allocate(in->count*sizeof *p);if(!p)return 1;for(size_t i=0;i<in->count;i++)p[i]=in->values[i];*out=(MpoWords){p,in->count};c->live++;return ++c->step==c->fail;}
static void drop_words(void *context,MpoWords *value){Context *c=context;c->drop_calls++;if(value->values){c->trace[c->n++%128]=value->count?value->values[0]:0;c->live--;release(value->values);}*value=(MpoWords){0};}
static int erased_clone(void *c,const void *in,void *out){return clone_blob(c,in,out);}
static void erased_drop(void *c,void *value){drop_blob(c,value);}
static void erased_move(void *c,void *destination,void *source){(void)c;MpoBlob *dst=destination,*src=source;*dst=*src;*src=(MpoBlob){0};}
static int ordinary(void){
 MPU32Ring u={0};MPPairRing p={0};for(unsigned i=0;i<4;i++){CHECK(mp_u32_push(&u,i)&&mp_pair_push(&p,(MPPair){i,i+9}));}
 uint32_t x=99;MPPair pair={99,99};CHECK(mp_u32_pop(&u,&x)&&x==0&&mp_pair_pop(&p,&pair)&&pair.x==0&&pair.y==9);
 CHECK(mp_u32_push(&u,4)&&mp_pair_push(&p,(MPPair){4,13}));for(unsigned i=1;i<5;i++)CHECK(mp_u32_pop(&u,&x)&&x==i&&mp_pair_pop(&p,&pair)&&pair.x==i&&pair.y==i+9);
 x=99;CHECK(!mp_u32_pop(&u,&x)&&x==99);puts("MPO_BASE actual-two-instances wrap FIFO fields");return 0;
}
static int failures(void){
 unsigned char byte=7;MpoBlob input={&byte,1};
 for(unsigned fail=1;fail<=2;fail++){Context c={0};MpoBlobRing r;CHECK(mpo_blob_init(&r,(MpoBlobOps){&c,clone_blob,drop_blob})==MPO_OK);c.fail=fail;
  CHECK(mpo_blob_push(&r,&input)==MPO_CLONE&&r.count==0&&r.head==0&&c.live==0&&c.drop_calls==1&&input.bytes==&byte&&byte==7);
  for(unsigned i=0;i<4;i++)CHECK(r.data[i].bytes==NULL&&r.data[i].length==0);
  c.fail=0;CHECK(mpo_blob_push(&r,&input)==MPO_OK&&r.count==1&&c.live==1);mpo_blob_clear(&r);CHECK(c.live==0);
 }
 Context c={0};MpoBlobRing r;CHECK(mpo_blob_init(&r,(MpoBlobOps){&c,clone_blob,drop_blob})==MPO_OK);for(unsigned i=0;i<4;i++)CHECK(mpo_blob_push(&r,&input)==MPO_OK);
 unsigned steps=c.step,drops=c.drop_calls;CHECK(mpo_blob_push(&r,&input)==MPO_FULL&&c.step==steps&&c.drop_calls==drops&&r.count==4);mpo_blob_clear(&r);CHECK(c.live==0);
 puts("MPO_FAILURE before-and-after-allocation rollback cleanup full-no-effects");return 0;
}
static int transfer(void){
 Context c={0};MpoBlobRing r;CHECK(mpo_blob_init(&r,(MpoBlobOps){&c,clone_blob,drop_blob})==MPO_OK);MpoBlob out={NULL,99};CHECK(mpo_blob_pop(&r,&out)==MPO_EMPTY&&out.length==99&&out.bytes==NULL);
 for(unsigned char i=1;i<=4;i++){MpoBlob in={&i,1};CHECK(mpo_blob_push(&r,&in)==MPO_OK);}
 CHECK(mpo_blob_pop(&r,&out)==MPO_OK&&out.bytes[0]==1&&c.live==4);drop_blob(&c,&out);
 unsigned char five=5;MpoBlob in={&five,1};CHECK(mpo_blob_push(&r,&in)==MPO_OK);mpo_blob_clear(&r);CHECK(c.live==0&&c.n==5);for(unsigned i=0;i<5;i++)CHECK(c.trace[i]==i+1);
 unsigned drops=c.drop_calls;mpo_blob_clear(&r);CHECK(c.drop_calls==drops&&r.count==0&&r.head==1);
 Context w={0};MpoWordsRing words;uint32_t data[]={UINT32_MAX,7};MpoWords source={data,2},result={0};CHECK(mpo_words_init(&words,(MpoWordsOps){&w,clone_words,drop_words})==MPO_OK&&mpo_words_push(&words,&source)==MPO_OK);
 data[1]=99;CHECK(mpo_words_pop(&words,&result)==MPO_OK&&result.count==2&&result.values[0]==UINT32_MAX&&result.values[1]==7);drop_words(&w,&result);CHECK(w.live==0);
 unsigned char first=11,second=22;Twin borrowed={&first,&second};
 for(unsigned fail=1;fail<=4;fail++){Context tc={0};TwinRing tr;CHECK(twin_init(&tr,(TwinOps){&tc,twin_clone,twin_drop})==MPO_OK);tc.fail=fail;
  CHECK(twin_push(&tr,&borrowed)==MPO_CLONE&&tr.count==0&&tc.live==0&&tc.drop_calls==1);
  tc.fail=0;Twin got={0};CHECK(twin_push(&tr,&borrowed)==MPO_OK&&tc.live==2&&twin_pop(&tr,&got)==MPO_OK&&*got.first==11&&*got.second==22);twin_drop(&tc,&got);CHECK(tc.live==0);
 }
 puts("MPO_TRANSFER two-owning-types FIFO-drop empty-output unique-move");return 0;
}
static int erased(void){
 Context c={0};MpoBlob storage[5]={{0}},out={0};void *slots[4]={storage,storage+1,storage+2,storage+3};MpoErasedRing r;
 CHECK(mpo_erased_init(&r,slots,storage+4,(MpoErasedOps){&c,erased_clone,erased_drop,erased_move})==MPO_OK);
 for(unsigned char i=1;i<=4;i++){MpoBlob in={&i,1};CHECK(mpo_erased_push(&r,&in)==MPO_OK);}
 unsigned steps=c.step,drops=c.drop_calls;unsigned char five=5;MpoBlob input={&five,1};CHECK(mpo_erased_push(&r,&input)==MPO_FULL&&c.step==steps&&c.drop_calls==drops);
 for(unsigned i=1;i<=4;i++){CHECK(mpo_erased_pop(&r,&out)==MPO_OK&&out.length==1&&out.bytes&&out.bytes[0]==i);drop_blob(&c,&out);}
 CHECK(c.live==0&&r.count==0&&storage[4].bytes==NULL);
 c.fail=c.step+2;CHECK(mpo_erased_push(&r,&input)==MPO_CLONE&&c.live==0&&r.count==0&&storage[4].bytes==NULL);
 puts("MPO_ERASED actual-typed-storage scratch cleanup equivalent-effects");return 0;
}
static uint64_t trace_hash(const Context *c){uint64_t h=0;for(unsigned i=0;i<c->n;i++)h=h*257+c->trace[i];return h;}
static int histories(void){
 char line[128];while(fgets(line,sizeof line,stdin)){
  size_t n=strcspn(line,"\r\n");CHECK(n<=32);
  for(unsigned mode=0;mode<3;mode++){
   Context c={0};MpoBlobRing typed;MpoErasedRing erased_ring;MpoBlob storage[5]={{0}};void *slots[4]={storage,storage+1,storage+2,storage+3};
   CHECK((mode==0?mpo_blob_init(&typed,(MpoBlobOps){&c,clone_blob,drop_blob}):mode==1?mpo_manual_init(&typed,(MpoBlobOps){&c,clone_blob,drop_blob}):mpo_erased_init(&erased_ring,slots,storage+4,(MpoErasedOps){&c,erased_clone,erased_drop,erased_move}))==MPO_OK);
   printf("[");
   for(size_t i=0;i<n;i++){
    unsigned char value=line[i]=='B'?2:1;MpoBlob in={&value,1},out={NULL,99};int status=MPO_OK;unsigned result=99;
    c.fail=line[i]=='F'?c.step+1:line[i]=='G'?c.step+2:0;
    if(line[i]=='P'){
     status=mode==0?mpo_blob_pop(&typed,&out):mode==1?mpo_manual_pop(&typed,&out):mpo_erased_pop(&erased_ring,&out);
     if(status==MPO_OK){CHECK(out.bytes&&out.length==1);result=out.bytes[0];drop_blob(&c,&out);}else CHECK(out.bytes==NULL&&out.length==99);
    }else if(line[i]=='C'){if(mode==0)mpo_blob_clear(&typed);else if(mode==1)mpo_manual_clear(&typed);else mpo_erased_clear(&erased_ring);}
    else status=mode==0?mpo_blob_push(&typed,&in):mode==1?mpo_manual_push(&typed,&in):mpo_erased_push(&erased_ring,&in);
    size_t head=mode<2?typed.head:erased_ring.head,count=mode<2?typed.count:erased_ring.count;
    CHECK(head<4&&count<=4&&c.live==count);
    printf("%s[%d,%zu,%zu,%u,%u,%u,%u,%" PRIu64 "]",i?",":"",status,head,count,c.live,c.step,c.drop_calls,result,trace_hash(&c));
   }
   puts("]");if(mode==0)mpo_blob_clear(&typed);else if(mode==1)mpo_manual_clear(&typed);else mpo_erased_clear(&erased_ring);CHECK(c.live==0);
  }
 }
 return ferror(stdin)?1:0;
}
static int metrics(void){
 double elapsed[3][7];uint64_t checksum[3]={0};const unsigned iterations=10000;
 for(unsigned mode=0;mode<3;mode++)for(unsigned sample=0;sample<7;sample++){
  Context c={0};MpoBlobRing typed;MpoErasedRing erased_ring;MpoBlob storage[5]={{0}};void *slots[4]={storage,storage+1,storage+2,storage+3};
  CHECK((mode==0?mpo_blob_init(&typed,(MpoBlobOps){&c,clone_blob,drop_blob}):mode==1?mpo_manual_init(&typed,(MpoBlobOps){&c,clone_blob,drop_blob}):mpo_erased_init(&erased_ring,slots,storage+4,(MpoErasedOps){&c,erased_clone,erased_drop,erased_move}))==MPO_OK);
  clock_t start=clock();uint64_t sum=0;
  for(unsigned i=0;i<iterations;i++){
   for(unsigned char j=1;j<=4;j++){MpoBlob in={&j,1};CHECK((mode==0?mpo_blob_push(&typed,&in):mode==1?mpo_manual_push(&typed,&in):mpo_erased_push(&erased_ring,&in))==MPO_OK);}
   for(unsigned j=0;j<4;j++){MpoBlob out={0};CHECK((mode==0?mpo_blob_pop(&typed,&out):mode==1?mpo_manual_pop(&typed,&out):mpo_erased_pop(&erased_ring,&out))==MPO_OK);sum+=out.bytes[0];drop_blob(&c,&out);}
  }
  elapsed[mode][sample]=(double)(clock()-start)/CLOCKS_PER_SEC;CHECK(sum==100000&&c.live==0&&c.drop_calls==iterations*4);checksum[mode]=sum;
 }
 printf("{\"iterations\":%u,\"checksum\":%" PRIu64 ",\"typed_size\":%zu,\"erased_control_size\":%zu,\"erased_storage_size\":%zu,\"element_size\":%zu,\"element_alignment\":%zu",iterations,checksum[0],sizeof(MpoBlobRing),sizeof(MpoErasedRing),sizeof(MpoBlob)*5,sizeof(MpoBlob),_Alignof(MpoBlob));
 const char *names[]={"typed_seconds","manual_seconds","erased_seconds"};for(unsigned m=0;m<3;m++){CHECK(checksum[m]==checksum[0]);printf(",\"%s\":[",names[m]);for(unsigned s=0;s<7;s++)printf("%s%.9f",s?",":"",elapsed[m][s]);printf("]");}puts("}");return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"mono_base"))return ordinary();
 if(!strcmp(argv[1],"owning_failure"))return failures();
 if(!strcmp(argv[1],"owning_transfer"))return transfer();
 if(!strcmp(argv[1],"owning_erased"))return erased();
 if(!strcmp(argv[1],"histories"))return histories();
 if(!strcmp(argv[1],"metrics"))return metrics();
 return 2;
}
