#include "object_runtime_finalizers.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c) do { if(!(c)) { fprintf(stderr,"OR_CHECK:%d:%s\n",__LINE__,#c); return 1; } } while(0)
static const ORFHandle nil={0,0};
typedef struct { unsigned calls; int errors, resurrect, allocate, fail_alloc; ORFHandle child; } Context;
static ORStatus close_callback(ORFHeap *h, ORFHandle a, void *opaque) {
  unsigned *calls=opaque;
  (*calls)++;
  return orf_close_resource(h,a);
}
static ORStatus resurrect_other(ORFHeap *h, ORFHandle a, void *opaque) {
  (void)a;
  return orf_root(h,*(ORFHandle *)opaque,1);
}
static ORStatus callback(ORFHeap *h, ORFHandle a, void *opaque) {
  Context *c=opaque; uint16_t dead=123; unsigned released=123;
  ORFHandle out={99,99}; ORStatus status=OR_UNIMPLEMENTED;
  c->calls++;
  if(c->resurrect && orf_root(h,a,1)!=OR_OK) c->errors++;
  if(orf_collect(h,&dead)!=OR_STATE || dead!=123) c->errors++;
  if(orf_run_one(h,&out,&status)!=OR_STATE || out.slot!=99 || status!=OR_UNIMPLEMENTED) c->errors++;
  if(orf_cleanup(h,1,&released)!=OR_STATE || released!=123) c->errors++;
  if(orf_close(h)!=OR_STATE) c->errors++;
  if(c->allocate) {
    ORStatus s=orf_new(h,0,0,0,c->fail_alloc,&out);
    if(c->fail_alloc) { if(s!=OR_FULL || out.slot!=99) c->errors++; }
    else if(s!=OR_OK) c->errors++; else c->child=out;
  }
  return OR_FULL; /* Application finalization failed after committed mutations. */
}
static int finalizers(void) {
  ORFHeap h, before; ORFHandle a,b,out; ORStatus status; uint16_t dead;
  unsigned released,i; Context c={0};
  orf_init(&h); c.resurrect=1; c.allocate=1;
  CHECK(orf_new(&h,callback,&c,1,0,&a)==OR_OK);
  CHECK(orf_weak(&h,0,a)==OR_OK && orf_root(&h,a,0)==OR_OK);
  CHECK(c.calls==0 && h.resources==1); /* No implicit prompt finalization. */
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==0 && h.weak.weak[0]==0 && h.weak.pending==1);
  before=h; dead=777;
  CHECK(orf_collect(&h,&dead)==OR_STATE && dead==777 && memcmp(&h,&before,sizeof h)==0);
  CHECK(orf_run_one(&h,&out,&status)==OR_OK && out.slot==a.slot && status==OR_FULL);
  CHECK(c.calls==1 && !c.errors && c.child.slot==2 && h.weak.finalized==1);
  CHECK(h.cleanup_pending==1 && h.resources==1);
  CHECK(orf_collect(&h,&dead)==OR_STATE);
  CHECK(orf_cleanup(&h,0,&released)==OR_OK && released==0 && h.resources==1);
  CHECK(orf_cleanup(&h,1,&released)==OR_DONE && released==1 && h.resources_closed==1);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==0);
  out=(ORFHandle){99,99}; status=OR_UNIMPLEMENTED;
  CHECK(orf_run_one(&h,&out,&status)==OR_DONE && out.slot==99 && status==OR_UNIMPLEMENTED);
  CHECK(orf_root(&h,a,0)==OR_OK && orf_root(&h,c.child,0)==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==3 && c.calls==1);
  CHECK(orf_root(&h,a,1)==OR_INVALID && orf_close_resource(&h,a)==OR_INVALID);
  CHECK(orf_new(&h,0,0,0,0,&b)==OR_OK && b.slot==1 && b.generation==2);
  before=h;out=(ORFHandle){99,99};
  CHECK(orf_new(&h,0,0,0,1,&out)==OR_FULL && out.slot==99 && memcmp(&h,&before,sizeof h)==0);
  CHECK(orf_close(&h)==OR_OK && orf_close(&h)==OR_STATE);

  orf_init(&h); c=(Context){0}; c.allocate=1;c.fail_alloc=1;
  CHECK(orf_new(&h,callback,&c,0,0,&a)==OR_OK && orf_root(&h,a,0)==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && orf_run_one(&h,&out,&status)==OR_OK);
  CHECK(c.calls==1 && !c.errors && h.weak.graph.live==1);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==1);

  orf_init(&h); c=(Context){0};
  for(i=0;i<16;i++) { CHECK(orf_new(&h,callback,&c,1,0,&a)==OR_OK); CHECK(orf_root(&h,a,0)==OR_OK); }
  out=(ORFHandle){99,99};before=h;
  CHECK(orf_new(&h,0,0,0,0,&out)==OR_FULL && out.slot==99 && memcmp(&h,&before,sizeof h)==0);
  CHECK(orf_collect(&h,&dead)==OR_OK && !dead && h.weak.pending==65535);
  for(i=0;i<16;i++) CHECK(orf_run_one(&h,&out,&status)==OR_OK && out.slot==i+1);
  CHECK(c.calls==16 && !c.errors && h.cleanup_pending==65535);
  CHECK(orf_cleanup(&h,5,&released)==OR_OK && released==5);
  CHECK(orf_cleanup(&h,99,&released)==OR_DONE && released==11 && h.resources_closed==16);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==65535);

  orf_init(&h);
  for(i=0;i<48;i++) { CHECK(orf_new(&h,0,0,0,0,&a)==OR_OK); CHECK(orf_root(&h,a,0)==OR_OK); CHECK(orf_collect(&h,&dead)==OR_OK && dead); }
  before=h;out=(ORFHandle){99,99};
  CHECK(orf_new(&h,0,0,0,0,&out)==OR_FULL && out.slot==99 && memcmp(&h,&before,sizeof h)==0);
  CHECK(orf_close(&h)==OR_OK);
  orf_init(&h); c=(Context){0};
  CHECK(orf_new(&h,callback,&c,1,0,&a)==OR_OK);
  CHECK(orf_close(&h)==OR_OK && h.resources_closed==1 && c.calls==0);
  CHECK(orf_new(&h,0,0,0,0,&out)==OR_STATE);
  orf_init(&h); i=0;
  CHECK(orf_new(&h,close_callback,&i,1,0,&a)==OR_OK && orf_root(&h,a,0)==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && orf_run_one(&h,&out,&status)==OR_OK);
  CHECK(status==OR_OK && i==1 && !h.resources && !h.cleanup_pending && h.resources_closed==1);
  CHECK(orf_close_resource(&h,a)==OR_STATE && orf_collect(&h,&dead)==OR_OK && dead==1);
  orf_init(&h); c=(Context){0};
  CHECK(orf_new(&h,resurrect_other,&b,0,0,&a)==OR_OK);
  CHECK(orf_new(&h,callback,&c,0,0,&b)==OR_OK && orf_weak(&h,0,b)==OR_OK);
  CHECK(orf_root(&h,a,0)==OR_OK && orf_root(&h,b,0)==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && h.weak.pending==3 && h.weak.weak[0]==0);
  CHECK(orf_run_one(&h,&out,&status)==OR_OK && out.slot==a.slot && status==OR_OK);
  CHECK(h.weak.graph.roots==2 && h.weak.weak[0]==0);
  CHECK(orf_run_one(&h,&out,&status)==OR_OK && out.slot==b.slot && c.calls==1 && !c.errors);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==1 && h.weak.graph.live==2);
  CHECK(orf_root(&h,b,0)==OR_OK && orf_collect(&h,&dead)==OR_OK && dead==2);
  return 0;
}
static int ephemeron(void) {
  ORFHeap h; ORFHandle a[4]; uint16_t dead; unsigned i;
  orf_init(&h);
  for(i=0;i<4;i++) { CHECK(orf_new(&h,0,0,0,0,&a[i])==OR_OK); CHECK(orf_root(&h,a[i],0)==OR_OK); }
  CHECK(orf_root(&h,a[0],1)==OR_OK && orf_edge(&h,a[1],0,a[2])==OR_OK);
  CHECK(orf_pair(&h,0,a[2],a[3])==OR_OK && orf_pair(&h,1,a[0],a[1])==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==0 && h.weak.graph.live==15);
  CHECK(orf_root(&h,a[0],0)==OR_OK && orf_edge(&h,a[3],0,a[0])==OR_OK);
  CHECK(orf_collect(&h,&dead)==OR_OK && dead==15 && h.weak.graph.live==0);
  for(i=0;i<4;i++) CHECK(!h.weak.key[i] && !h.weak.value[i]);
  CHECK(orf_pair(&h,0,nil,a[0])==OR_INVALID);
  return 0;
}
static int oracle(void) {
  unsigned graph,mask,v0,v1,i,code; ORFHeap h; ORFHandle a[4]; uint16_t dead;
  for(graph=0;graph<64;graph++) for(mask=0;mask<16;mask++)
    for(v0=0;v0<5;v0++) for(v1=0;v1<5;v1++) {
      orf_init(&h);
      for(i=0;i<4;i++) { CHECK(orf_new(&h,0,0,0,0,&a[i])==OR_OK); CHECK(orf_root(&h,a[i],(mask>>i)&1)==OR_OK); }
      code=graph;
      for(i=0;i<3;i++) { unsigned target=code%4;code/=4; CHECK(orf_edge(&h,a[i],0,target?a[target-1]:nil)==OR_OK); }
      CHECK(orf_pair(&h,0,v0?a[2]:nil,v0?a[v0-1]:nil)==OR_OK);
      CHECK(orf_pair(&h,1,v1?a[0]:nil,v1?a[v1-1]:nil)==OR_OK);
      CHECK(orf_collect(&h,&dead)==OR_OK);
      printf("F %u %u %u %u %u %u\n",graph,mask,v0,v1,h.weak.graph.live,dead);
    }
  return 0;
}
int main(int argc,char **argv) {
  if(argc!=2) return 2;
  if(!strcmp(argv[1],"finalizers")) return finalizers();
  if(!strcmp(argv[1],"ephemeron_graph")) return ephemeron();
  if(!strcmp(argv[1],"oracle")) return oracle();
  return 2;
}
