#define _POSIX_C_SOURCE 200809L
#include "recovery.h"
#include "linux_port.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"MC_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct {
    int fds[16];unsigned live[16],next,opens,fail_open,retains,released_error,closes;
} Resources;
static int resource_open(void *context,unsigned ordinal,int *out) {
    Resources *r=context;(void)ordinal;++r->opens;
    if(r->fail_open==r->opens)return 0;
    CHECK(r->next<16);int fd=open("/dev/null",O_RDONLY);CHECK(fd>=0);
    unsigned id=r->next++;r->fds[id]=fd;r->live[id]=1;*out=(int)id;return 1;
}
static McClose resource_close(void *context,int id) {
    Resources *r=context;CHECK(id>=0 && (unsigned)id<r->next && r->live[id]);
    ++r->closes;
    if(r->retains){--r->retains;return MC_RETAINED;}
    CHECK(close(r->fds[id])==0);r->live[id]=0;
    if(r->released_error){--r->released_error;return MC_RELEASED_ERROR;}
    return MC_RELEASED;
}
static unsigned live_resources(Resources *r) {
    unsigned n=0;for(unsigned i=0;i<r->next;++i)n+=r->live[i];return n;
}
static void retire(McRecovery *s,OwAllocator *pool,Resources *r) {
    uint32_t token=0;unsigned errors=99;
    CHECK(mc_retire_current(s,&token));CHECK(mc_recover(s,token,&errors));
    CHECK(!errors && pool->live==0 && live_resources(r)==0 && !pool->invalid_releases);
    errors=99;CHECK(!mc_recover(s,token,&errors) && errors==99);
}
static void close_recovery(unsigned oracle) {
    /* Cartesian failure stages include allocation, open and retained-close.
       Resource IDs are monotonic; descriptor numbers are allowed to be reused. */
    for(unsigned allocation=0;allocation<=3;++allocation)
    for(unsigned open_failure=0;open_failure<=2;++open_failure) {
        Resources resources={0};OwAllocator pool={0};McRecovery state;
        CHECK(mc_recovery_init(&state,(McResources){&resources,resource_open,resource_close}));
        V4Allocator allocator=ow_allocator(&pool);
        McRecovered first=mc_publish_recoverable(&state,(const unsigned char *)"2:3",3,allocator);
        CHECK(first.publication.committed && !first.cleanup_pending && pool.live==3);
        McFive old=state.current;
        pool.fail_at=allocation?pool.attempts+allocation:0;
        resources.fail_open=open_failure?resources.opens+open_failure:0;
        resources.retains=2;
        McRecovered next=mc_publish_recoverable(&state,(const unsigned char *)"7:9",3,allocator);
        if(oracle)printf("{\"allocation\":%u,\"open\":%u,\"committed\":%u,\"pending\":%u,\"cells\":%zu,\"resources\":%u,\"sum\":%d,\"generation\":%u,\"attempts\":%zu,\"releases\":%zu,\"opens\":%u}\n",allocation,open_failure,(unsigned)next.publication.committed,next.cleanup_pending,pool.live,live_resources(&resources),state.current.cells.block[0][0],state.generation,pool.attempts,pool.releases,resources.opens);
        if(allocation || open_failure) {
            CHECK(!next.publication.committed);
            CHECK(state.current.cells.block[0]==old.cells.block[0] && old.cells.block[0][0]==5);
        } else {
            CHECK(next.publication.committed && next.cleanup_pending);
            CHECK(state.current.cells.block[0][0]==16 && pool.live==6 && live_resources(&resources)==4);
        }
        if(next.cleanup_pending) {
            unsigned errors=99;McFive published=state.current;
            CHECK(!mc_recover(&state,next.recovery_token-1,&errors) && errors==99);
            McRecovered blocked=mc_publish_recoverable(&state,(const unsigned char *)"1:1",3,allocator);
            CHECK(!blocked.publication.committed && state.current.cells.block[0]==published.cells.block[0]);
            resources.retains=0;
            CHECK(mc_recover(&state,next.recovery_token,&errors));
        }
        resources.retains=0;resources.fail_open=0;pool.fail_at=0;
        retire(&state,&pool,&resources);
    }
    Resources resources={0};OwAllocator pool={0};McRecovery state;
    CHECK(mc_recovery_init(&state,(McResources){&resources,resource_open,resource_close}));
    V4Allocator allocator=ow_allocator(&pool);
    CHECK(mc_publish_recoverable(&state,(const unsigned char *)"1:2",3,allocator).publication.committed);
    resources.released_error=1;
    McRecovered r=mc_publish_recoverable(&state,(const unsigned char *)"4:5",3,allocator);
    CHECK(r.publication.committed && !r.cleanup_pending && r.close_errors==1);
    CHECK(pool.live==3 && live_resources(&resources)==2);
    state.generation=UINT32_MAX-1;size_t reserved_attempts=pool.attempts;
    CHECK(!mc_publish_recoverable(&state,(const unsigned char *)"1:1",3,allocator).publication.committed);
    CHECK(pool.attempts==reserved_attempts);
    retire(&state,&pool,&resources);
    state.generation=UINT32_MAX;size_t attempts=pool.attempts;
    CHECK(!mc_publish_recoverable(&state,(const unsigned char *)"1:1",3,allocator).publication.committed);
    CHECK(pool.attempts==attempts);
    if(!oracle)printf("recoverable publication: 12 failure combinations, released-error and retirement boundaries\n");
}
typedef struct {McRead read;McLinuxPort port;} AsyncOwner;
typedef struct {
    AsyncOwner *owner;unsigned cleanups,receivers,freed,queued,consumed,discarded,joined,exits;
    MsgStatus status;unsigned progress,release;
} Observer;
static void cleanup(void *context) {
    Observer *o=context;McLinuxPort *p=&o->owner->port;
    int quiescent=mc_linux_quiescent(p);
    /* Contain a rejected implementation before exiting: preserve the observed
       failure, but reap its stopped worker so TSan's exit-time thread-leak
       diagnostic does not replace our intended semantic assertion status. */
    if(!quiescent && p->started)CHECK(pthread_join(p->worker,NULL)==0);
    CHECK(quiescent);
    o->queued=p->queued;o->consumed=p->consumed;o->discarded=p->discarded;
    o->joined=p->joined;o->exits=p->worker_exits;
    CHECK(o->queued==o->consumed+o->discarded && o->joined==o->exits);
    CHECK(mc_linux_destroy(p));++o->cleanups;
}
static void receiver(void *context,MsgStatus status,unsigned value) {
    Observer *o=context;CHECK(o->cleanups==1 && !o->receivers);
    ++o->receivers;o->status=status;o->progress=value;
    if(o->release){free(o->owner);o->owner=NULL;o->freed=1;}
}
static void real_io(void) {
    for(unsigned repeat=0;repeat<16;++repeat)
    for(unsigned mode=0;mode<5;++mode)for(unsigned release=0;release<2;++release) {
        int pair[2];CHECK(socketpair(AF_UNIX,SOCK_STREAM,0,pair)==0);
        CHECK(fcntl(pair[0],F_SETFL,O_NONBLOCK)==0);
        Observer o={0};o.release=release;o.owner=calloc(1,sizeof(*o.owner));CHECK(o.owner);
        AsyncOwner *a=o.owner;unsigned char out[8];memset(out,0x5a,sizeof(out));
        CHECK(mc_linux_init(&a->port,&a->read,pair[0]));
        CHECK(mc_read_init(&a->read,mc_linux_interface(&a->port),3,19,out,receiver,&o,cleanup,&o)==MSG_OK);
        if(mode==4){CHECK(write(pair[1],"abc",3)==3);CHECK(mc_read_poll(&a->read)==MSG_OK);}
        else {
            CHECK(mc_read_poll(&a->read)==MSG_BUSY);
            if(mode!=0) {
                CHECK(write(pair[1],mode==1?"abc":"a",mode==1?3:1)==(mode==1?3:1));
                CHECK(mc_linux_wait(&a->port,5000));
            }
            if(mode<2) {
                CHECK(mc_linux_request_cancel(&a->port));
                CHECK(o.cleanups==0 && o.receivers==0);
                CHECK(!mc_linux_dispatch(&a->port));
                CHECK(mc_linux_finish_cancel(&a->port)==MSG_CANCELLED);
                CHECK(o.status==MSG_CANCELLED && o.progress==0 && out[0]==0x5a);
                if(mode==1)CHECK(o.discarded>=1);
            } else {
                CHECK(mc_linux_dispatch(&a->port));
                if(mode==3) {
                    CHECK(shutdown(pair[1],SHUT_WR)==0);
                    MsgStatus status=mc_read_poll(&a->read);
                    if(status==MSG_BUSY) {
                        CHECK(mc_linux_wait(&a->port,5000));
                        CHECK(mc_linux_dispatch(&a->port));
                        status=mc_read_poll(&a->read);
                    }
                    CHECK(status==MSG_FAILED);
                    CHECK(o.status==MSG_FAILED && o.progress==1 && out[0]==0x5a);
                } else {
                    CHECK(mc_read_poll(&a->read)==MSG_BUSY);
                    CHECK(write(pair[1],"bc",2)==2);
                    CHECK(mc_linux_wait(&a->port,5000));
                    CHECK(mc_linux_dispatch(&a->port));
                    CHECK(mc_read_poll(&a->read)==MSG_OK);
                }
            }
        }
        if(mode==2 || mode==4)CHECK(o.status==MSG_OK && o.progress==3 && !memcmp(out,"abc",3) && out[3]==0x5a);
        CHECK(o.cleanups==1 && o.receivers==1 && o.freed==release);
        if(!release)free(o.owner);
        CHECK(close(pair[0])==0 && close(pair[1])==0);
    }
    printf("real socket backend: 160 completion/cancellation/EOF/inline-destruction histories\n");
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    if(!strcmp(argv[1],"publish_close"))close_recovery(0);
    else if(!strcmp(argv[1],"oracle"))close_recovery(1);
    else if(!strcmp(argv[1],"real_io"))real_io();
    else CHECK(0);
    return 0;
}
