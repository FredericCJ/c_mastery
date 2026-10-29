#include "stream.h"
#ifndef PS_MUTANT
#define PS_MUTANT 0
#endif
void pt_session_init(struct pt_session *s){if(s){*s=(struct pt_session){0};cp_parser_init(&s->parser);cp_queue_init(&s->queue);}}
enum cp_status pt_session_open(struct pt_session *s){
    if(!s||s->connected||s->epoch==UINT64_MAX)return CP_INVALID;
    ++s->epoch;s->connected=true;s->rejects=0;
#if PS_MUTANT != 1
    cp_parser_init(&s->parser);cp_queue_init(&s->queue);
#endif
    return CP_OK;
}
void pt_session_close(struct pt_session *s){if(s){s->connected=false;
#if PS_MUTANT != 1
    cp_parser_init(&s->parser);cp_queue_init(&s->queue);
#endif
}}
enum cp_status pt_session_feed(struct pt_session *s,const unsigned char *bytes,size_t n,size_t *used){
    if(!s||!s->connected||!used||(!bytes&&n))return CP_INVALID;
    *used=0;
    for(;;){
        size_t consumed=0;
        enum cp_status result=cp_feed(&s->parser,n?bytes+*used:NULL,n-*used,&consumed);
        *used+=consumed;
        if(result==CP_FRAME){result=cp_deliver(&s->parser,&s->queue);if(result!=CP_OK)return result;}
        else if(result==CP_REJECT){
            if(s->rejects!=SIZE_MAX)++s->rejects;
#if PS_MUTANT == 2
            *used=n;return CP_NEED_MORE;
#endif
        }
        else return result;
    }
}
enum cp_status pt_session_take(struct pt_session *s,struct cp_message *m){if(!s||!s->connected||!m)return CP_INVALID;return cp_take(&s->queue,m);}
