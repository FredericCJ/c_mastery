#include "wrappers.h"
#include <limits.h>
#define CHECK(x) do{if(!(x)){aw_failure(#x,__LINE__);return 41;}}while(0)
static int same(const void *a,const void *b,size_t n){const unsigned char *x=a,*y=b;for(size_t i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static void copy(void *a,const void *b,size_t n){unsigned char *x=a;const unsigned char *y=b;for(size_t i=0;i<n;++i)x[i]=y[i];}
struct aw_pool { struct aw_bank banks[2];unsigned leased[2],attempts,releases,fail,reentrant,blocked,bad;struct aw_owner *owner; };
struct aw_compare { struct aw_context *order;struct aw_owner *owner;unsigned calls,fail,bad,reentrant,blocked; };
static struct aw_bank *acquire(void *arg){struct aw_pool *p=arg;++p->attempts;if(p->reentrant){struct aw_record r={99,0};p->blocked+=(aw_put(p->owner,&r)==AW_BUSY);}
 if(p->fail)return 0;
 for(unsigned i=0;i<2;++i)if(!p->leased[i]){p->leased[i]=1;return p->banks+i;}
 return 0;}
static void release(void *arg,struct aw_bank *bank){struct aw_pool *p=arg;for(unsigned i=0;i<2;++i)if(bank==p->banks+i && p->leased[i]){for(unsigned j=0;j<3;++j)if(bank->pointers[j])p->bad=1;p->leased[i]=0;++p->releases;return;}p->bad=1;}
static int compare(void *arg,const struct aw_record *a,const struct aw_record *b,int *out){struct aw_compare *c=arg;++c->calls;
 if(c->reentrant){struct aw_record r={88,0};c->blocked+=(aw_put(c->owner,&r)==AW_BUSY);return 1;}
 if(c->fail==c->calls)return 1;
 int rank=(a->rank>b->rank)-(a->rank<b->rank);*out=rank?(c->order->descending?-rank:rank):(a->id>b->id)-(a->id<b->id);
 if(c->bad)*out=1;
 return 0;
}
static enum aw_status init(struct aw_owner *o,struct aw_pool *p,struct aw_compare *c,struct aw_context *ctx,unsigned kind,unsigned cap,unsigned dispatch,unsigned limit){p->owner=o;c->owner=o;c->order=ctx;return aw_init(o,kind,cap,limit,ctx,dispatch,compare,c,(struct aw_backing){p,acquire,release});}
static int values(struct aw_owner *o,const int32_t *ids,const int32_t *ranks,unsigned count){CHECK(o->count==count && o->ids.count==count);int packet[7];packet[0]=(int)count;
 for(unsigned i=0;i<count;++i){struct aw_view v;struct aw_record r;CHECK(aw_view(o,i,&v)==AW_OK);CHECK(aw_read(v,&r)==AW_OK);CHECK(r.id==ids[i] && r.rank==ranks[i]);CHECK(cd_tree_find(&o->ids,r.id)>=0);packet[1+2*i]=r.id;packet[2+2*i]=r.rank;}
 aw_observe("ORDER",packet,1+2*count);return 0;}
static int layout(void){struct aw_record r[3]={{7,2},{4,1},{9,2}};int packet[]={sizeof(struct aw_record),(int)((unsigned char*)&r[1]-(unsigned char*)&r[0]),(int)((unsigned char*)&r[2]-(unsigned char*)&r[0]),sizeof(r),sizeof(struct cd_queue_node),sizeof(struct cd_queue),sizeof(struct aw_owner),sizeof(struct aw_bank)};aw_observe("LAYOUT",packet,8);CHECK(packet[0]==8 && packet[1]==8 && packet[2]==16 && packet[3]==24);
 struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};CHECK(init(&o,&p,&c,&ctx,AW_VALUE,3,AW_FINITE,3)==AW_OK);CHECK(aw_put(&o,r)==AW_OK);struct aw_view v;CHECK(aw_view(&o,0,&v)==AW_OK);CHECK(aw_remove(&o,7)==AW_OK);struct aw_record out={55,66};CHECK(aw_read(v,&out)==AW_STALE && out.id==55);unsigned char before[sizeof o];copy(before,&o,sizeof o);CHECK(aw_put(&o,r+1)==AW_EXHAUSTED && same(before,&o,sizeof o));CHECK(aw_destroy(&o)==AW_OK);CHECK(aw_read(v,&out)==AW_STALE);return 0;}
static int queues(void){struct cd_ring ring;int backing[3]={0};struct cd_queue q;cd_queue_init(&q);CHECK(cd_ring_init(&ring,backing,3)==CD_OK);const int ops[7]={11,22,0,33,44,55,0};unsigned scan_total=0;
 for(unsigned step=0;step<7;++step){int rv=-1,qv=-1;unsigned ring_head=(unsigned)ring.head,ring_count=(unsigned)ring.count;int old[3];copy(old,backing,sizeof old);unsigned char qb[sizeof q];copy(qb,&q,sizeof q);enum cd_status a,b;size_t done=77;
  if(ops[step]){unsigned scan=0;if(q.count<q.available){while(q.nodes[scan].live)++scan;++scan;}scan_total+=scan;a=cd_ring_push(&ring,ops+step,1,0,&done);b=cd_queue_push(&q,ops[step],0,0);if(step==5){CHECK(a==CD_FULL && b==CD_FULL && done==77);CHECK(ring.head==ring_head && ring.count==ring_count && same(old,backing,sizeof old) && same(qb,&q,sizeof q));}else CHECK(a==CD_OK && b==CD_OK && done==1);
  }else{a=cd_ring_pop(&ring,&rv,1);b=cd_queue_pop(&q,&qv);CHECK(a==CD_OK && b==CD_OK && rv==qv);CHECK(rv==(step==2?11:22));}
  CHECK(ring.count==q.count);int packet[24]={0};unsigned n=0;packet[n++]=(int)step;packet[n++]=ops[step];packet[n++]=(int)ring.head;packet[n++]=(int)ring.count;for(unsigned i=0;i<3;++i)packet[n++]=backing[i];packet[n++]=q.head;packet[n++]=q.tail;packet[n++]=(int)q.count;for(unsigned i=0;i<3;++i){packet[n++]=q.nodes[i].value;packet[n++]=q.nodes[i].next;packet[n++]=(int)q.nodes[i].live;}packet[n++]=(int)scan_total;aw_observe("QUEUE",packet,n);
  int at=q.head;for(unsigned i=0;i<q.count;++i){CHECK(at>=0 && at<3);CHECK(q.nodes[at].value==backing[(ring.head+i)%3]);at=q.nodes[at].next;}CHECK(at==-1);
 }
 CHECK(q.count==2 && backing[ring.head]==33 && backing[(ring.head+1)%3]==44 && scan_total==7);return 0;}
static int fixture(void){for(unsigned kind=0;kind<2;++kind)for(unsigned dispatch=0;dispatch<2;++dispatch){struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};struct aw_record r[4]={{7,2},{4,1},{9,2},{10,0}};CHECK(init(&o,&p,&c,&ctx,kind,3,dispatch,100)==AW_OK);
 CHECK(aw_put(&o,r)==AW_OK);CHECK(!values(&o,(int32_t[]){7},(int32_t[]){2},1));CHECK(aw_put(&o,r+1)==AW_OK);CHECK(!values(&o,(int32_t[]){4,7},(int32_t[]){1,2},2));CHECK(aw_put(&o,r+2)==AW_OK);CHECK(!values(&o,(int32_t[]){4,7,9},(int32_t[]){1,2,2},3));unsigned char old[sizeof o];copy(old,&o,sizeof o);unsigned attempts=p.attempts;
 CHECK(aw_put(&o,r+3)==AW_FULL);CHECK(!values(&o,(int32_t[]){4,7,9},(int32_t[]){1,2,2},3));CHECK(aw_put(&o,r)==AW_DUPLICATE);CHECK(!values(&o,(int32_t[]){4,7,9},(int32_t[]){1,2,2},3));CHECK(aw_put(&o,r)==AW_DUPLICATE && aw_put(&o,r+3)==AW_FULL);CHECK(same(old,&o,sizeof o) && p.attempts==attempts);CHECK(aw_remove(&o,4)==AW_OK);CHECK(!values(&o,(int32_t[]){7,9},(int32_t[]){2,2},2));CHECK(aw_destroy(&o)==AW_OK && !p.bad && !p.leased[0] && !p.leased[1] && !o.context && !o.compare && !o.compare_context && !o.backing.context);int packet[]={(int)kind,(int)dispatch,(int)p.attempts,(int)p.releases,(int)c.calls,0};aw_observe("FIXTURE",packet,6);
 }
 return 0;}
static int ownership(void){for(unsigned kind=0;kind<2;++kind){struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};struct aw_record r={7,2},out={99,99};CHECK(init(&o,&p,&c,&ctx,kind,1,AW_CALLBACK,100)==AW_OK && aw_put(&o,&r)==AW_OK);struct aw_view v;CHECK(aw_view(&o,0,&v)==AW_OK);r.rank=40;
 if(kind==AW_VALUE){CHECK(aw_read(v,&out)==AW_OK && out.id==7 && out.rank==2);}else{CHECK(aw_read(v,&out)==AW_MUTATED && out.id==99 && out.rank==99);r.rank=2;CHECK(aw_read(v,&out)==AW_OK && out.rank==2);}
 p.fail=1;unsigned char before[sizeof o];copy(before,&o,sizeof o);struct aw_bank *old=o.bank;CHECK(aw_reserve(&o,2)==AW_BACKING && same(before,&o,sizeof o));CHECK(aw_read(v,&out)==AW_OK);p.fail=0;CHECK(aw_reserve(&o,2)==AW_OK && old!=o.bank);CHECK(aw_read(v,&out)==AW_STALE);CHECK(aw_view(&o,0,&v)==AW_OK);CHECK(aw_reserve(&o,1)==AW_OK && aw_read(v,&out)==AW_OK);CHECK(aw_destroy(&o)==AW_OK && aw_read(v,&out)==AW_STALE);CHECK(!p.bad);
 }
 return 0;}
static int backing(void){for(unsigned kind=0;kind<2;++kind)for(unsigned fault=0;fault<5;++fault){struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};struct aw_record r[3]={{7,2},{4,1},{9,2}};CHECK(init(&o,&p,&c,&ctx,kind,2,AW_CALLBACK,100)==AW_OK);
 for(unsigned step=0;step<5;++step){unsigned char before[sizeof o],banks[sizeof p.banks];copy(before,&o,sizeof o);copy(banks,p.banks,sizeof banks);p.fail=step==fault;enum aw_status s=step<2?aw_put(&o,r+step):step==2?aw_reserve(&o,3):step==3?aw_put(&o,r+2):aw_remove(&o,4);
  if(p.fail){CHECK(s==AW_BACKING && same(before,&o,sizeof o) && same(banks,p.banks,sizeof banks));p.fail=0;s=step<2?aw_put(&o,r+step):step==2?aw_reserve(&o,3):step==3?aw_put(&o,r+2):aw_remove(&o,4);}
  CHECK(s==AW_OK);
 }CHECK(!values(&o,(int32_t[]){7,9},(int32_t[]){2,2},2));CHECK(aw_destroy(&o)==AW_OK && !p.bad && p.attempts==6 && p.releases==5);int packet[]={(int)kind,(int)fault,(int)p.attempts,(int)p.releases};aw_observe("BACKING",packet,4);
 }
 return 0;}
static int contexts(void){struct aw_record r[3]={{7,2},{4,1},{9,2}};for(unsigned fail=1;fail<=9;++fail){struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};CHECK(init(&o,&p,&c,&ctx,AW_VALUE,3,AW_CALLBACK,100)==AW_OK && aw_put(&o,r)==AW_OK && aw_put(&o,r+1)==AW_OK);unsigned char old[sizeof o];copy(old,&o,sizeof o);c.fail=c.calls+fail;CHECK(aw_put(&o,r+2)==AW_COMPARE && same(old,&o,sizeof o) && p.attempts==2);CHECK(aw_destroy(&o)==AW_OK);}
 struct aw_owner o;struct aw_pool p={0};struct aw_compare c={0};struct aw_context ctx={1,0};CHECK(init(&o,&p,&c,&ctx,AW_BORROW,3,AW_CALLBACK,100)==AW_OK);c.bad=1;CHECK(aw_put(&o,r)==AW_COMPARE && o.count==0);c.bad=0;c.reentrant=1;CHECK(aw_put(&o,r)==AW_COMPARE && c.blocked==1);c.reentrant=0;p.reentrant=1;CHECK(aw_put(&o,r)==AW_OK && p.blocked==1);p.reentrant=0;struct aw_view v;struct aw_record out={99,99};CHECK(aw_view(&o,0,&v)==AW_OK);ctx.epoch=2;ctx.descending=1;CHECK(aw_read(v,&out)==AW_COMPARE && out.id==99);CHECK(aw_set_context(&o,&ctx)==AW_COMPARE);ctx.epoch=1;ctx.descending=0;CHECK(aw_remove(&o,7)==AW_OK);ctx.epoch=2;ctx.descending=1;unsigned generation=o.generation;CHECK(aw_set_context(&o,&ctx)==AW_OK && o.generation==generation+1);CHECK(aw_put(&o,r)==AW_OK && aw_put(&o,r+1)==AW_OK && aw_put(&o,r+2)==AW_OK);CHECK(!values(&o,(int32_t[]){7,9,4},(int32_t[]){2,2,1},3));CHECK(aw_destroy(&o)==AW_OK);return 0;}
static int kernel(void){const int32_t keys[5]={INT32_MIN,1,2,3,INT32_MAX},ranks[7]={INT32_MIN,-2,-1,0,1,2,INT32_MAX};unsigned packets=0;
 for(unsigned code=0;code<27;++code)for(unsigned direction=0;direction<2;++direction){unsigned v=code;struct aw_record data[3];for(unsigned i=0;i<3;++i){data[i]=(struct aw_record){(int32_t)(i+1),((int)(v%3))-1};v/=3;}
 for(unsigned i=0;i<3;++i)for(unsigned j=i+1;j<3;++j){int less=direction?data[j].rank>data[i].rank:data[j].rank<data[i].rank;if(data[j].rank==data[i].rank)less=data[j].id<data[i].id;if(less){struct aw_record t=data[i];data[i]=data[j];data[j]=t;}}
 for(unsigned n=0;n<=3;++n)for(unsigned ki=0;ki<5;++ki)for(unsigned ri=0;ri<7;++ri){struct aw_record q={keys[ki],ranks[ri]};unsigned at=0;while(at<n){int less=direction?data[at].rank>q.rank:data[at].rank<q.rank;if(data[at].rank==q.rank)less=data[at].id<q.id;if(!less)break;++at;}struct aw_record before[3];copy(before,data,sizeof data);unsigned got=aw_lower(data,n,&q,direction);CHECK(got==at && same(before,data,sizeof data));int packet[]={(int)code,(int)direction,(int)n,q.id,q.rank,(int)got};aw_observe("KERNEL",packet,6);++packets;}
 CHECK(aw_probe(data,3,data,direction)==1);
 }CHECK(packets==7560);struct aw_record extremes[3]={{INT32_MIN,INT32_MIN},{0,0},{INT32_MAX,INT32_MAX}};CHECK(aw_lower(extremes,3,extremes+2,0)==2);return 0;}
static int name(const char *a,const char *b){while(*a && *a==*b){++a;++b;}return *a==*b;}
int aw_test(const char *n){if(name(n,"layout"))return layout();if(name(n,"queues"))return queues();if(name(n,"fixture"))return fixture();if(name(n,"ownership"))return ownership();if(name(n,"backing"))return backing();if(name(n,"contexts"))return contexts();if(name(n,"kernel"))return kernel();return 2;}
