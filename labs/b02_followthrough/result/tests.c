#include "result.h"
#include "old_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BF_CHECK %d %s\n",__LINE__,#x);exit(1);}}while(0)
static void render(void){
 unsigned char out[258];size_t w=777;struct b2_result r={0};CHECK(bf_number(0,0,&r)==B2_OK);
 r.tag=0;r.value.number=-123;memset(out,0xa5,sizeof out);CHECK(b2_render(&r,out,3,&w)==B2_FULL&&w==777&&out[0]==0xa5);
 CHECK(b2_render(&r,out,4,&w)==B2_OK&&w==4&&memcmp(out,"-123",4)==0&&out[4]==0xa5);
 for(int tag=0;tag<=2;tag+=2){r.tag=(unsigned)tag;if(tag==0)r.value.number=10000;else r.value.error=-10000;w=777;memset(out,0xa5,sizeof out);CHECK(b2_render(&r,out,6,&w)==B2_RANGE&&w==777&&out[0]==0xa5);}
 r.tag=9;w=777;CHECK(b2_render(&r,out,6,&w)==B2_INVALID&&w==777&&out[0]==0xa5);
 unsigned char all[256];for(unsigned i=0;i<256;++i)all[i]=(unsigned char)i;r.tag=1;r.value.text.data=all;r.value.text.length=256;
 CHECK(b2_render(&r,out,256,&w)==B2_OK&&w==256&&memcmp(out,all,256)==0&&out[256]==0xa5);
 r.value.text.data=NULL;r.value.text.length=0;CHECK(b2_render(&r,NULL,0,&w)==B2_OK&&w==0);r.value.text.length=1;w=777;CHECK(b2_render(&r,out,6,&w)==B2_INVALID&&w==777);
}
static void construct(void){
 struct b2_result r;CHECK(bf_number(-17,0,&r)==B2_OK&&r.tag==0);CHECK(r.value.number==-17);CHECK(bf_number(7,2,&r)==B2_OK&&r.tag==2);CHECK(r.value.error==7);
 CHECK(bf_number(10000,0,&r)==B2_RANGE&&r.tag==2&&r.value.error==7);CHECK(bf_number(1,1,&r)==B2_INVALID&&r.tag==2);
 unsigned char data[]={97,0,98};CHECK(bf_text(data,3,&r)==B2_OK&&r.tag==1&&r.value.text.data==data&&r.value.text.length==3);CHECK(bf_text(NULL,1,&r)==B2_INVALID&&r.value.text.data==data);
 CHECK(bf_text(NULL,0,&r)==B2_OK&&r.value.text.length==0);CHECK(bf_text(data,3,NULL)==B2_INVALID);
}
struct bf_memory{unsigned calls,releases,live,fail;unsigned char bytes[256];};
static void*allocate(void*ctx,size_t n){struct bf_memory*m=ctx;CHECK(n&&n<=256);++m->calls;if(m->fail)return NULL;CHECK(!m->live);m->live=1;return m->bytes;}
static void release(void*ctx,void*p){struct bf_memory*m=ctx;CHECK(p==m->bytes&&m->live);m->live=0;++m->releases;}
static void*heap_alloc(void*ctx,size_t n){++*(unsigned*)ctx;return malloc(n);}
static void heap_free(void*ctx,void*p){--*(unsigned*)ctx;free(p);}
static void owning(void){
 struct bf_memory m={0};struct bf_value a,b;bf_value_init(&a);bf_value_init(&b);unsigned char bytes[]={97,0,98},out[3]={0};size_t n=99;
 m.fail=1;CHECK(bf_clone(&a,bytes,3,allocate,release,&m)==B2_FAILED&&a.tag==BF_EMPTY&&!m.live);m.fail=0;
 CHECK(bf_clone(&a,bytes,3,allocate,release,&m)==B2_OK&&a.tag==BF_OWNED&&m.live);bytes[0]=99;
 CHECK(bf_value_render(&a,out,3,&n)==B2_OK&&n==3&&out[0]==97&&out[1]==0&&out[2]==98);
 CHECK(bf_move(&b,&a)==B2_OK&&a.tag==BF_EMPTY&&b.tag==BF_OWNED);bf_reset(&a);CHECK(m.releases==0);
 struct bf_bytes take={0};CHECK(bf_take(&b,&take)==B2_OK&&b.tag==BF_EMPTY&&take.length==3);bf_reset(&b);CHECK(m.releases==0);bf_bytes_drop(&take);bf_bytes_drop(&take);CHECK(m.releases==1&&!m.live);
 CHECK(bf_take(&b,&take)==B2_INVALID);CHECK(bf_move(&a,&a)==B2_INVALID);CHECK(bf_clone(&a,NULL,0,allocate,release,&m)==B2_OK&&m.calls==2);bf_reset(&a);CHECK(m.releases==1);
 CHECK(bf_clone(&a,bytes,3,allocate,release,&m)==B2_OK);CHECK(bf_clone(&a,bytes,3,allocate,release,&m)==B2_INVALID&&m.calls==3);bf_reset(&a);CHECK(m.releases==2);
 unsigned live=0;CHECK(bf_clone(&a,bytes,3,heap_alloc,heap_free,&live)==B2_OK&&live==1);CHECK(bf_move(&b,&a)==B2_OK);CHECK(bf_take(&b,&take)==B2_OK);bf_bytes_drop(&take);CHECK(live==0);
}
static void wire(void){
 struct bf_wire r={0},decoded={0};r.version=1;r.tag=0;r.number=-9999;unsigned char bytes[260],out[260];size_t n=777,w=888;
 memset(bytes,0xa5,sizeof bytes);CHECK(bf_encode(&r,bytes,7,&n)==B2_FULL&&n==777&&bytes[0]==0xa5);CHECK(bf_encode(&r,bytes,8,&n)==B2_OK&&n==8);CHECK(bf_decode(bytes,n,&decoded)==B2_OK&&decoded.number==-9999);
 CHECK(bf_old_render_wire(bytes,n,out,5,&w)==B2_OK&&w==5&&memcmp(out,"-9999",5)==0);
 r.version=2;r.tag=3;r.boolean=1;CHECK(bf_encode(&r,bytes,260,&n)==B2_OK&&n==5);CHECK(bf_decode(bytes,n,&decoded)==B2_OK&&decoded.boolean==1&&decoded.tag==3);
 w=888;memset(out,0xa5,sizeof out);CHECK(bf_old_render_wire(bytes,n,out,260,&w)==B2_INVALID&&w==888&&out[0]==0xa5);
 bytes[4]=2;decoded.number=777;CHECK(bf_decode(bytes,n,&decoded)==B2_INVALID&&decoded.number==777);bytes[0]=1;bytes[4]=1;CHECK(bf_decode(bytes,n,&decoded)==B2_INVALID&&decoded.number==777);CHECK(bf_old_render_wire(bytes,n,out,260,&w)==B2_INVALID&&w==888);
 for(unsigned tag=4;tag<256;++tag){bytes[0]=2;bytes[1]=(unsigned char)tag;CHECK(bf_decode(bytes,n,&decoded)==B2_INVALID&&decoded.number==777);}
 r.version=2;r.tag=1;r.length=256;for(unsigned i=0;i<256;++i)r.bytes[i]=(unsigned char)i;CHECK(bf_encode(&r,bytes,260,&n)==B2_OK&&n==260);CHECK(bf_decode(bytes,n,&decoded)==B2_OK&&decoded.length==256&&memcmp(r.bytes,decoded.bytes,256)==0);
 for(size_t i=0;i<n;++i){decoded.number=777;CHECK(bf_decode(bytes,i,&decoded)==B2_INVALID&&decoded.number==777);}
}
int main(int argc,char**argv){
 CHECK(argc==2);
 if(!strcmp(argv[1],"numeric")){unsigned tag;int value;size_t cap;while(scanf("%u %d %zu",&tag,&value,&cap)==3){CHECK(cap<=6);struct b2_result r;CHECK(bf_number(value,tag,&r)==B2_OK);unsigned char out[8];memset(out,0xa5,sizeof out);size_t n=777;int status=b2_render(&r,out,cap,&n);printf("%d %zu ",status,n);for(unsigned i=0;i<8;++i)printf("%02x",out[i]);puts("");}return 0;}
 if(!strcmp(argv[1],"wire_oracle")){unsigned version,tag;int value;size_t length;while(scanf("%u %u %d %zu",&version,&tag,&value,&length)==4){CHECK(length<=256);struct bf_wire r={0},d={0};r.version=version;r.tag=tag;r.number=value;r.length=length;r.boolean=(unsigned)value;for(size_t i=0;i<length;++i)r.bytes[i]=(unsigned char)(i*37+11);unsigned char wirebytes[260],rendered[260];size_t n=0,w=777;CHECK(bf_encode(&r,wirebytes,260,&n)==B2_OK);CHECK(bf_decode(wirebytes,n,&d)==B2_OK);int old=bf_old_render_wire(wirebytes,n,rendered,260,&w);printf("%u %u %d %zu %u %d %zu ",d.version,d.tag,d.number,d.length,d.boolean,old,w);for(size_t i=0;i<n;++i)printf("%02x",wirebytes[i]);puts("");}return 0;}
 if(!strcmp(argv[1],"metrics")){printf("{\"result_bytes\":%zu,\"value_bytes\":%zu,\"wire_record_bytes\":%zu,\"max_text\":256,\"max_wire\":260}\n",sizeof(struct b2_result),sizeof(struct bf_value),sizeof(struct bf_wire));return 0;}
 if(!strcmp(argv[1],"render"))render();else if(!strcmp(argv[1],"construct"))construct();else if(!strcmp(argv[1],"owning"))owning();else if(!strcmp(argv[1],"wire"))wire();else CHECK(0);
 printf("BF %s validated\n",argv[1]);return 0;
}
