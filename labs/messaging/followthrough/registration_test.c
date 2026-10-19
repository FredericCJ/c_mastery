#define _POSIX_C_SOURCE 200809L
#include "registration.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"SC_CHECK:%d:%s\n",__LINE__,#x);exit(1);}} while(0)
static void inject(void *p) {CHECK(mfr_send(p,1,77)==MSG_OK);}
typedef struct {MfrHub *h;unsigned k;} Producer;
static void *produce(void *p) {
    Producer *a=p;
    for (unsigned i=0;i<1000;i++) {
        MsgStatus s;
        do {s=mfr_send(a->h,a->k,a->k*1000+i);if (s==MSG_FULL) sched_yield();} while(s==MSG_FULL);
        CHECK(s==MSG_OK);
    }
    CHECK(mfr_close(a->h,a->k)==MSG_OK);return NULL;
}
static void *wait_cancel(void *p) {
    unsigned k=13,v=17;CHECK(mfr_select(p,3,true,&k,&v)==MSG_CANCELLED);
    CHECK(k==13 && v==17);return NULL;
}
int main(void) {
    MfrHub h;unsigned k=91,v=92;
    CHECK(mfr_init(&h,0,1)==MSG_INVALID);CHECK(mfr_init(&h,4,4)==MSG_OK);
    CHECK(mfr_select(&h,0,false,&k,&v)==MSG_INVALID && k==91 && v==92);
    CHECK(mfr_select(&h,3,false,&k,&v)==MSG_EMPTY && k==91 && v==92);
    CHECK(mfr_send(&h,0,3)==MSG_OK && mfr_send(&h,1,5)==MSG_OK);
    CHECK(mfr_select(&h,3,false,&k,&v)==MSG_OK && k==0 && v==3);
    CHECK(mfr_select(&h,3,false,&k,&v)==MSG_OK && k==1 && v==5);
    struct timespec deadline;CHECK(clock_gettime(CLOCK_REALTIME,&deadline)==0);
    deadline.tv_nsec+=2000000;if (deadline.tv_nsec>=1000000000L) {++deadline.tv_sec;deadline.tv_nsec-=1000000000L;}
    CHECK(mfr_select_probe(&h,3,&k,&v,&deadline,inject,&h)==MSG_OK && k==1 && v==77);
    size_t waiting;uint64_t sleeps;mfr_observe(&h,&waiting,&sleeps);
    CHECK(waiting==0 && sleeps==0); /* Publication after scan must prevent sleeping. */
    CHECK(mfr_send(&h,0,11)==MSG_OK && mfr_close(&h,0)==MSG_OK);
    CHECK(mfr_select(&h,1,false,&k,&v)==MSG_OK && k==0 && v==11);
    CHECK(mfr_select(&h,1,false,&k,&v)==MSG_CLOSED);
    CHECK(mfr_send(&h,0,1)==MSG_CLOSED);CHECK(mfr_destroy(&h)==MSG_OK);
    CHECK(mfr_init(&h,4,4)==MSG_OK);Producer a[2]={{&h,0},{&h,1}};pthread_t p[2];
    CHECK(pthread_create(&p[0],NULL,produce,&a[0])==0);CHECK(pthread_create(&p[1],NULL,produce,&a[1])==0);
    unsigned seen[2000]={0};size_t count=0;
    for (;;) {
        MsgStatus s=mfr_select(&h,3,true,&k,&v);if (s==MSG_CLOSED) break;
        CHECK(s==MSG_OK && v<2000 && k==v/1000 && seen[v]++==0);++count;
    }
    CHECK(count==2000);CHECK(pthread_join(p[0],NULL)==0 && pthread_join(p[1],NULL)==0);
    CHECK(mfr_destroy(&h)==MSG_OK);
    CHECK(mfr_init(&h,1,1)==MSG_OK);pthread_t waiter;
    CHECK(pthread_create(&waiter,NULL,wait_cancel,&h)==0);
    do {mfr_observe(&h,&waiting,&sleeps);if (!waiting) sched_yield();} while(!waiting);
    CHECK(mfr_destroy(&h)==MSG_BUSY);mfr_cancel(&h);
    CHECK(pthread_join(waiter,NULL)==0);CHECK(mfr_destroy(&h)==MSG_OK);
    puts("registered select: wake window, closed drain, 2000 distinct values and cancellation passed");return 0;
}
