#define _POSIX_C_SOURCE 200809L
#include "lifecycle.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TL_ASSERT:%s:%d:%s\n",__FILE__,__LINE__,#x);exit(41);}}while(0)
struct join_script {unsigned calls,fail;};
static int scripted(pthread_t t,void **out,void *arg){struct join_script *s=arg;s->calls++;if(s->calls==s->fail)return EBUSY;return pthread_join(t,out);}
static void pool(void){
    int previous;CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&previous)==0);
    int e=99;CHECK(!tl_pool_start(5,0,0,&e)&&e==EINVAL);CHECK(!tl_pool_start(2,3,0,&e)&&e==EINVAL);CHECK(!tl_pool_start(2,0,1,&e)&&e==ENOMEM);
    for(unsigned n=0;n<=4;n++)for(unsigned f=0;f<=n;f++){
        struct tl_pool *p=tl_pool_start(n,f,0,&e);CHECK(p&&e==0);unsigned count=f?f-1:n;
        if(count)CHECK(tl_pool_dispose(&p)==EBUSY&&p);
        struct tl_stats out={99,99,99,99,99};CHECK(tl_pool_collect(p,NULL,NULL,&out)==0);
        unsigned sum=0;for(unsigned i=1;i<=count;i++)sum+=i*i;
        CHECK(out.created==count&&out.joined==count&&out.completed==count&&out.sum==sum&&out.create_error==(f?EAGAIN:0));
        CHECK(tl_pool_dispose(&p)==0&&!p);CHECK(tl_pool_dispose(&p)==EINVAL);
    }
    for(unsigned failure=1;failure<=4;failure++){
        struct tl_pool *p=tl_pool_start(4,0,0,&e);CHECK(p&&e==0);struct join_script s={0,failure};
        struct tl_stats out={88,88,88,88,88};errno=ERANGE;
        CHECK(tl_pool_collect(p,scripted,&s,&out)==EBUSY&&errno==ERANGE);
        int preserved_join_output=out.created==88&&out.joined==88&&out.completed==88&&out.sum==88&&out.create_error==88;
        CHECK(tl_pool_dispose(&p)==EBUSY&&p);s.fail=0;
        CHECK(tl_pool_collect(p,scripted,&s,&out)==0&&out.joined==4&&out.sum==30);
        CHECK(s.calls==5);CHECK(tl_pool_collect(p,scripted,&s,&out)==0&&out.sum==30&&s.calls==5);CHECK(tl_pool_dispose(&p)==0&&!p);CHECK(preserved_join_output);
    }
    puts("OBS pool successful-prefix=15 retained-join-errors=4 sum=30");
    CHECK(pthread_setcancelstate(previous,NULL)==0);
}
static void detached(void){
    for(int cancel=0;cancel<=1;cancel++){
        struct tl_monitor m;CHECK(tl_monitor_init(&m)==0);unsigned char bytes[4]={1,2,3,4};struct tl_creator c={&m,bytes,4,cancel,99};pthread_t creator;
        CHECK(pthread_create(&creator,NULL,tl_detached_creator,&c)==0);
        CHECK(pthread_mutex_lock(&m.mutex)==0);while(!m.creator_waiting)CHECK(pthread_cond_wait(&m.changed,&m.mutex)==0);CHECK(m.created==1&&m.freed==0);CHECK(pthread_mutex_unlock(&m.mutex)==0);
        if(cancel)CHECK(pthread_cancel(creator)==0);
        void *result=(void *)&m;CHECK(pthread_join(creator,&result)==0);CHECK(result==(cancel?PTHREAD_CANCELED:NULL));CHECK(c.error==0);
        memset(bytes,99,sizeof bytes);
        CHECK(pthread_mutex_lock(&m.mutex)==0);int worker_pending_after_creator=m.live_refs==1&&m.freed==0&&m.completed==0;
        m.release_worker=1;CHECK(pthread_cond_broadcast(&m.changed)==0);
        while(!m.freed)CHECK(pthread_cond_wait(&m.changed,&m.mutex)==0);
        CHECK(m.freed==1&&m.live_refs==0&&m.completed==1&&m.sum==10);CHECK(pthread_mutex_unlock(&m.mutex)==0);
        CHECK(tl_monitor_destroy(&m)==0);CHECK(worker_pending_after_creator);
    }
    for(size_t n=0;n<=16;n+=16){
        struct tl_monitor m;CHECK(tl_monitor_init(&m)==0);unsigned char input[16];memset(input,2,sizeof input);
        struct tl_creator c={&m,n?input:NULL,n,1,99};pthread_t creator;CHECK(pthread_create(&creator,NULL,tl_detached_creator,&c)==0);
        CHECK(pthread_mutex_lock(&m.mutex)==0);while(!m.creator_waiting)CHECK(pthread_cond_wait(&m.changed,&m.mutex)==0);
        m.release_worker=1;CHECK(pthread_cond_broadcast(&m.changed)==0);CHECK(pthread_mutex_unlock(&m.mutex)==0);
        CHECK(pthread_join(creator,NULL)==0&&c.error==0);
        CHECK(pthread_mutex_lock(&m.mutex)==0);while(!m.freed)CHECK(pthread_cond_wait(&m.changed,&m.mutex)==0);
        CHECK(m.sum==2*n&&m.freed==1&&m.live_refs==0);CHECK(pthread_mutex_unlock(&m.mutex)==0);CHECK(tl_monitor_destroy(&m)==0);
    }
    struct tl_monitor invalid;CHECK(tl_monitor_init(&invalid)==0);struct tl_creator c={&invalid,NULL,1,0,99};CHECK(tl_detached_creator(&c)==NULL&&c.error==EINVAL);CHECK(tl_monitor_destroy(&invalid)==0);
    puts("OBS detached normal-and-canceled-creator worker-reference-retained copied-sum=10 last-user-free=1");
}
int main(int argc,char **argv){CHECK(argc==2);if(!strcmp(argv[1],"pool"))pool();else if(!strcmp(argv[1],"detached"))detached();else CHECK(0);return 0;}
