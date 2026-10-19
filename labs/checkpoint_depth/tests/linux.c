#define _GNU_SOURCE
#include "checkpoint_linux.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CP_CHECK: %s:%d %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
static void send_frame(int fd,unsigned n){
    CPMessage m={.sequence=(uint16_t)n,.length=3,.payload={0x31,0x32,0x33}};unsigned char wire[CP_WIRE];size_t bytes;
    CHECK(cp_encode(&m,wire,sizeof wire,&bytes)==CP_OK);CHECK(write(fd,wire,bytes)==(ssize_t)bytes);
}
static void take(CPAQueue *q,unsigned seq){CPAHandle h;CPMessage m;CHECK(cpa_take(q,&h,&m,0)==CPA_OK && m.sequence==seq && m.payload[0]==0x31);CHECK(cpa_finish(q,h,0)==CPA_OK);}
static CPLStatus pump(CPLReader *r,uint64_t token,CPAQueue *q){for(unsigned i=0;i<16;++i){CPLStatus s=cpl_pump(r,token,q);if(s!=CPL_PROGRESS)return s;}CHECK(0);return CPL_ERROR;}
typedef struct {CPAQueue *queue;pthread_mutex_t mutex;unsigned seen[64],total;} Workers;
static void *worker(void *argument){
    Workers *w=argument;
    for(;;){
        CPAHandle handle;CPMessage message;CPAStatus status=cpa_take(w->queue,&handle,&message,1);
        if(status==CPA_CLOSED)return 0;
        CHECK(status==CPA_OK && message.sequence<64 && message.payload[0]==0x31);
        CHECK(!pthread_mutex_lock(&w->mutex));++w->seen[message.sequence];++w->total;CHECK(!pthread_mutex_unlock(&w->mutex));
        CHECK(cpa_finish(w->queue,handle,0)==CPA_OK);
    }
}
static void pipeline(void){
    int epfd=epoll_create1(EPOLL_CLOEXEC),p[2];CHECK(epfd>=0 && !pipe2(p,O_NONBLOCK|O_CLOEXEC));
    CPAQueue q;CPLReader r;CHECK(cpa_init(&q,128)==CPA_OK);cpl_init(&r,3,2);CHECK(!cpl_open(&r,epfd,p[0],1));
    Workers workers={.queue=&q};CHECK(!pthread_mutex_init(&workers.mutex,0));pthread_t ids[2];
    for(unsigned i=0;i<2;++i)CHECK(!pthread_create(&ids[i],0,worker,&workers));
    /* 832 bytes fit the Linux pipe minimum; no remote sender or network target. */
    for(unsigned i=0;i<64;++i)send_frame(p[1],i);
    CHECK(!close(p[1]));struct epoll_event event;CHECK(epoll_wait(epfd,&event,1,1000)==1);
    CPLStatus status=CPL_PROGRESS;
    for(unsigned retries=0;retries<10000 && status!=CPL_EOF;++retries){
        status=pump(&r,event.data.u64,&q);CHECK(status==CPL_BLOCKED || status==CPL_EOF);
        if(status==CPL_BLOCKED){struct timespec pause={0,100000};(void)nanosleep(&pause,0);}
    }
    CHECK(status==CPL_EOF);CPAStatus closed=cpa_close(&q,0);CHECK(closed==CPA_CLOSED || closed==CPA_PENDING);
    for(unsigned i=0;i<2;++i)CHECK(!pthread_join(ids[i],0));
    CHECK(workers.total==64 && q.accepted==64 && q.completed==64 && q.releases==64 && !q.discarded);
    for(unsigned i=0;i<64;++i)CHECK(workers.seen[i]==1);
    CHECK(cpa_destroy(&q)==CPA_OK && !pthread_mutex_destroy(&workers.mutex));CHECK(!cpl_close(&r) && !close(epfd));
}
void cp_test_linux_reader(void){
    for(int edge=0;edge<=1;++edge){
        int descriptors[2],epfd=epoll_create1(EPOLL_CLOEXEC);CHECK(epfd>=0 && !pipe2(descriptors,O_NONBLOCK|O_CLOEXEC));
        CPAQueue q;CHECK(cpa_init(&q,32)==CPA_OK);CPLReader r;cpl_init(&r,7,3);
        CHECK(!cpl_open(&r,epfd,descriptors[0],edge));uint64_t old=cpl_token(&r);
        send_frame(descriptors[1],1);send_frame(descriptors[1],2);send_frame(descriptors[1],3);
        struct epoll_event event;CHECK(epoll_wait(epfd,&event,1,1000)==1 && event.data.u64==old);
        CHECK(pump(&r,event.data.u64,&q)==CPL_BLOCKED && q.count==2 && r.parser.ready);
        /* Kernel pipe is drained, but parser-held frame still needs explicit resumption. */
        CHECK(epoll_wait(epfd,&event,1,0)==0);take(&q,1);
        CHECK(pump(&r,old,&q)==CPL_DRAINED && !r.parser.ready);take(&q,2);take(&q,3);
        int previous_fd=r.fd;CHECK(!cpl_close(&r));CHECK(!close(descriptors[1]));
        CHECK(!pipe2(descriptors,O_NONBLOCK|O_CLOEXEC));
        if(descriptors[0]!=previous_fd){CHECK(dup2(descriptors[0],previous_fd)==previous_fd);CHECK(!close(descriptors[0]));descriptors[0]=previous_fd;}
        CHECK(!cpl_open(&r,epfd,descriptors[0],edge) && r.fd==previous_fd && cpl_token(&r)!=old);
        send_frame(descriptors[1],4);
        CHECK(cpl_pump(&r,old,&q)==CPL_STALE && q.count==0);
        CHECK(pump(&r,cpl_token(&r),&q)==CPL_DRAINED);take(&q,4);
        CHECK(!close(descriptors[1]));CHECK(pump(&r,cpl_token(&r),&q)==CPL_EOF);
        CHECK(!cpl_close(&r));CHECK(cpa_close(&q,0)==CPA_CLOSED && cpa_destroy(&q)==CPA_OK);CHECK(!close(epfd));
    }
    /* Truncated EOF and generation exhaustion are distinct from ordinary EOF/reuse. */
    int epfd=epoll_create1(EPOLL_CLOEXEC),p[2];CHECK(epfd>=0 && !pipe2(p,O_NONBLOCK|O_CLOEXEC));
    CPLReader r;cpl_init(&r,0,1);CPAQueue q;CHECK(cpa_init(&q,2)==CPA_OK && !cpl_open(&r,epfd,p[0],1));
    const unsigned char prefix[]={0xa5,1,2};CHECK(write(p[1],prefix,sizeof prefix)==(ssize_t)sizeof prefix && !close(p[1]));
    CHECK(pump(&r,cpl_token(&r),&q)==CPL_TRUNCATED && q.accepted==0);CHECK(!cpl_close(&r) && r.retired);
    CHECK(cpl_open(&r,epfd,-1,1)==-1);CHECK(cpa_close(&q,0)==CPA_CLOSED && cpa_destroy(&q)==CPA_OK && !close(epfd));
    pipeline();
}
