#define _GNU_SOURCE
#include "bridge.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"P5_CHECK line=%d %s\n",__LINE__,#x);exit(1);}}while(0)
static struct timespec until(unsigned seconds){struct timespec t;CHECK(!clock_gettime(CLOCK_MONOTONIC,&t));t.tv_sec+=seconds;return t;}
static void read_progress(P5Bridge *p,size_t expected){
 struct timespec t=until(3);CHECK(!pthread_mutex_lock(&p->mutex));
 while(p->used<expected)CHECK(!pthread_cond_timedwait(&p->changed,&p->mutex,&t));
 CHECK(!memcmp(p->bytes,"ABC",expected));CHECK(!pthread_mutex_unlock(&p->mutex));
}
int main(void){
 int fds[2];CHECK(!pipe2(fds,O_NONBLOCK|O_CLOEXEC));P5Bridge p;
 CHECK(!p5_bridge_init(&p,fds[0],1));CHECK(!close(fds[0])); /* duplicate is owned */
 CHECK(!p5_bridge_offer(&p,1) && !p5_bridge_offer(&p,2));uint32_t g=99;
 CHECK(!p5_bridge_begin(&p,&g) && g==1);CHECK(write(fds[1],"ABC",3)==3);read_progress(&p,3);
 CHECK(p5_bridge_cancel(&p,g)==1 && p5_bridge_cancel(&p,g)==1);
 struct timespec now=until(0);CHECK(p5_bridge_wait(&p,&now)==1);
 CHECK(p5_bridge_observe(&p,g)==1 && p5_bridge_reap(&p)==1 && p5_bridge_destroy(&p)==1);
 CHECK(!pthread_mutex_lock(&p.mutex));CHECK(p.queue.count==2 && p.operation.releases==0 && p.discarded==0 && p.callbacks==0 && !p.quiescent);CHECK(!pthread_mutex_unlock(&p.mutex));
 CHECK(!p5_bridge_release_gate(&p));struct timespec limit=until(3);CHECK(!p5_bridge_wait(&p,&limit));
 CHECK(!p5_bridge_observe(&p,g) && !p5_bridge_observe(&p,g));
 CHECK(p.operation.releases==1 && p.discarded==2 && p.queue.count==0 && p.callbacks==1 && p.joins==0);
 CHECK(p5_bridge_destroy(&p)==1 && fcntl(p.fd,F_GETFD)>=0);CHECK(!p5_bridge_reap(&p) && p.joins==1);
 CHECK(!p5_bridge_offer(&p,3));uint32_t next=99;CHECK(!p5_bridge_begin(&p,&next) && next==2);
 CHECK(p5_bridge_cancel(&p,g)==-1 && p5_bridge_cancel(&p,next)==1);
 CHECK(p5_bridge_observe(&p,g)==-1 && p.operation.releases==1);
 CHECK(!p5_bridge_release_gate(&p));limit=until(3);CHECK(!p5_bridge_wait(&p,&limit));
 CHECK(!p5_bridge_observe(&p,next) && p.operation.releases==2 && p.discarded==3 && p.callbacks==2);
 CHECK(!p5_bridge_reap(&p) && p.joins==2);
 p.operation.generation=UINT32_MAX;next=99;CHECK(p5_bridge_begin(&p,&next)==-1 && next==99);
 CHECK(!p5_bridge_destroy(&p) && !p.initialized && !close(fds[1]));
 /* Synchronous original composition: false completion retains both items. */
 algo_ring q;FAsync op={F_IDLE,0,0};uint32_t ticket;unsigned discarded=0;
 CHECK(algo_ring_init(&q,4)==ALGO_OK && algo_ring_put(&q,1)==ALGO_OK && algo_ring_put(&q,2)==ALGO_OK && f_submit(&op,&ticket)==F_OK);
 CHECK(in_shutdown(&q,&op,0,ticket,&discarded)==1 && q.count==2 && !op.releases);
 CHECK(in_shutdown(&q,&op,1,ticket,&discarded)==0 && q.count==0 && discarded==2 && op.releases==1);
 puts("P5_BRIDGE_PASS real_pipe=3 callbacks=2 joins=2 retained_deadline=1 exact_FAsync_import");
}
