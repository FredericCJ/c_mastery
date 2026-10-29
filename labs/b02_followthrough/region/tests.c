#include "region.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BF_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
struct bf_clean {struct mb_region*r;struct mb_span span;unsigned id,*trace,*used;};
static void clean(void*v){struct bf_clean*c=v;c->trace[(*c->used)++]=c->id;unsigned char out=0;struct mb_span s;
 CHECK(mb_region_read(c->r,c->span,0,&out)==MB_OK&&out==42);
 CHECK(mb_region_reset(c->r)==MB_BUSY&&mb_region_alloc(c->r,1,1,&s)==MB_BUSY&&mb_region_cleanup(c->r,clean,c)==MB_BUSY);
}
static void region(void){
 struct mb_region r,foreign;mb_region_init(&r);mb_region_init(&foreign);struct mb_span a,b;CHECK(mb_region_alloc(&r,3,1,&a)==MB_OK&&a.offset==0&&mb_region_alloc(&r,5,4,&b)==MB_OK&&b.offset==4&&r.cursor==9&&r.payload==8&&r.high==9);r.bytes[0]=42;
 unsigned trace[4],used=0;struct bf_clean c[4];for(unsigned i=0;i<4;++i){c[i]=(struct bf_clean){&r,a,i+1,trace,&used};CHECK(mb_region_cleanup(&r,clean,&c[i])==MB_OK);}CHECK(mb_region_cleanup(&r,clean,&c[0])==MB_FULL);
 unsigned char out=77;CHECK(mb_region_read(&foreign,a,0,&out)==MB_STALE&&out==77);struct mb_span bad=a;bad.offset=SIZE_MAX;CHECK(mb_region_read(&r,bad,0,&out)==MB_INVALID&&out==77);
 CHECK(mb_region_reset(&r)==MB_OK&&used==4&&trace[0]==4&&trace[1]==3&&trace[2]==2&&trace[3]==1&&!r.cursor&&!r.payload&&r.high==9);
 CHECK(mb_region_read(&r,a,0,&out)==MB_STALE&&mb_region_alloc(&r,32,8,&b)==MB_OK&&r.high==32&&mb_region_reset(&r)==MB_OK&&r.generation==3);
 CHECK(mb_region_alloc(&r,1,1,&b)==MB_OK&&mb_region_reset(&r)==MB_OK&&r.generation==4&&mb_region_alloc(&r,1,1,&b)==MB_FULL&&mb_region_reset(&r)==MB_OK&&r.generation==4);
 struct bf_cohort own;bf_cohort_init(&own);unsigned total=77;const unsigned char w[]={1,1,0,2};CHECK(bf_cohort_batch(&own,w,sizeof w,0,0,&total)==MB_OK&&total==2&&bf_cohort_reset(&own)==MB_OK);
 puts("BF region validated actual-MB32 cleanup4-3-2-1 read-during-cleanup retirement3");
}
static void cohort(void){
 static const unsigned char batches[2][13]={{4,1,0,1,2,1,0,3,255,255,4,0,8},{4,5,0,3,6,0,4,7,0,5,8,0,6}};
 struct bf_cohort c;bf_cohort_init(&c);CHECK(_Alignof(struct bf_object)>=32&&((uintptr_t)&c.objects[0])%32==0);unsigned total=77;
 for(unsigned batch=0;batch<2;++batch){CHECK(bf_cohort_batch(&c,batches[batch],13,0,0,&total)==MB_OK&&total==(batch?18u:65800u));CHECK(c.bytes.cursor==13&&c.bytes.payload==13&&c.bytes.high==13);for(unsigned i=0;i<4;++i)CHECK(c.objects[i].live&&c.objects[i].opens==batch+1);CHECK(bf_cohort_reset(&c)==MB_OK);for(unsigned i=0;i<4;++i)CHECK(!c.objects[i].live&&c.objects[i].closes==batch+1&&c.trace[batch*4+i]==batch*4+4-i);}
 for(unsigned kind=0;kind<2;++kind)for(unsigned at=1;at<=4;++at){struct bf_cohort f;bf_cohort_init(&f);total=77;CHECK(bf_cohort_batch(&f,batches[0],13,kind?0:at,kind?at:0,&total)==(kind?MB_FULL:MB_FAILED)&&total==77&&!f.bytes.cursor&&!f.bytes.count);unsigned opened=0,closed=0;for(unsigned i=0;i<4;++i){CHECK(!f.objects[i].live);opened+=f.objects[i].opens;closed+=f.objects[i].closes;}CHECK(opened==closed&&opened==(kind?at:at-1));}
 struct bf_cohort empty;bf_cohort_init(&empty);unsigned char bad[]={1,0,0,1};total=77;CHECK(bf_cohort_batch(&empty,bad,4,0,0,&total)==MB_INVALID&&total==77&&!empty.bytes.cursor&&empty.bytes.generation==1);
 puts("BF cohort validated typed-align32 two-batches every-open-and-registration-failure ordinal manual-unwind");
}
static void slice(void){
 _Alignas(8) unsigned char storage[64];struct bf_slice s;struct bf_slice_span span;CHECK(bf_slice_init(&s,storage,64,3,29)==MB_OK&&bf_slice_alloc(&s,5,8,&span)==MB_OK&&span.offset==8&&s.cursor==10&&s.payload==5&&s.high==10);CHECK((uintptr_t)&storage[span.offset]%8==0);storage[span.offset]=42;unsigned char out=77;CHECK(bf_slice_read(&s,span,0,&out)==MB_OK&&out==42);struct bf_slice_span prior=span;CHECK(bf_slice_alloc(&s,SIZE_MAX,8,&span)==MB_FULL&&span.offset==prior.offset&&s.cursor==10&&s.payload==5);CHECK(bf_slice_reset(&s)==MB_OK&&s.high==10&&bf_slice_read(&s,prior,0,&out)==MB_STALE);
 CHECK(bf_slice_init(&s,storage,64,64,1)==MB_INVALID&&s.generation==2);puts("BF slice validated absolute-base3 offset8 padding5 checked-failure generation");
}
struct bf_heap {struct bf_chunk*live[3];unsigned attempts,fail,count,releases,reverse[3];struct bf_growth*g;};
static struct bf_chunk*acquire(void*v){struct bf_heap*h=v;struct bf_growth_span s;CHECK(bf_growth_alloc(h->g,1,1,&s)==MB_BUSY&&bf_growth_reset(h->g)==MB_BUSY);++h->attempts;if(h->attempts==h->fail)return NULL;struct bf_chunk*c=malloc(sizeof *c);CHECK(c);h->live[h->count++]=c;return c;}
static void release(void*v,struct bf_chunk*c){struct bf_heap*h=v;CHECK(h->count&&h->live[h->count-1]==c);h->reverse[h->releases++]=h->count;--h->count;free(c);}
static void growth(void){
 for(unsigned fail=0;fail<=2;++fail){struct bf_chunk first;struct bf_growth g;struct bf_heap h={0};h.fail=fail;h.g=&g;CHECK(bf_growth_init(&g,&first,acquire,release,&h)==MB_OK);struct bf_growth_span span={NULL,77,77,77,77};unsigned admitted=0;
  for(unsigned i=0;i<4;++i){size_t before=g.payload,occupied=g.occupied,count=g.count;struct bf_growth_span old=span;int rc=bf_growth_alloc(&g,20,8,&span);if(rc==MB_FULL){CHECK(g.payload==before&&g.occupied==occupied&&g.count==count&&span.generation==old.generation&&span.chunk==old.chunk);break;}CHECK(rc==MB_OK);++admitted;g.chunks[span.chunk]->bytes[span.offset]=42;unsigned char out=0;CHECK(bf_growth_read(&g,span,0,&out)==MB_OK&&out==42);}
  CHECK(admitted==(fail?fail:4)&&g.payload==20*admitted&&g.occupied==g.payload&&g.high==g.payload&&g.capacity_high==32*admitted);
  if(!fail)CHECK(bf_growth_alloc(&g,20,8,&span)==MB_FULL&&h.attempts==3);
  CHECK(bf_growth_reset(&g)==MB_OK&&h.releases==admitted-1&&!h.count&&g.count==1&&!g.payload&&g.high==20*admitted);unsigned char out=77;CHECK(bf_growth_read(&g,span,0,&out)==MB_STALE&&out==77);
 }
 struct mb_region fixed;mb_region_init(&fixed);struct mb_span s;CHECK(mb_region_alloc(&fixed,20,8,&s)==MB_OK&&mb_region_alloc(&fixed,20,8,&s)==MB_FULL&&fixed.payload==20&&fixed.cursor==20);
 puts("BF growth validated actual-upstream-typed-malloc fixed20-vs-grow80 retained-capacity128 failure-unchanged reverse-release");
}
static void children(void){struct bf_region_tree t;bf_tree_init(&t);struct mb_span p,a,b;CHECK(mb_region_alloc(&t.parent,1,1,&p)==MB_OK&&mb_region_alloc(&t.child[0],1,1,&a)==MB_OK&&mb_region_alloc(&t.child[1],1,1,&b)==MB_OK);t.parent.bytes[0]=1;t.child[0].bytes[0]=2;t.child[1].bytes[0]=3;unsigned char out=77;CHECK(bf_tree_child_reset(&t,0)==MB_OK&&mb_region_read(&t.child[0],a,0,&out)==MB_STALE&&mb_region_read(&t.child[1],b,0,&out)==MB_OK&&out==3&&mb_region_read(&t.parent,p,0,&out)==MB_OK&&out==1);CHECK(mb_region_read(&t.child[0],b,0,&out)==MB_STALE&&bf_tree_child_reset(&t,2)==MB_INVALID);CHECK(bf_tree_reset(&t)==MB_OK&&mb_region_read(&t.parent,p,0,&out)==MB_STALE&&mb_region_read(&t.child[1],b,0,&out)==MB_STALE);puts("BF children validated parent-and-sibling-survive selected-child-reset whole-tree-invalidation");}
struct bf_retry {struct bf_retry_region*r;struct bf_borrow b;unsigned fail,calls,closed,id,*trace,*used;};
static int retry_close(void*v){struct bf_retry*r=v;CHECK(!r->closed);++r->calls;r->trace[(*r->used)++]=r->id;unsigned char out=0;CHECK(bf_arena_get(&r->r->arena,r->b,0,&out)==B2_OK&&out==42&&bf_retry_reset(r->r)==BF_ARENA_BUSY);if(r->fail){--r->fail;return 1;}r->closed=1;return 0;}
static void retry(void){struct bf_retry_region r;bf_retry_init(&r);struct bf_borrow b;CHECK(bf_retry_reserve(&r,8,8,&b)==B2_OK&&bf_arena_set(&r.arena,b,0,42)==B2_OK);unsigned trace[8],used=0;struct bf_retry c[3];for(unsigned i=0;i<3;++i){c[i]=(struct bf_retry){&r,b,i==1,0,0,i+1,trace,&used};CHECK(bf_retry_register(&r,retry_close,&c[i])==B2_OK);}CHECK(bf_retry_reset(&r)==BF_ARENA_CLEANUP&&r.active&&r.arena.raw.cursor==8&&used==3&&trace[0]==3&&trace[1]==2&&trace[2]==1);struct bf_borrow old=b;CHECK(bf_retry_reserve(&r,1,1,&b)==BF_ARENA_BUSY&&bf_retry_reset(&r)==B2_OK&&used==4&&trace[3]==2);unsigned char out=77;CHECK(bf_arena_get(&r.arena,old,0,&out)==B2_STALE&&!r.arena.raw.cursor&&r.active);for(unsigned i=0;i<14;++i)CHECK(bf_retry_reset(&r)==B2_OK);CHECK(!r.active&&bf_retry_reserve(&r,1,1,&b)==B2_FULL&&bf_retry_reset(&r)==B2_OK);puts("BF retry validated actual-prior-arena retained-failed-list once-successful-close finite-root-retirement");}
static int extents(void){size_t cursor,n,a;while(scanf("%zu %zu %zu",&cursor,&n,&a)==3){struct mb_region r;mb_region_init(&r);struct mb_span unused;if(cursor)CHECK(mb_region_alloc(&r,cursor,1,&unused)==MB_OK);struct mb_span out={NULL,77,77,77};int rc=mb_region_alloc(&r,n,a,&out);printf("%d %zu %zu %zu %zu\n",rc,out.offset,r.cursor,r.payload,r.high);}return 0;}
static int slices(void){size_t begin,cursor,n,a;while(scanf("%zu %zu %zu %zu",&begin,&cursor,&n,&a)==4){_Alignas(8) unsigned char storage[64];struct bf_slice s;struct bf_slice_span out={NULL,77,77,77},unused;CHECK(bf_slice_init(&s,storage,64,begin,64-begin)==MB_OK);if(cursor)CHECK(bf_slice_alloc(&s,cursor,1,&unused)==MB_OK);int rc=bf_slice_alloc(&s,n,a,&out);printf("%d %zu %zu %zu %zu\n",rc,out.offset,s.cursor,s.payload,s.high);}return 0;}
struct bf_pool {struct bf_chunk cells[3];unsigned used,attempts,fail;};
static struct bf_chunk*pool_get(void*v){struct bf_pool*p=v;if(++p->attempts==p->fail)return NULL;return &p->cells[p->used++];}
static void pool_put(void*v,struct bf_chunk*c){struct bf_pool*p=v;CHECK(p->used&&c==&p->cells[p->used-1]);--p->used;}
static int history(void){unsigned n,fail;char ops[8];while(scanf("%u %u %7s",&n,&fail,ops)==3){CHECK(n<=5&&fail<=3);struct bf_chunk first;struct bf_growth g;struct bf_pool p={0};p.fail=fail;CHECK(bf_growth_init(&g,&first,pool_get,pool_put,&p)==MB_OK);printf("%u",n);for(unsigned i=0;i<n;++i){unsigned op=(unsigned)(ops[i]-'0');CHECK(op<6);size_t sizes[3]={1,20,32};struct bf_growth_span s={NULL,77,77,77,77};int rc=bf_growth_alloc(&g,sizes[op/2],op%2?8:1,&s);printf(" %d %zu %zu %zu %zu %zu %u",rc,s.chunk,s.offset,g.count,g.payload,g.occupied,p.attempts);}CHECK(bf_growth_reset(&g)==MB_OK&&!p.used);putchar('\n');}return 0;}
int main(int argc,char**argv){CHECK(argc==2);if(!strcmp(argv[1],"region"))region();else if(!strcmp(argv[1],"cohort"))cohort();else if(!strcmp(argv[1],"slice"))slice();else if(!strcmp(argv[1],"growth"))growth();else if(!strcmp(argv[1],"children"))children();else if(!strcmp(argv[1],"retry"))retry();else if(!strcmp(argv[1],"extents"))return extents();else if(!strcmp(argv[1],"slices"))return slices();else if(!strcmp(argv[1],"oracle"))return history();else if(!strcmp(argv[1],"limits"))printf("%zu %zu\n",SIZE_MAX,_Alignof(max_align_t));else if(!strcmp(argv[1],"metrics"))printf("{\"region_bytes\":%zu,\"object_bytes\":%zu,\"object_align\":%zu,\"cohort_bytes\":%zu,\"slice_bytes\":%zu,\"growth_bytes\":%zu,\"chunk_bytes\":%zu,\"retry_bytes\":%zu}\n",sizeof(struct mb_region),sizeof(struct bf_object),_Alignof(struct bf_object),sizeof(struct bf_cohort),sizeof(struct bf_slice),sizeof(struct bf_growth),sizeof(struct bf_chunk),sizeof(struct bf_retry_region));else return 2;return 0;}
