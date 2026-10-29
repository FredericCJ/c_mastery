#define _POSIX_C_SOURCE 200809L
#include "checkpoint_linux.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
void cpl_init(struct cpl_reader *r,unsigned slot,unsigned limit){
    *r=(struct cpl_reader){.slot=slot,.generation=1,.generation_limit=limit,.fd=-1,.epfd=-1,.retired=!limit};
    cp_parser_init(&r->parser);
}
uint64_t cpl_token(const struct cpl_reader *r){return ((uint64_t)r->generation<<32)|r->slot;}
int cpl_open(struct cpl_reader *r,int epfd,int fd,int edge){
    if(!r || r->live || r->retired)return -1;
    int flags=fcntl(fd,F_GETFL);
    if(flags<0 || !(flags&O_NONBLOCK)){r->last_errno=flags<0?errno:EINVAL;return -1;}
    struct epoll_event event={.events=EPOLLIN|EPOLLRDHUP|(edge?EPOLLET:0),.data.u64=cpl_token(r)};
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event)){r->last_errno=errno;return -1;}
    r->epfd=epfd;r->fd=fd;r->live=1;r->cursor=r->end=0;r->eof=0;r->last_errno=r->cleanup_errno=0;
    cp_parser_init(&r->parser);return 0;
}
enum cpl_status cpl_pump(struct cpl_reader *r,uint64_t token,struct cpa_queue *q){
    if(!r || !q)return CPL_ERROR;
    /* A saved fd integer never substitutes for this lifetime check. */
    if(!r->live || (CP_MUTANT!=29 && token!=cpl_token(r)))return CPL_STALE;
    for(unsigned work=0;work<32;++work){
        struct cp_message message;
        if(cp_peek(&r->parser,&message)==CP_FRAME){
            enum cpa_status offered=cpa_offer(q,&message,0);
            if(offered==CPA_FULL){
                if(CP_MUTANT==30){struct cp_queue discard;cp_queue_init(&discard);(void)cp_deliver(&r->parser,&discard);}
                return CPL_BLOCKED;
            }
            if(offered!=CPA_OK)return CPL_ERROR;
            struct cp_queue committed;cp_queue_init(&committed);
            if(cp_deliver(&r->parser,&committed)!=CP_OK)return CPL_ERROR;
            continue;
        }
        /* An empty feed also processes nested candidates already buffered by the parser. */
        size_t used=0;
        enum cp_status status=cp_feed(&r->parser,r->bytes+r->cursor,r->end-r->cursor,&used);
        r->cursor+=used;
        if(status==CP_FRAME || status==CP_REJECT)continue;
        if(status!=CP_NEED_MORE)return CPL_ERROR;
        if(r->cursor!=r->end)return CPL_ERROR;
        if(r->eof)return r->parser.used?CPL_TRUNCATED:CPL_EOF;
        ssize_t count=read(r->fd,r->bytes,sizeof r->bytes);
        if(count>0){r->cursor=0;r->end=(size_t)count;continue;}
        if(!count){r->eof=1;continue;}
        if(errno==EINTR)continue;
        if(errno==EAGAIN || errno==EWOULDBLOCK)return CPL_DRAINED;
        r->last_errno=errno;return CPL_ERROR;
    }
    return CPL_PROGRESS;
}
int cpl_close(struct cpl_reader *r){
    if(!r || !r->live)return -1;
    /* Invalidate before disposal; external owner serializes reader operations. */
    r->live=0;
    if(r->generation==r->generation_limit)r->retired=1;else ++r->generation;
    int result=0;
    if(epoll_ctl(r->epfd,EPOLL_CTL_DEL,r->fd,0)){r->last_errno=errno;result=-1;}
    if(close(r->fd)){r->cleanup_errno=errno;result=-1;}
    r->fd=r->epfd=-1;r->cursor=r->end=0;cp_parser_init(&r->parser);return result;
}
