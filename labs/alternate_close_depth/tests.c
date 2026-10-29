#define _POSIX_C_SOURCE 200809L
#include "close_owner.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"QC_CHECK:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
struct qc_script { struct qc_raw raw; unsigned calls; int fd; };
static struct qc_raw scripted(void *p,int fd){struct qc_script *s=p;s->calls++;s->fd=fd;return s->raw;}
static void core(void)
{
    struct qc_owner o={17,QC_CLOSED};struct qc_observation v={QC_MOVED,99};struct qc_script s={{-1,EINTR},0,0};int fd=99;
    CHECK(qc_init(NULL,3)==QC_INVALID);CHECK(qc_init(&o,-1)==QC_INVALID);CHECK(o.fd==17&&o.state==QC_CLOSED);
    CHECK(qc_init(&o,7)==QC_OK);
    CHECK(qc_once(&o,NULL,&s,&v)==QC_INVALID&&v.error==99);
    CHECK(qc_once(&o,scripted,&s,NULL)==QC_INVALID&&s.calls==0);
    CHECK(qc_once(&o,scripted,&s,&v)==QC_AGAIN&&s.calls==1&&s.fd==7&&o.fd==7&&o.state==QC_RETRY&&v.error==EINTR);
    CHECK(qc_once(&o,scripted,&s,&v)==QC_AGAIN&&s.calls==2);
    s.raw=(struct qc_raw){0,123};CHECK(qc_once(&o,scripted,&s,&v)==QC_OK&&s.calls==3&&o.fd==-1&&v.error==0);
    v=(struct qc_observation){QC_MOVED,99};CHECK(qc_once(&o,scripted,&s,&v)==QC_TERMINAL&&s.calls==3&&v.error==99);
    CHECK(qc_handoff(&o,&fd)==QC_TERMINAL&&fd==99);
    { struct qc_owner q;CHECK(qc_init(&q,8)==QC_OK);s.raw=(struct qc_raw){-1,EINTR};CHECK(qc_once(&q,scripted,&s,&v)==QC_AGAIN);
      CHECK(qc_handoff(&q,&fd)==QC_OK&&fd==8&&q.state==QC_MOVED&&q.fd==-1);
      CHECK(qc_handoff(&q,&fd)==QC_TERMINAL); }
    { const struct qc_raw faults[]={{-1,EIO},{-1,ENOSPC},{-1,ENOSYS},{-1,EBADF},{-1,0},{1,0},{-2,EINTR}};
      for(size_t i=0;i<sizeof faults/sizeof faults[0];i++){
        struct qc_owner q;CHECK(qc_init(&q,9)==QC_OK);s.raw=faults[i];unsigned n=s.calls;fd=99;
        CHECK(qc_once(&q,scripted,&s,&v)==QC_UNKNOWN&&q.state==QC_QUARANTINED&&q.fd==9&&s.calls==n+1);
        CHECK(qc_handoff(&q,&fd)==QC_TERMINAL&&fd==99);
        CHECK(qc_once(&q,scripted,&s,&v)==QC_TERMINAL&&s.calls==n+1);
      }
    }
    puts("QC_OK:core");
}
struct qc_real { unsigned calls; int inject; };
static struct qc_raw real_close(void *p,int fd)
{
    struct qc_real *r=p;r->calls++;
    if(r->inject && r->calls==1)return (struct qc_raw){-1,EINTR};
    int rc=close(fd);return (struct qc_raw){rc,rc<0?errno:0};
}
static struct qc_raw uncertain_close(void *p,int fd)
{
    struct qc_real *r=p;r->calls++;CHECK(close(fd)==0);return (struct qc_raw){-1,EIO};
}
static void native(void)
{
    int f[2],g[2],saved,old;struct qc_owner o,q;struct qc_observation v;struct qc_real r={0,1};
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&old)==0);CHECK(pipe(f)==0);saved=f[0];
    CHECK(qc_init(&o,saved)==QC_OK);CHECK(qc_once(&o,real_close,&r,&v)==QC_AGAIN&&r.calls==1);
    CHECK(fcntl(saved,F_GETFD)>=0);CHECK(qc_once(&o,real_close,&r,&v)==QC_OK&&r.calls==2);
    errno=0;CHECK(fcntl(saved,F_GETFD)==-1&&errno==EBADF);CHECK(close(f[1])==0);
    CHECK(pipe(f)==0);saved=f[0];r=(struct qc_real){0,0};CHECK(qc_init(&q,saved)==QC_OK);
    CHECK(qc_once(&q,uncertain_close,&r,&v)==QC_UNKNOWN&&r.calls==1);
    CHECK(pipe(g)==0);if(g[0]!=saved){CHECK(dup2(g[0],saved)==saved);CHECK(close(g[0])==0);}
    CHECK(qc_once(&q,uncertain_close,&r,&v)==QC_TERMINAL&&r.calls==1);
    CHECK(fcntl(saved,F_GETFD)>=0);CHECK(close(saved)==0);CHECK(close(g[1])==0);CHECK(close(f[1])==0);
    CHECK(pthread_setcancelstate(old,NULL)==0);puts("QC_OK:native-seam-linux");
}
struct qc_shared { pthread_mutex_t m;pthread_cond_t cv;int ready,go,fd; };
static void *worker(void *p)
{
    struct qc_shared *s=p;struct qc_owner o;struct qc_observation v;struct qc_script fault={{-1,EINTR},0,0};int old;
    CHECK(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&old)==0);CHECK(qc_init(&o,s->fd)==QC_OK);
    CHECK(qc_once(&o,scripted,&fault,&v)==QC_AGAIN);CHECK(pthread_mutex_lock(&s->m)==0);
    s->ready=1;CHECK(pthread_cond_broadcast(&s->cv)==0);while(!s->go)CHECK(pthread_cond_wait(&s->cv,&s->m)==0);
    CHECK(qc_handoff(&o,&s->fd)==QC_OK);CHECK(pthread_mutex_unlock(&s->m)==0);
    CHECK(pthread_setcancelstate(old,NULL)==0);pthread_testcancel();return NULL;
}
static void cancellation(void)
{
    int f[2];pthread_t t;void *result=NULL;struct qc_shared s={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,0,0};
    CHECK(pipe(f)==0);s.fd=f[0];CHECK(pthread_create(&t,NULL,worker,&s)==0);CHECK(pthread_mutex_lock(&s.m)==0);
    while(!s.ready){CHECK(pthread_cond_wait(&s.cv,&s.m)==0);}
    CHECK(pthread_cancel(t)==0);s.go=1;
    CHECK(pthread_cond_broadcast(&s.cv)==0);CHECK(pthread_mutex_unlock(&s.m)==0);CHECK(pthread_join(t,&result)==0);
    CHECK(result==PTHREAD_CANCELED&&fcntl(s.fd,F_GETFD)>=0);CHECK(close(s.fd)==0);CHECK(close(f[1])==0);
    CHECK(pthread_cond_destroy(&s.cv)==0);CHECK(pthread_mutex_destroy(&s.m)==0);puts("QC_OK:cancellation-handoff");
}
int main(int argc,char **argv)
{
    CHECK(argc==2);if(!strcmp(argv[1],"core"))core();else if(!strcmp(argv[1],"native"))native();else if(!strcmp(argv[1],"cancel"))cancellation();else return 2;return 0;
}
