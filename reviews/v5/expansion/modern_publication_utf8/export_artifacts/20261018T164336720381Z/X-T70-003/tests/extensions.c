#include "dod_ext.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name="arguments";
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DOD_CHECK:%s:%d:%s\n",test_name,__LINE__,#x);exit(1);}}while(0)
static void layout(void) {
 for(size_t n=0;n<=8;++n)for(unsigned cold=0;cold<2;++cold){
  DRow rows[8],result[8];for(unsigned i=0;i<8;++i)rows[i]=(DRow){(int32_t)i-3,(int32_t)i%3-1,i+40};
  DColumns8 columns;DTiles tiles;CHECK(d_rows_to_columns(rows,n,&columns)==D_OK);
  CHECK(d_columns_to_rows(&columns,result,n)==D_OK);
  for(size_t i=0;i<n;++i)CHECK(result[i].x==rows[i].x&&result[i].cold==rows[i].cold);
  CHECK(d_rows_to_tiles(rows,n,cold,&tiles)==D_OK);
  CHECK(d_tiles_to_rows(&tiles,result,n)==D_OK);
  for(size_t i=0;i<n;++i)CHECK(result[i].x==rows[i].x&&result[i].velocity==rows[i].velocity&&result[i].cold==(cold?rows[i].cold:0));
  result[0]=(DRow){91,92,93};
  if(n){CHECK(d_tiles_to_rows(&tiles,result,n-1)==D_FULL);CHECK(result[0].x==91&&result[0].cold==93);}
  CHECK(d_rows_step(rows,n,2)==D_OK&&d_columns8_step(&columns,2)==D_OK);
  for(size_t i=0;i<n;++i)CHECK(rows[i].x==columns.x[i]&&rows[i].cold==columns.cold[i]);
 }
 for(unsigned bad=0;bad<8;++bad){DRow rows[8];for(unsigned i=0;i<8;++i)rows[i]=(DRow){5,1,90};rows[bad].x=INT32_MAX;
  CHECK(d_rows_step(rows,8,1)==D_RANGE);
  for(unsigned i=0;i<8;++i)CHECK(rows[i].x==(i==bad?INT32_MAX:5));}
 CHECK(d_rows_step(NULL,0,0)==D_OK);
 DRow large[32];int32_t x[32],velocity[32];
 for(unsigned i=0;i<32;++i){large[i]=(DRow){(int32_t)i,2,i+40};x[i]=(int32_t)i;velocity[i]=2;}
 CHECK(d_rows_step_many(large,32,10)==D_OK&&d_columns_step_many(x,velocity,32,10)==D_OK);
 for(unsigned i=0;i<32;++i)CHECK(large[i].x==(int32_t)i+20&&x[i]==large[i].x&&large[i].cold==i+40);
 large[31].x=INT32_MAX;x[31]=INT32_MAX;
 CHECK(d_rows_step_many(large,32,1)==D_RANGE&&d_columns_step_many(x,velocity,32,1)==D_RANGE);
 CHECK(large[0].x==20&&x[0]==20);
 unsigned cold[32];DRow roundtrip[32];
 for(unsigned i=0;i<32;++i){x[i]=-777;velocity[i]=-778;cold[i]=999;roundtrip[i]=(DRow){-779,-780,998};}
 CHECK(d_rows_to_arrays(large,32,x,velocity,cold,32)==D_OK);
 for(unsigned i=0;i<32;++i)CHECK(x[i]==large[i].x&&velocity[i]==large[i].velocity&&cold[i]==large[i].cold);
 CHECK(d_arrays_to_rows(x,velocity,cold,32,roundtrip,32)==D_OK);
 for(unsigned i=0;i<32;++i)CHECK(roundtrip[i].x==large[i].x&&roundtrip[i].velocity==large[i].velocity&&roundtrip[i].cold==large[i].cold);
 x[0]=123;roundtrip[0].x=456;
 CHECK(d_rows_to_arrays(large,32,x,velocity,cold,31)==D_FULL&&x[0]==123);
 CHECK(d_arrays_to_rows(x,velocity,cold,32,roundtrip,31)==D_FULL&&roundtrip[0].x==456);
 CHECK(d_rows_to_arrays(NULL,0,NULL,NULL,NULL,0)==D_OK&&d_arrays_to_rows(NULL,NULL,NULL,0,NULL,0)==D_OK);
}
static uint32_t scalar16(uint32_t a,uint32_t b){uint32_t out=0;for(unsigned i=0;i<2;++i){uint32_t n=((a>>(16*i))&65535)+((b>>(16*i))&65535);if(n>65535)n=65535;out|=n<<(16*i);}return out;}
static void swar16(void){
 const uint32_t edge[]={0,1,32767,32768,65534,65535};
 for(unsigned i=0;i<6;++i)for(unsigned j=0;j<6;++j)for(unsigned k=0;k<6;++k)for(unsigned l=0;l<6;++l){
  uint32_t a=edge[i]|(edge[k]<<16),b=edge[j]|(edge[l]<<16);CHECK(d_swar16_add(a,b)==scalar16(a,b));}
 for(uint32_t i=0;i<65536;++i){uint32_t a=i|((65535-i)<<16),b=1u|(i<<16);CHECK(d_swar16_add(a,b)==scalar16(a,b));}
}
typedef struct {unsigned calls;unsigned live[32];} Resources;
static void destroy(void *ctx,void *resource){Resources *r=ctx;unsigned *v=resource;CHECK(*v<32&&r->live[*v]);r->live[*v]=0;++r->calls;}
static void segmented(void){
 Resources r={0};unsigned ids[32];for(unsigned i=0;i<32;++i)ids[i]=i;
 DSegment s;d_segment_init(&s,destroy,&r);DSegmentHandle h[8];
 for(unsigned i=0;i<8;++i){r.live[i]=1;CHECK(d_segment_add(&s,(int)i,&ids[i],&h[i])==D_OK);CHECK(h[i].page==i/4);}
 DSegmentHandle extra={0};r.live[8]=1;CHECK(d_segment_add(&s,99,&ids[8],&extra)==D_FULL);CHECK(r.live[8]);r.live[8]=0;
 const int *borrow;CHECK(d_segment_borrow(&s,h[0],&borrow)==D_OK&&*borrow==0);
 CHECK(d_segment_remove(&s,h[0])==D_OK&&r.calls==1);
 r.live[9]=1;CHECK(d_segment_add(&s,77,&ids[9],&extra)==D_OK);
 int out=91;CHECK(d_segment_get(&s,h[0],&out)==D_STALE&&out==91);
 /* Slot storage still lives: this defined observation demonstrates that a
    generation token cannot revoke a previously borrowed address. */
 CHECK(*borrow==77);CHECK(d_segment_remove(&s,extra)==D_OK);
 r.live[10]=1;CHECK(d_segment_add(&s,88,&ids[10],&extra)==D_OK);CHECK(d_segment_remove(&s,extra)==D_OK);
 r.live[11]=1;CHECK(d_segment_add(&s,99,&ids[11],&extra)==D_FULL);CHECK(r.live[11]);r.live[11]=0;
 for(unsigned i=1;i<8;++i)CHECK(d_segment_remove(&s,h[i])==D_OK);
 CHECK(r.calls==10);for(unsigned i=0;i<32;++i)CHECK(!r.live[i]);
}
static void query(void){
 DQueryWorld w;d_query_world_init(&w);DQuery cache={0};
 CHECK(d_query_set(&w,0,1,10,0)==D_OK);CHECK(d_query_set(&w,1,3,20,1)==D_OK);
 CHECK(d_query_get(&w,1,2,&cache)==D_OK&&cache.count==1&&cache.ids[0]==0);
 CHECK(d_query_set(&w,0,3,10,2)==D_OK);
 CHECK(d_query_get(&w,1,2,&cache)==D_OK&&cache.count==0);
 for(unsigned req=0;req<4;++req)for(unsigned exc=0;exc<4;++exc){DQuery before=cache;
  int status=d_query_get(&w,req,exc,&cache);
  if(req&exc){CHECK(status==D_INVALID&&cache.version==before.version&&cache.count==before.count);continue;}
  CHECK(status==D_OK);unsigned n=0;for(unsigned id=0;id<4;++id){unsigned mask=w.ecs.mask[id];if(mask&&(mask&req)==req&&!(mask&exc)){CHECK(cache.ids[n]==id);++n;}}CHECK(cache.count==n);
 }
 w.version=UINT_MAX;unsigned mask=w.ecs.mask[2];CHECK(d_query_set(&w,2,2,0,1)==D_FULL&&w.ecs.mask[2]==mask);
}
static void oracle(void){
 const uint32_t edge[]={0,1,32767,32768,65534,65535};
 for(unsigned i=0;i<6;++i)for(unsigned j=0;j<6;++j)for(unsigned k=0;k<6;++k)for(unsigned l=0;l<6;++l){
  uint32_t a=edge[i]|(edge[k]<<16),b=edge[j]|(edge[l]<<16);printf("{\"kind\":\"swar16\",\"a\":%u,\"b\":%u,\"sum\":%u}\n",a,b,d_swar16_add(a,b));}
 for(size_t n=0;n<=8;++n)for(unsigned cold=0;cold<2;++cold){
  DRow rows[8],out[8];for(unsigned i=0;i<8;++i)rows[i]=(DRow){(int32_t)i-3,(int32_t)i%3-1,i+40};
  DTiles t;CHECK(d_rows_to_tiles(rows,n,cold,&t)==D_OK);CHECK(d_tiles_to_rows(&t,out,n)==D_OK);
  printf("{\"kind\":\"tiles\",\"count\":%zu,\"cold\":%u,\"rows\":[",n,cold);
  for(size_t i=0;i<n;++i)printf("%s[%d,%d,%u]",i?",":"",out[i].x,out[i].velocity,out[i].cold);
  puts("]}");
 }
}
int main(int argc,char **argv){CHECK(argc==2);test_name=argv[1];if(!strcmp(argv[1],"layout"))layout();else if(!strcmp(argv[1],"swar16"))swar16();else if(!strcmp(argv[1],"segmented"))segmented();else if(!strcmp(argv[1],"query"))query();else if(!strcmp(argv[1],"oracle"))oracle();else CHECK(0);return 0;}
