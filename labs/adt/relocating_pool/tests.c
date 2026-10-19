#define _POSIX_C_SOURCE 200809L
#include "relocating_pool.h"
#include <inttypes.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ADO_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
typedef struct {unsigned calls,frees,live,fail;} Memory;
/* All callbacks in a native test are serialized by its sole pool mutex. The
   registry only makes deliberate assertion-failing controls safe to exit. */
static void *allocated[16];
static void cleanup(void){for(unsigned i=0;i<16;++i)free(allocated[i]);}
static void *allocate(void *ctx,size_t n){Memory *m=ctx;++m->calls;if(m->fail==m->calls)return NULL;void *p=malloc(n);if(!p)return NULL;for(unsigned i=0;i<16;++i)if(!allocated[i]){allocated[i]=p;++m->live;return p;}abort();}
static void deallocate(void *ctx,void *p){Memory *m=ctx;for(unsigned i=0;i<16;++i)if(allocated[i]==p){allocated[i]=NULL;++m->frees;--m->live;free(p);return;}abort();}
static AdpOps ops(Memory *m){return(AdpOps){allocate,deallocate,m};}
static int end(AdpPool *p,Memory *m){CHECK(adp_close(p)==ADP_OK);CHECK(m->live==0);CHECK(adp_destroy(p)==ADP_OK);return 0;}
static int identity(void){
 Memory m={0};AdpIds ids;AdpId a,b,keep={88};CHECK(adp_ids_init(&ids,UINT64_MAX-1)==ADP_OK);CHECK(adp_issue(&ids,&a)==ADP_OK&&a.value==UINT64_MAX-1);CHECK(adp_issue(&ids,&b)==ADP_OK&&b.value==UINT64_MAX);CHECK(adp_issue(&ids,&keep)==ADP_FULL&&keep.value==88);CHECK(adp_ids_init(&ids,0)==ADP_INVALID&&ids.exhausted);
 AdpPool p,q;CHECK(adp_init(&p,a,ops(&m),1)==ADP_OK);CHECK(adp_init(&q,b,ops(&m),1)==ADP_OK);AdpHandle h,j,copy;CHECK(adp_acquire(&p,77,&h)==ADP_OK);CHECK(adp_acquire(&q,22,&j)==ADP_OK);uint32_t value=99;CHECK(adp_get(&q,h,&value)==ADP_STALE&&value==99);copy=h;CHECK(adp_get(&p,copy,&value)==ADP_OK&&value==77);CHECK(adp_release(&p,h)==ADP_OK);CHECK(adp_acquire(&p,88,&j)==ADP_OK);value=99;CHECK(adp_get(&p,h,&value)==ADP_STALE&&value==99);CHECK(adp_release(&p,copy)==ADP_STALE);CHECK(adp_close(&p)==ADP_OK);CHECK(adp_destroy(&p)==ADP_OK);CHECK(end(&q,&m)==0);puts("ADP_ID nominal-runtime-generation");return 0;
}
static int retirement(void){
 Memory m={0};AdpPool p;AdpHandle h[4],old,keep={{99},99,99};CHECK(adp_init(&p,(AdpId){4},ops(&m),UINT64_MAX-1)==ADP_OK);
 for(unsigned i=0;i<4;++i){CHECK(adp_acquire(&p,i,&h[i])==ADP_OK&&h[i].index==i);old=h[i];CHECK(adp_release(&p,old)==ADP_OK);CHECK(adp_acquire(&p,i,&h[i])==ADP_OK&&h[i].generation==UINT64_MAX);CHECK(adp_release(&p,h[i])==ADP_OK);}
 CHECK(adp_acquire(&p,8,&keep)==ADP_FULL&&keep.index==99&&keep.generation==99);AdpSnapshot s;CHECK(adp_snapshot(&p,&s)==ADP_OK);for(unsigned i=0;i<4;++i)CHECK(s.slots[i].state==ADP_RETIRED&&s.slots[i].generation==UINT64_MAX);
 CHECK(end(&p,&m)==0);CHECK(adp_init(&p,(AdpId){5},ops(&m),1)==ADP_OK);CHECK(adp_acquire(&p,4,&old)==ADP_OK);CHECK(adp_release(&p,old)==ADP_OK);CHECK(adp_acquire(&p,4,&old)==ADP_OK&&old.generation==2);CHECK(end(&p,&m)==0);puts("ADP_RETIRE finite-no-wrap");return 0;
}
typedef struct {AdpPool *pool;AdpHandle handle;unsigned bad;} MovedReader;
static void *moved_reader(void *context){MovedReader *r=context;for(unsigned i=0;i<1000;++i){uint32_t v=0;if(adp_get(r->pool,r->handle,&v)!=ADP_OK||v!=4321)++r->bad;}return NULL;}
static int moved_readers(AdpPool *p,AdpHandle h){MovedReader r[3];pthread_t threads[3];unsigned created=0,bad=0;for(unsigned i=0;i<3;++i){r[i]=(MovedReader){p,h,0};if(pthread_create(&threads[i],NULL,moved_reader,&r[i]))break;++created;}for(unsigned i=0;i<created;++i){if(pthread_join(threads[i],NULL))++bad;bad+=r[i].bad;}CHECK(created==3&&bad==0);return 0;}
static int relocation(void){
 Memory m={.fail=1};AdpPool p,q;CHECK(adp_init(&p,(AdpId){7},ops(&m),1)==ADP_NOMEM&&m.live==0);m.fail=0;CHECK(adp_init(&p,(AdpId){7},ops(&m),UINT64_MAX-1)==ADP_OK);AdpHandle h;CHECK(adp_acquire(&p,4321,&h)==ADP_OK);CHECK(adp_destroy(&p)==ADP_STATE);m.fail=m.calls+1;CHECK(adp_compact(&p)==ADP_NOMEM&&m.live==1);uint32_t v=0;CHECK(adp_get(&p,h,&v)==ADP_OK&&v==4321);m.fail=0;CHECK(adp_compact(&p)==ADP_OK&&m.live==1);CHECK(adp_get(&p,h,&v)==ADP_OK&&v==4321);CHECK(moved_readers(&p,h)==0);CHECK(adp_move(&q,&p)==ADP_OK);v=9;CHECK(adp_get(&p,h,&v)==ADP_CLOSED&&v==9);CHECK(adp_destroy(&p)==ADP_OK);CHECK(adp_get(&q,h,&v)==ADP_OK&&v==4321);CHECK(moved_readers(&q,h)==0);CHECK(end(&q,&m)==0);puts("ADP_MOVE fresh-mutex-transferred-block joined-restarted-readers=6000");return 0;
}
typedef struct {AdpPool *p;AdpHandle old;atomic_uint *start;unsigned bad,reads;} Worker;
static void *reader(void *arg){Worker *w=arg;while(!atomic_load_explicit(w->start,memory_order_acquire)){}for(unsigned i=0;i<10000;++i){uint32_t v=987;AdpStatus s=adp_get(w->p,w->old,&v);if(!((s==ADP_OK&&v==123)||(s==ADP_STALE&&v==987)||(s==ADP_CLOSED&&v==987)))++w->bad;++w->reads;}return NULL;}
/* A defined atomic witness: checking a generation before reading an atomic
   payload does not reserve the incarnation. Handshake gives deterministic
   interleaving without introducing a C data race as the "proof". */
typedef struct {atomic_uint generation,value,step;unsigned observed;} Witness;
static void *check_then_read(void *arg){Witness *w=arg;unsigned g=atomic_load(&w->generation);atomic_store(&w->step,1);while(atomic_load(&w->step)!=2){}w->observed=g==1?atomic_load(&w->value):0;return NULL;}
static int native(void){
 Memory m={0};AdpPool p;CHECK(adp_init(&p,(AdpId){17},ops(&m),1)==ADP_OK);AdpHandle h;CHECK(adp_acquire(&p,123,&h)==ADP_OK);atomic_uint start=0;Worker w[3];pthread_t t[3];unsigned created=0,bad=0;
 for(unsigned i=0;i<3;++i){w[i]=(Worker){&p,h,&start,0,0};if(pthread_create(&t[i],NULL,reader,&w[i]))break;++created;}atomic_store_explicit(&start,1,memory_order_release);
 if(adp_release(&p,h)!=ADP_OK)++bad;
 for(unsigned i=0;i<1000;++i){AdpHandle next;if(adp_acquire(&p,i+1000,&next)!=ADP_OK){++bad;break;}if(i%10==0&&adp_compact(&p)!=ADP_OK)++bad;if(adp_release(&p,next)!=ADP_OK)++bad;}
 if(adp_close(&p)!=ADP_OK)++bad;
 /* Join EVERY borrower before inspecting semantic failures. A rejected control
    must not turn an early return into a dangling worker-context access. */
 for(unsigned i=0;i<created;++i)if(pthread_join(t[i],NULL))++bad;
 for(unsigned i=0;i<created;++i)if(w[i].bad||w[i].reads!=10000)++bad;
 CHECK(created==3&&bad==0);CHECK(m.live==0);uint32_t v=9;CHECK(adp_get(&p,h,&v)==ADP_CLOSED&&v==9);CHECK(adp_close(&p)==ADP_OK);CHECK(adp_destroy(&p)==ADP_OK);
 Witness x;atomic_init(&x.generation,1);atomic_init(&x.value,123);atomic_init(&x.step,0);x.observed=0;pthread_t a;CHECK(pthread_create(&a,NULL,check_then_read,&x)==0);while(atomic_load(&x.step)!=1){}atomic_store(&x.generation,2);atomic_store(&x.value,999);atomic_store(&x.step,2);CHECK(pthread_join(a,NULL)==0&&x.observed==999);puts("ADP_NATIVE copied-read-close-join atomic-witness=999 reads=30000");return 0;
}
static int baseline(void){ADPool p,q;ad_pool_init(&p);ad_pool_init(&q);ADBrand h,j;uint32_t v=99;CHECK(ad_pool_acquire(&p,7,&h));CHECK(ad_pool_acquire(&q,8,&j));CHECK(!ad_pool_get(&q,h,&v)&&v==99);CHECK(ad_pool_release(&p,h));CHECK(ad_pool_acquire(&p,9,&j));CHECK(!ad_pool_get(&p,h,&v)&&v==99);CHECK(!ad_pool_release(&p,h));p.slot[j.index].generation=UINT_MAX;j.generation=UINT_MAX;CHECK(ad_pool_release(&p,j));CHECK(p.slot[j.index].retired);for(unsigned i=1;i<4;++i)CHECK(ad_pool_acquire(&p,i,&j)&&j.index==i);CHECK(!ad_pool_acquire(&p,5,&j));puts("ADP_BASE actual-fixed-address-pool");return 0;}
static void slotprint(const AdpSlot *s){printf(" %" PRIu64 " %u %" PRIu32,s->generation,(unsigned)s->state,s->value);}
static int oracle(void){char line[128];while(fgets(line,sizeof line,stdin)){size_t n=strcspn(line,"\n");line[n]=0;if(n<2)return 2;Memory m={0};AdpPool p,q;AdpPool *now=&p;int moved=0,closed=0;CHECK(adp_init(now,(AdpId){71},ops(&m),line[0]=='x'?UINT64_MAX-1:1)==ADP_OK);AdpHandle h[2]={{{71},9,1},{{71},9,1}};
 for(size_t i=1;i<n;++i){AdpStatus s=ADP_INVALID;uint32_t out=999;char op=line[i];if(op=='0'||op=='1')s=adp_acquire(now,(uint32_t)(10+i),&h[op-'0']);else if(op=='2'||op=='3')s=adp_get(now,h[op-'2'],&out);else if(op=='4'||op=='5')s=adp_release(now,h[op-'4']);else if(op=='6')s=adp_compact(now);else if(op=='7'){m.fail=m.calls+1;s=adp_compact(now);m.fail=0;}else if(op=='8'){if(!moved){s=adp_move(&q,&p);if(s==ADP_OK){now=&q;moved=1;}}else s=ADP_STATE;}else if(op=='9'){s=adp_close(now);closed=1;}else if(op=='a'){AdpHandle bad=h[0];bad.pool.value=72;s=adp_get(now,bad,&out);}else if(op=='b'){h[1]=h[0];s=ADP_OK;}
 printf("%u %" PRIu32 " %u %u %u %u %u %" PRIu64 " %u %" PRIu64,(unsigned)s,out,(unsigned)closed,m.calls,m.frees,m.live,h[0].index,h[0].generation,h[1].index,h[1].generation);if(!closed){AdpSnapshot snap;CHECK(adp_snapshot(now,&snap)==ADP_OK);for(unsigned j=0;j<4;++j)slotprint(&snap.slots[j]);}puts(";");}
 CHECK(adp_close(now)==ADP_OK);CHECK(adp_destroy(now)==ADP_OK);if(moved)CHECK(adp_destroy(&p)==ADP_OK);CHECK(m.live==0);puts(".");}return 0;}
static int baseoracle(void){char line[128];while(fgets(line,sizeof line,stdin)){ADPool p;ad_pool_init(&p);ADBrand h[2]={{&p,9,1},{&p,9,1}};size_t n=strcspn(line,"\n");for(size_t i=0;i<n;++i){int s=0;uint32_t out=999;char op=line[i];if(op=='0'||op=='1')s=ad_pool_acquire(&p,(uint32_t)(10+i),&h[op-'0']);else if(op=='2'||op=='3')s=ad_pool_get(&p,h[op-'2'],&out);else if(op=='4'||op=='5')s=ad_pool_release(&p,h[op-'4']);else{h[1]=h[0];s=1;}printf("%d %" PRIu32 " %u %u %u %u",s,out,h[0].index,h[0].generation,h[1].index,h[1].generation);for(unsigned j=0;j<4;++j)printf(" %u %u %u %" PRIu32,p.slot[j].generation,p.slot[j].live,p.slot[j].retired,p.slot[j].value);puts(";");}puts(".");}return 0;}
static double seconds(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))abort();return (double)t.tv_sec+(double)t.tv_nsec/1e9;}
static int metrics(void){Memory m={0};AdpPool p;CHECK(adp_init(&p,(AdpId){1},ops(&m),1)==ADP_OK);AdpHandle h;CHECK(adp_acquire(&p,123,&h)==ADP_OK);double raw[7],safe[7];uint64_t checksum=0;ADPool base;ADBrand b;ad_pool_init(&base);CHECK(ad_pool_acquire(&base,123,&b));for(unsigned k=0;k<7;++k){double t=seconds();checksum=0;for(unsigned i=0;i<100000;++i){uint32_t v;CHECK(ad_pool_get(&base,b,&v));checksum+=v;}raw[k]=seconds()-t;t=seconds();checksum=0;for(unsigned i=0;i<100000;++i){uint32_t v;CHECK(adp_get(&p,h,&v)==ADP_OK);checksum+=v;}safe[k]=seconds()-t;}CHECK(end(&p,&m)==0);printf("{\"iterations\":100000,\"checksum\":%" PRIu64 ",\"pool_size\":%zu,\"handle_size\":%zu,\"snapshot_size\":%zu,\"raw_seconds\":[",checksum,sizeof(AdpPool),sizeof(AdpHandle),sizeof(AdpSnapshot));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",raw[i]);printf("],\"safe_seconds\":[");for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",safe[i]);puts("]}");return 0;}
int main(int argc,char **argv){if(atexit(cleanup)||argc!=2)return 2;if(!strcmp(argv[1],"pool_identity"))return identity();if(!strcmp(argv[1],"pool_retirement"))return retirement();if(!strcmp(argv[1],"pool_relocation"))return relocation();if(!strcmp(argv[1],"pool_native"))return native();if(!strcmp(argv[1],"pool_base"))return baseline();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"baseoracle"))return baseoracle();if(!strcmp(argv[1],"metrics"))return metrics();return 2;}
