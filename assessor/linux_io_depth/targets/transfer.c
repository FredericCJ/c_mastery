#define _POSIX_C_SOURCE 200809L
#include "transfer.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
#ifndef IT_MUTANT
#define IT_MUTANT 0
#endif
int it_progress(const struct iovec*v,unsigned n,size_t*cursor,ITWrite fn,void*ctx){size_t total=0;struct iovec remaining[8];if(n>8)return -EINVAL;for(unsigned i=0;i<n;i++){if(v[i].iov_len>(size_t)SSIZE_MAX-total)return -EINVAL;total+=v[i].iov_len;}if(*cursor>total)return -EINVAL;for(unsigned attempt=0;attempt<16&&*cursor<total;attempt++){size_t skip=IT_MUTANT==1?0:*cursor;unsigned count=0;for(unsigned i=0;i<n;i++){if(skip>=v[i].iov_len){skip-=v[i].iov_len;continue;}remaining[count].iov_base=(unsigned char*)v[i].iov_base+skip;remaining[count++].iov_len=v[i].iov_len-skip;skip=0;}ssize_t got=fn(ctx,remaining,(int)count);if(got==-EINTR)continue;if(got==-EAGAIN||got==-EWOULDBLOCK)return 1;if(got<0)return (int)got;if(!got||(size_t)got>total-*cursor)return -EIO;*cursor+=(size_t)got;}return *cursor==total?0:1;}
int it_queue_init(ITQueue*q){memset(q,0,sizeof *q);int e=pthread_mutex_init(&q->lock,NULL);if(e)return e;e=pthread_cond_init(&q->readable,NULL);if(e){pthread_mutex_destroy(&q->lock);return e;}e=pthread_cond_init(&q->writable,NULL);if(e){pthread_cond_destroy(&q->readable);pthread_mutex_destroy(&q->lock);}return e;}
int it_push(ITQueue*q,uint64_t value){pthread_mutex_lock(&q->lock);while(q->count==3&&!q->closed)pthread_cond_wait(&q->writable,&q->lock);if(q->closed){pthread_mutex_unlock(&q->lock);return 0;}unsigned i=IT_MUTANT==2?(q->head+q->count)&2:(q->head+q->count)%3;q->values[i]=value;++q->count;pthread_cond_signal(&q->readable);pthread_mutex_unlock(&q->lock);return 1;}
int it_pop(ITQueue*q,uint64_t*v){pthread_mutex_lock(&q->lock);while(!q->count&&!q->closed)pthread_cond_wait(&q->readable,&q->lock);if(!q->count){pthread_mutex_unlock(&q->lock);return 0;}*v=q->values[q->head];q->head=(q->head+1)%3;--q->count;pthread_cond_signal(&q->writable);pthread_mutex_unlock(&q->lock);return 1;}
void it_queue_close(ITQueue*q){pthread_mutex_lock(&q->lock);q->closed=1;pthread_cond_broadcast(&q->readable);pthread_cond_broadcast(&q->writable);pthread_mutex_unlock(&q->lock);}
void it_queue_destroy(ITQueue*q){pthread_cond_destroy(&q->readable);pthread_cond_destroy(&q->writable);pthread_mutex_destroy(&q->lock);}
int it_time(ITLoop*s,uint64_t now,int progress){if(now<s->now)return -EINVAL;if(s->closed)return 0;s->now=now;if(now-s->start>=s->total||now-s->last>=s->idle){s->closed=1;return 0;}if(progress){s->last=now;if(IT_MUTANT==3)s->start=now;}return 1;}
int it_choose(ITLoop*s){if(s->closed)return -1;for(unsigned i=0;i<2;i++){unsigned p=(s->next+i)%2;if(s->ready&(1u<<p)){s->next=IT_MUTANT==4?0:1-p;return (int)p;}}return -1;}
void it_drained(ITLoop*s,unsigned p){if(p<2)s->ready&=~(1u<<p);}
int it_direct_ok(uintptr_t p,size_t n,uint64_t off,unsigned ma,unsigned oa){return ma&&oa&&p%ma==0&&n%oa==0&&off%oa==0;}
