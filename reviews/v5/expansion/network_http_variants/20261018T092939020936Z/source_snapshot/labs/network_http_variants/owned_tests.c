#define _POSIX_C_SOURCE 200809L
#include "owned.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NV_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct {pthread_mutex_t m;pthread_cond_t c;unsigned ready,go;} Gate;
static void gate_init(Gate *g){CHECK(!pthread_mutex_init(&g->m,0));CHECK(!pthread_cond_init(&g->c,0));g->ready=g->go=0;}
static void gate_wait(Gate *g){pthread_mutex_lock(&g->m);g->ready++;pthread_cond_broadcast(&g->c);while(!g->go)pthread_cond_wait(&g->c,&g->m);pthread_mutex_unlock(&g->m);}
static void gate_go(Gate *g,unsigned ready){pthread_mutex_lock(&g->m);while(g->ready<ready)pthread_cond_wait(&g->c,&g->m);g->go=1;pthread_cond_broadcast(&g->c);pthread_mutex_unlock(&g->m);}
static void gate_destroy(Gate *g){CHECK(!pthread_cond_destroy(&g->c));CHECK(!pthread_mutex_destroy(&g->m));}
static int descriptor(void){int p[2];CHECK(!pipe(p));CHECK(!close(p[1]));CHECK(!fcntl(p[0],F_SETFL,O_NONBLOCK));CHECK(!fcntl(p[0],F_SETFD,FD_CLOEXEC));return p[0];}
typedef struct {NVOwned *pool;HFHandle handle;Gate *gate;int result,fd,op;} Work;
static void *worker(void *v){Work *w=v;int previous;CHECK(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&previous));
  if(w->op==0){NVLease l={0};CHECK(!nv_owned_borrow(w->pool,w->handle,&l));w->fd=l.fd;gate_wait(w->gate);CHECK(fcntl(l.fd,F_GETFD)>=0);CHECK(!nv_owned_release(&l));CHECK(nv_owned_release(&l)==NV_STALE);w->result=0;}
  else {gate_wait(w->gate);if(w->op==1)w->result=nv_owned_close(w->pool,w->handle);else w->result=nv_owned_admit(w->pool,w->fd,&w->handle);}
  return 0;
}
int main(void){int previous;CHECK(!pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&previous));NVOwned p;CHECK(!nv_owned_init(&p));HFHandle h[4];
  for(unsigned i=0;i<4;i++){int fd=descriptor();CHECK(!nv_owned_admit(&p,fd,&h[i]));}
  CHECK(nv_owned_destroy(&p)==NV_BUSY);Gate g;gate_init(&g);Work w={&p,h[0],&g,-1,-1,0};pthread_t t;CHECK(!pthread_create(&t,0,worker,&w));
  pthread_mutex_lock(&g.m);while(!g.ready)pthread_cond_wait(&g.c,&g.m);pthread_mutex_unlock(&g.m);
  CHECK(!nv_owned_close(&p,h[0]));CHECK(nv_owned_close(&p,h[0])==NV_STALE);NVLease untouched={0};CHECK(nv_owned_borrow(&p,h[0],&untouched)==NV_STALE && !untouched.active);
  int candidate=descriptor();HFHandle next={99,99};CHECK(nv_owned_admit(&p,candidate,&next)==NV_FULL && next.index==99);CHECK(fcntl(candidate,F_GETFD)>=0);
  gate_go(&g,1);CHECK(!pthread_join(t,0) && !w.result);gate_destroy(&g);errno=0;CHECK(fcntl(w.fd,F_GETFD)==-1 && errno==EBADF);
  CHECK(!nv_owned_admit(&p,candidate,&next));CHECK(next.index==h[0].index && next.generation!=h[0].generation);CHECK(nv_owned_close(&p,h[0])==NV_STALE);h[0]=next;
  for(unsigned i=0;i<200;i++){
    gate_init(&g);int fd=descriptor();Work close_work={&p,h[0],&g,-1,-1,1},admit_work={&p,{99,99},&g,-1,fd,2};pthread_t a,b;
    CHECK(!pthread_create(&a,0,worker,&close_work));CHECK(!pthread_create(&b,0,worker,&admit_work));gate_go(&g,2);CHECK(!pthread_join(a,0));CHECK(!pthread_join(b,0));gate_destroy(&g);
    CHECK(close_work.result==NV_OK);CHECK(admit_work.result==NV_OK || admit_work.result==NV_FULL);
    if(admit_work.result==NV_FULL){CHECK(admit_work.handle.index==99 && fcntl(fd,F_GETFD)>=0);CHECK(!nv_owned_admit(&p,fd,&admit_work.handle));}
    CHECK(admit_work.handle.index==h[0].index && admit_work.handle.generation==h[0].generation+1);CHECK(nv_owned_close(&p,h[0])==NV_STALE);h[0]=admit_work.handle;
  }
  for(unsigned i=0;i<4;i++)CHECK(!nv_owned_close(&p,h[i]));CHECK(!nv_owned_destroy(&p));
  NVOwned q;CHECK(!nv_owned_init(&q));for(unsigned i=0;i<4;i++)q.core.slot[i].generation=UINT32_MAX;
  candidate=descriptor();next=(HFHandle){99,99};CHECK(nv_owned_admit(&q,candidate,&next)==NV_FULL && next.index==99 && fcntl(candidate,F_GETFD)>=0);CHECK(!close(candidate));CHECK(!nv_owned_destroy(&q));
  puts("NV_OK:owned:held-borrow,close-admit-200,caller-retention,retirement,joined-destroy");return 0;
}
