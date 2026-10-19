#ifndef B06_IO_TRANSFER_H
#define B06_IO_TRANSFER_H
#include <stdint.h>
#include <stddef.h>
#include <sys/uio.h>
#include <pthread.h>
typedef ssize_t (*ITWrite)(void *,const struct iovec *,int);
/* Callback returns bytes or negative errno; no state reentry or retained vectors.
 * Source spans stay live across resumptions. cursor is accepted byte prefix. */
int it_progress(const struct iovec *,unsigned,size_t *cursor,ITWrite,void *);
typedef struct { pthread_mutex_t lock; pthread_cond_t readable,writable;
 uint64_t values[3]; unsigned head,count,closed; } ITQueue;
int it_queue_init(ITQueue *);
int it_push(ITQueue *,uint64_t);
int it_pop(ITQueue *,uint64_t *);
void it_queue_close(ITQueue *);
void it_queue_destroy(ITQueue *);
typedef struct {uint64_t start,last,now,idle,total;unsigned ready,next,closed;} ITLoop;
/* Time monotonic; positive progress only refreshes idle, never total. */
int it_time(ITLoop *,uint64_t now,int progressed);
int it_choose(ITLoop *);
void it_drained(ITLoop *,unsigned peer);
int it_direct_ok(uintptr_t address,size_t length,uint64_t offset,unsigned mem_align,unsigned off_align);
#endif
