#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTAINERS_ASSERT:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct{CDVector*owner;unsigned calls,live,fail;size_t expected_length;} Alloc;
static int*take(void*v,size_t count){Alloc*a=v;CHECK(a->owner->length==a->expected_length&&a->owner->length<=a->owner->capacity);++a->calls;if(a->fail==a->calls)return NULL;int*p=malloc(count*sizeof*p);if(p)++a->live;return p;}
static void release(void*v,int*p){Alloc*a=v;CHECK(a->live);--a->live;free(p);}
typedef struct{unsigned calls,fail,live,destroyed;} Cloner;
static int clone(void*v,const CDPayload*in,CDPayload*out){Cloner*c=v;++c->calls;if(c->calls==c->fail)return 1;out->value=in->value;out->resource=c->calls;c->live|=1u<<out->resource;return 0;}
static void destroy(void*v,CDPayload*p){Cloner*c=v;CHECK(c->live&(1u<<p->resource));c->live&=~(1u<<p->resource);++c->destroyed;p->resource=0;}
int main(void){
 int data[5]={10,20,30,0,77},old=0;CDArray a;CHECK(cd_array_init(&a,data,4)==CD_OK);a.length=3;int value=15;
 CHECK(cd_array_insert(&a,1,&value)==CD_OK&&a.length==4&&data[1]==15&&data[3]==30&&data[4]==77);
 CHECK(cd_array_erase(&a,2,&old)==CD_OK&&old==20&&a.length==3&&data[2]==30);
 CHECK(cd_array_get(&a,a.length,&old)==CD_INVALID);CHECK(cd_array_insert(&a,a.length+1,&value)==CD_INVALID);
 data[0]=1;data[1]=2;data[2]=3;CHECK(cd_array_insert(&a,0,&data[1])==CD_OK&&data[0]==2&&data[1]==1&&data[2]==2&&data[3]==3);
 int snapshot[4];memcpy(snapshot,data,sizeof snapshot);CHECK(cd_array_insert(&a,0,&value)==CD_FULL&&!memcmp(snapshot,data,sizeof snapshot));
 /* Erase-current keeps the same logical index for the successor. */
 for(size_t at=0;at<a.length;){CHECK(cd_array_erase(&a,at,&old)==CD_OK);}CHECK(a.length==0);
 CHECK(cd_array_destroy(&a)==CD_OK&&cd_array_destroy(&a)==CD_INVALID&&cd_array_insert(&a,0,&value)==CD_INVALID);
 CHECK(cd_array_init(&a,NULL,0)==CD_OK&&cd_array_insert(&a,0,&value)==CD_FULL&&cd_array_get(&a,0,&old)==CD_INVALID);
 CHECK(cd_array_init(&a,data,3)==CD_OK);for(int i=1;i<=3;++i){CHECK(cd_array_insert(&a,a.length,&i)==CD_OK);}int fourth=4;CHECK(cd_array_insert(&a,a.length,&fourth)==CD_FULL&&a.length==3&&data[0]==1&&data[2]==3&&cd_array_destroy(&a)==CD_OK);
 CHECK(cd_array_init(&a,data,4)==CD_OK);data[0]=1;data[1]=2;data[2]=3;a.length=3;int nine=9;CHECK(cd_array_insert(&a,0,&nine)==CD_OK&&data[0]==9&&data[1]==1&&data[2]==2&&data[3]==3);CHECK(cd_array_erase(&a,0,&old)==CD_OK&&old==9&&data[0]==1&&data[2]==3);
 int source[5]={0,1,2,3,4};CDView view,sub;CHECK(cd_view_init(&view,source,5,0,3,2)==CD_OK&&cd_view_get(&view,2,&old)==CD_OK&&old==4);
 CHECK(cd_view_sub(&view,1,2,&sub)==CD_OK&&cd_view_get(&sub,0,&old)==CD_OK&&old==2);
 CHECK(cd_view_init(&view,source,5,4,3,-2)==CD_OK);for(size_t i=0;i<3;++i){CHECK(cd_view_get(&view,i,&old)==CD_OK&&old==4-(int)(2*i));}
 CHECK(cd_view_sub(&view,3,0,&sub)==CD_OK&&cd_view_get(&sub,0,&old)==CD_INVALID);
 CHECK(cd_view_init(&view,source,5,0,2,-1)==CD_INVALID&&cd_view_init(&view,source,5,0,SIZE_MAX,2)==CD_INVALID);
 CDRing ring;int ring_storage[5]={0};CDSpans spans;size_t written=99;CHECK(cd_ring_init(&ring,ring_storage,5)==CD_OK);
 ring.head=4;ring.count=3;ring_storage[4]=4;ring_storage[0]=7;ring_storage[1]=9;
 CHECK(cd_ring_read(&ring,&spans)==CD_OK&&spans.first_count==1&&spans.second_count==2&&spans.first[0]==4&&spans.second[0]==7);
 int output[5]={0};CHECK(cd_ring_pop(&ring,output,2)==CD_OK&&output[0]==4&&output[1]==7);
 int input[3]={8,9,10};CHECK(cd_ring_push(&ring,input,3,0,&written)==CD_OK&&written==3&&ring.count==4);
 CHECK(cd_ring_push(&ring,input,2,0,&written)==CD_FULL&&ring.count==4&&written==3);
 CHECK(cd_ring_push(&ring,input,2,1,&written)==CD_OK&&written==1&&ring.count==5);
 CHECK(cd_ring_pop(&ring,output,5)==CD_OK&&output[0]==9&&output[1]==8&&output[2]==9&&output[3]==10&&output[4]==8);
 CHECK(cd_ring_reserve(&ring,2,&spans)==CD_OK);CHECK(cd_ring_commit(&ring,3)==CD_INVALID&&ring.count==0&&ring.reserving);
 spans.first[0]=40;if(spans.first_count>1)spans.first[1]=41;else spans.second[0]=41;
 CHECK(cd_ring_read(&ring,&spans)==CD_BUSY&&cd_ring_commit(&ring,2)==CD_OK&&cd_ring_pop(&ring,output,2)==CD_OK&&output[0]==40&&output[1]==41);
 CHECK(cd_ring_init(&ring,NULL,0)==CD_OK&&cd_ring_read(&ring,&spans)==CD_OK&&spans.first==NULL&&cd_ring_reserve(&ring,0,&spans)==CD_OK&&cd_ring_commit(&ring,0)==CD_OK);
 Alloc allocator={0};CDVector vector;allocator.owner=&vector;CHECK(cd_vector_init(&vector,(CDAllocator){&allocator,take,release})==CD_OK);
 for(int i=1;i<=8;++i){allocator.expected_length=vector.length;CHECK(cd_vector_insert(&vector,vector.length,i)==CD_OK);}
 CHECK(vector.length==8&&vector.capacity==8&&vector.copies==7&&allocator.live==1);
 allocator.expected_length=8;allocator.fail=allocator.calls+1;int*live_before=vector.data;
 CHECK(cd_vector_insert(&vector,3,90)==CD_NOMEM&&vector.data==live_before&&vector.length==8&&vector.capacity==8&&allocator.live==1);for(unsigned i=0;i<8;++i)CHECK(vector.data[i]==(int)i+1);
 allocator.fail=0;CHECK(cd_vector_insert(&vector,3,90)==CD_OK&&vector.capacity==16&&vector.data[3]==90);
 allocator.expected_length=vector.length;CHECK(cd_vector_reserve(&vector,4)==CD_OK&&vector.capacity==16);
 allocator.fail=allocator.calls+1;CHECK(cd_vector_shrink(&vector)==CD_NOMEM&&vector.capacity==16&&vector.length==9);
 allocator.fail=0;CHECK(cd_vector_shrink(&vector)==CD_OK&&vector.capacity==9);
 while(vector.length){CHECK(cd_vector_erase(&vector,0,&old)==CD_OK);}allocator.expected_length=0;CHECK(cd_vector_shrink(&vector)==CD_OK&&vector.data==NULL&&allocator.live==0&&cd_vector_destroy(&vector)==CD_OK);
 allocator=(Alloc){&vector,0,0,0,0};CHECK(cd_vector_init(&vector,(CDAllocator){&allocator,take,release})==CD_OK);for(int i=1;i<=5;++i){allocator.expected_length=vector.length;CHECK(cd_vector_insert(&vector,vector.length,i)==CD_OK);}CHECK(vector.capacity==8&&vector.copies==7);allocator.expected_length=5;CHECK(cd_vector_reserve(&vector,4)==CD_OK&&vector.capacity==8&&cd_vector_shrink(&vector)==CD_OK&&vector.capacity==5&&vector.copies==12&&cd_vector_destroy(&vector)==CD_OK&&!allocator.live);
 CDPayload inputs[4]={{1,0},{2,0},{3,0},{4,0}},destination[4]={{99,0},{99,0},{99,0},{99,0}};
 for(unsigned failure=1;failure<=4;++failure){Cloner cloner={0,failure,0,0};CHECK(cd_clone_batch(destination,inputs,4,(CDPayloadOps){&cloner,clone,destroy})==CD_FAILED&&cloner.live==0&&cloner.destroyed==failure-1);for(unsigned i=0;i<4;++i)CHECK(destination[i].value==99);}
 Cloner cloner={0};CHECK(cd_clone_batch(destination,inputs,4,(CDPayloadOps){&cloner,clone,destroy})==CD_OK);for(unsigned i=0;i<4;++i){CHECK(destination[i].value==(int)i+1);destroy(&cloner,&destination[i]);}CHECK(cloner.live==0);
 puts("CONTAINERS_SEQUENCE_PASS array view ring vector prefix rollback");return 0;
}
