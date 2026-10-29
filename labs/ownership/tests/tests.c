#include "ownership.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"OW_CHECK:%s:%d:%s\n",__func__,__LINE__,#x);return 1;} }while(0)
static void append(void *p) { unsigned *n=p;++*n; }
static int frontier(void) {
 for(size_t fail=1;fail<=4;fail++) {
  struct ow_allocator a={0};a.fail_at=fail;struct ow_owner o={0};size_t lengths[3]={1,2,3};
  enum ow_status s=ow_construct(&o,ow_allocator(&a),lengths);
  if(fail<=3){CHECK(s==OW_NOMEM);CHECK(o.count==0);CHECK(a.live==0);CHECK(a.releases==fail-1);}
  else {CHECK(s==OW_OK);CHECK(o.count==3 && a.live==3);CHECK(ow_destroy(&o)==OW_OK);CHECK(ow_destroy(&o)==OW_OK);CHECK(a.live==0 && a.releases==3);}
 }
 struct ow_allocator a={0};struct ow_owner o={0};size_t bad[3]={1,0,2};CHECK(ow_construct(&o,ow_allocator(&a),bad)==OW_INVALID);CHECK(a.attempts==0);return 0;
}
struct ow_digit {unsigned *trace;unsigned digit;};
static void digit(void *p){struct ow_digit *d=p;*d->trace=*d->trace*10+d->digit;}
static int scope(void) {
 struct ow_scope s={0};unsigned trace=0;struct ow_digit d[3]={{&trace,1},{&trace,2},{&trace,3}};
 CHECK(ow_defer(&s,digit,&d[0],OW_ALWAYS)==OW_OK);CHECK(ow_defer(&s,digit,&d[1],OW_ON_SUCCESS)==OW_OK);CHECK(ow_defer(&s,digit,&d[2],OW_ON_ERROR)==OW_OK);
 CHECK(ow_finish(&s,0)==OW_OK);CHECK(trace==31 && s.count==0);CHECK(ow_finish(&s,0)==OW_OK);CHECK(trace==31);
 struct ow_allocator a={0};struct ow_owner old={0},fresh={0};size_t lengths[3]={1,1,1};CHECK(ow_construct(&old,ow_allocator(&a),lengths)==OW_OK);CHECK(ow_construct(&fresh,ow_allocator(&a),lengths)==OW_OK);
 int32_t *new_first=fresh.block[0];CHECK(ow_publish(&old,&fresh)==OW_OK);CHECK(old.block[0]==new_first && fresh.count==0 && a.live==3 && a.releases==3);CHECK(ow_destroy(&old)==OW_OK);CHECK(a.live==0);return 0;
}
static int move(void) {
 struct ow_allocator a={0};struct ow_owner src={0},dst={0};size_t lengths[3]={2,2,2};CHECK(ow_construct(&src,ow_allocator(&a),lengths)==OW_OK);
 CHECK(ow_move(&dst,&src)==OW_OK);CHECK(src.count==0 && src.block[0]==NULL && dst.count==3);CHECK(ow_consume(&src)==OW_DEAD);CHECK(ow_consume(&dst)==OW_OK);CHECK(ow_consume(&dst)==OW_DEAD);CHECK(a.releases==3 && a.invalid_releases==0);
 CHECK(ow_construct(&src,ow_allocator(&a),lengths)==OW_OK);CHECK(ow_construct(&dst,ow_allocator(&a),lengths)==OW_OK);int32_t *old=dst.block[0];CHECK(ow_move(&dst,&src)==OW_INVALID);CHECK(dst.block[0]==old && src.count==3);CHECK(ow_destroy(&src)==OW_OK);CHECK(ow_destroy(&dst)==OW_OK);return 0;
}
static int borrows(void) {
 struct ow_borrows b={.extent=8};struct ow_token a,z={3,99},out[2];
 CHECK(ow_borrow(&b,0,8,1,&a)==OW_OK);CHECK(ow_borrow(&b,3,1,0,&z)==OW_BUSY);CHECK(z.serial==99);
 CHECK(ow_split(&b,a,3,out)==OW_OK);CHECK(ow_release(&b,a)==OW_STALE);CHECK(b.loan[out[0].slot].length==3 && b.loan[out[1].slot].offset==3);
 CHECK(ow_release(&b,out[0])==OW_OK);CHECK(ow_release(&b,out[0])==OW_STALE);CHECK(ow_release(&b,out[1])==OW_OK);
 CHECK(ow_borrow(&b,SIZE_MAX,1,0,&z)==OW_INVALID);CHECK(ow_borrow(&b,8,0,1,&z)==OW_OK);CHECK(ow_release(&b,z)==OW_OK);
 b.loan[0].serial=UINT32_MAX;CHECK(ow_borrow(&b,0,1,0,&z)==OW_OK);CHECK(z.slot!=0);
 struct ow_token x[3];for(unsigned i=0;i<3;i++) { if(i==0)x[i]=z;else CHECK(ow_borrow(&b,0,1,0,&x[i])==OW_OK); }
 CHECK(ow_borrow(&b,0,1,0,&z)==OW_FULL);
 struct ow_borrows full={.extent=8};struct ow_token parent,others[3],children[2]={{3,90},{3,91}};
 CHECK(ow_borrow(&full,0,4,1,&parent)==OW_OK);for(unsigned i=0;i<3;i++)CHECK(ow_borrow(&full,4,1,0,&others[i])==OW_OK);
 CHECK(ow_split(&full,parent,2,children)==OW_FULL);CHECK(children[0].serial==90 && children[1].serial==91);CHECK(ow_release(&full,parent)==OW_OK);for(unsigned i=0;i<3;i++)CHECK(ow_release(&full,others[i])==OW_OK);return 0;
}
static int cell(void) {
 struct ow_cell c={.value=9,.borrows={.extent=1}};struct ow_token read;int32_t old=71;
 CHECK(ow_borrow(&c.borrows,0,1,0,&read)==OW_OK);CHECK(ow_cell_replace(&c,4,&old)==OW_BUSY);CHECK(old==71 && c.value==9);CHECK(ow_cell_write(&c,read,7)==OW_INVALID);CHECK(ow_cell_read(&c,read,&old)==OW_OK && old==9);
 CHECK(ow_release(&c.borrows,read)==OW_OK);CHECK(ow_cell_read(&c,read,&old)==OW_STALE);CHECK(ow_cell_replace(&c,4,&old)==OW_OK && old==9 && c.value==4);return 0;
}
struct ow_counts {atomic_uint destroyed,released;};
static void destroyed(void *p){struct ow_counts *c=p;(void)atomic_fetch_add(&c->destroyed,1);}
static void released(void *p){struct ow_counts *c=p;(void)atomic_fetch_add(&c->released,1);}
struct ow_worker {struct ow_atomic_shared *shared;unsigned failed;};
static void *worker(void *p) {
 struct ow_worker *w=p;
 for(unsigned i=0;i<5000;i++) {
  if(ow_atomic_retain(w->shared)!=OW_OK || ow_atomic_drop(w->shared)!=OW_OK){w->failed=1;return NULL;}
 }
 if(ow_atomic_drop(w->shared)!=OW_OK)w->failed=1;
 return NULL;
}
static int shared(void) {
 unsigned n=0;struct ow_shared s={1,1,append,append,&n};CHECK(ow_shared_downgrade(&s)==OW_OK);CHECK(ow_shared_retain(&s)==OW_OK);CHECK(ow_shared_drop(&s)==OW_OK && n==0);CHECK(ow_shared_drop(&s)==OW_OK && n==1);CHECK(s.weak==1);CHECK(ow_shared_upgrade(&s)==OW_DEAD);CHECK(ow_shared_weak_drop(&s)==OW_OK && n==2);
 struct ow_counts counts;atomic_init(&counts.destroyed,0);atomic_init(&counts.released,0);struct ow_atomic_shared a;atomic_init(&a.strong,5);atomic_init(&a.weak,2);a.destroy=destroyed;a.release=released;a.context=&counts;
 pthread_t threads[4];struct ow_worker w[4];for(unsigned i=0;i<4;i++){w[i]=(struct ow_worker){&a,0};CHECK(pthread_create(&threads[i],NULL,worker,&w[i])==0);}
 CHECK(ow_atomic_drop(&a)==OW_OK);
 for(unsigned i=0;i<4;i++){CHECK(pthread_join(threads[i],NULL)==0);CHECK(!w[i].failed);}
 CHECK(atomic_load(&counts.destroyed)==1 && atomic_load(&counts.released)==0);CHECK(ow_atomic_upgrade(&a)==OW_DEAD);CHECK(ow_atomic_weak_drop(&a)==OW_OK);CHECK(atomic_load(&counts.released)==1);
 printf("atomic_uint size=%zu alignment=%zu lock_free=%d; 20000 retain/drop pairs\n",sizeof(atomic_uint),_Alignof(atomic_uint),atomic_is_lock_free(&a.strong));return 0;
}
static int cow(void) {
 struct ow_allocator a={0};struct v4_allocator al=ow_allocator(&a);int32_t *data=al.take(al.context,3);CHECK(data!=NULL);data[0]=1;data[1]=2;data[2]=3;
 struct ow_cow old={1,data,3,al},fresh={0};struct ow_cow *owner=&old;CHECK(ow_cow_clone(&old)==OW_OK);a.fail_at=2;
 CHECK(ow_cow_write(&owner,&fresh,1,9)==OW_NOMEM);CHECK(owner==&old && old.refs==2 && old.data[1]==2 && fresh.refs==0);
 a.fail_at=0;CHECK(ow_cow_write(&owner,&fresh,1,9)==OW_OK);CHECK(owner==&fresh && old.data[1]==2 && fresh.data[1]==9 && old.refs==1);CHECK(ow_cow_drop(&old)==OW_OK);CHECK(ow_cow_drop(&fresh)==OW_OK);CHECK(a.live==0);return 0;
}
static int pin(void) {
 int old=0,new_value=0;struct ow_pin p;CHECK(ow_pin_init(&p,&old,7)==OW_OK);CHECK(ow_pin_check(&p,&old,8)==OW_INVALID);CHECK(ow_pin_check(&p,&new_value,7)==OW_STALE);CHECK(ow_pin_submit(&p,&old,7)==OW_OK);CHECK(ow_pin_cancel(&p,7)==OW_OK);CHECK(ow_pin_rebind(&p,&new_value,7)==OW_BUSY);CHECK(p.address==&old);CHECK(ow_pin_complete(&p,7)==OW_OK);CHECK(ow_pin_rebind(&p,&new_value,7)==OW_OK);CHECK(ow_pin_complete(&p,7)==OW_INVALID);return 0;
}
static int allocator(void) {
 struct ow_allocator a={0};struct v4_allocator al=ow_allocator(&a);a.fail_at=2;int32_t *p=al.take(al.context,16);CHECK(p!=NULL);CHECK(al.take(al.context,1)==NULL);CHECK(a.live==1 && a.peak==1);p[15]=19;al.release(al.context,p);CHECK(p[15]==INT32_MIN && a.live==0 && a.releases==1);al.release(al.context,p);CHECK(a.invalid_releases==1);
 a.attempts=SIZE_MAX;CHECK(al.take(al.context,1)==NULL && a.exhausted_counters==1 && a.attempts==SIZE_MAX);return 0;
}
static int facade(void) {
 struct ow_tenant_arena a;for(unsigned i=0;i<16;i++)a.data[i]=(i<8)?1:99;
 CHECK(ow_flawed_write(&a,0,UINT_MAX-3u,8,7)==OW_OK);CHECK(a.data[12]==7); /* harmless cross-tenant corruption */
 for(unsigned i=0;i<16;i++)a.data[i]=(i<8)?1:99;
 CHECK(ow_checked_write(&a,0,UINT_MAX-3u,8,7)==OW_INVALID);for(unsigned i=0;i<16;i++)CHECK(a.data[i]==(i<8?1:99));
 CHECK(ow_checked_write(&a,0,8,0,7)==OW_OK);CHECK(ow_checked_write(&a,0,8,1,7)==OW_INVALID && a.data[8]==99);CHECK(ow_checked_write(&a,1,7,1,7)==OW_OK && a.data[15]==7);CHECK(ow_checked_write(&a,1,7,2,7)==OW_INVALID);return 0;
}
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 const char *names[]={"frontier","scope","move","borrows","cell","shared","cow","pin","allocator","facade"};
 int (*tests[])(void)={frontier,scope,move,borrows,cell,shared,cow,pin,allocator,facade};
 for(unsigned i=0;i<10;i++)if(strcmp(argv[1],names[i])==0)return tests[i]();
 return 2;
}
