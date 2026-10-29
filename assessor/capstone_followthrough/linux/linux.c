#define _POSIX_C_SOURCE 200809L
#include "linux.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#ifndef CS_LINUX_MUTANT
#define CS_LINUX_MUTANT 0
#endif
void csl_init(struct cs_linux *a,struct cs_service *s,int epoll){*a=(struct cs_linux){.core=s,.epoll=epoll};for(unsigned i=0;i<2;++i)a->session[i]=(struct cs_session){.fd=-1,.generation=7};}
uint64_t csl_token(const struct cs_linux *a,unsigned slot){return ((uint64_t)a->session[slot].generation<<32)|slot;}
int csl_open(struct cs_linux *a,unsigned slot,int fd,int edge){
    if(!a || slot>=2 || a->session[slot].live || a->session[slot].generation==UINT_MAX)return -1;
    int flags=fcntl(fd,F_GETFL);if(flags<0 || !(flags&O_NONBLOCK))return -1;
    struct epoll_event ev={.events=EPOLLIN|EPOLLRDHUP|(edge?EPOLLET:0),.data.u64=csl_token(a,slot)};
    if(epoll_ctl(a->epoll,EPOLL_CTL_ADD,fd,&ev))return -1;
    a->session[slot].fd=fd;a->session[slot].live=1;a->session[slot].ready=a->session[slot].eof=0;a->core->input[slot].used=0;return 0;
}
int csl_close(struct cs_linux *a,unsigned slot){
    if(!a || slot>=2 || !a->session[slot].live)return -1;
    struct cs_session *s=&a->session[slot];s->live=s->ready=0;++s->generation;
    int failed=epoll_ctl(a->epoll,EPOLL_CTL_DEL,s->fd,NULL)!=0;
    if(close(s->fd))failed=1;
    s->fd=-1;a->core->input[slot].used=0;return failed?-1:0;
}
enum cs_status csl_event(struct cs_linux *a,uint64_t token){
    unsigned slot=(unsigned)(token&UINT32_MAX);if(!a || slot>=2)return CS_INVALID;
    struct cs_session *s=&a->session[slot];if(!s->live || (CS_LINUX_MUTANT!=1 && token!=csl_token(a,slot)))return CS_STALE;
    s->ready=1;return CS_OK;
}
struct cs_turn csl_turn(struct cs_linux *a,unsigned slot,size_t budget,int authorized){
    struct cs_turn out={0,CS_INVALID,0};if(!a || slot>=2 || !budget || budget>42)return out;
    struct cs_session *s=&a->session[slot];if(!s->live || !s->ready){out.status=CS_MORE;return out;}
    for(size_t actions=0;actions<2*budget+42;++actions){
        /* Flush parser-owned nested candidates before accepting a new byte. */
        size_t used=0;enum cs_status flushed=cs_feed(a->core,slot,NULL,0,&used,authorized,0);
        if(flushed!=CS_MORE){if(flushed==CS_INVALID || flushed==CS_CLOSED){out.status=flushed;return out;}continue;}
        if(out.bytes==budget){out.status=CS_PENDING;if(CS_LINUX_MUTANT==2)s->ready=0;return out;}
        unsigned char byte;ssize_t n=read(s->fd,&byte,1);
        if(n==1){++out.bytes;enum cs_status status=cs_feed(a->core,slot,&byte,1,&used,authorized,0);if(used!=1 || status==CS_INVALID){out.status=CS_INVALID;return out;}continue;}
        if(n==0){s->ready=0;s->eof=1;out.status=a->core->input[slot].used?CS_REJECT:CS_CLOSED;return out;}
        if(errno==EINTR){out.status=CS_PENDING;return out;}
        if(errno==EAGAIN || errno==EWOULDBLOCK){s->ready=0;out.status=CS_MORE;return out;}
        out.error=errno;out.status=CS_FAILED;return out;
    }
    out.status=CS_PENDING;return out;
}
struct cs_io csl_write(void *context,const unsigned char *bytes,size_t n){
    int fd=*(int *)context;ssize_t accepted=write(fd,bytes,n);
    if(accepted>=0)return(struct cs_io){(size_t)accepted,CS_IO_PROGRESS,0};
    int cause=errno;return(struct cs_io){0,cause==EINTR?CS_IO_INTERRUPTED:cause==EAGAIN||cause==EWOULDBLOCK?CS_IO_AGAIN:CS_IO_ERROR,(unsigned)cause};
}
