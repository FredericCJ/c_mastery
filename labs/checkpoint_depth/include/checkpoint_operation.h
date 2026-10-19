#ifndef CHECKPOINT_OPERATION_H
#define CHECKPOINT_OPERATION_H
#include <stddef.h>
#include <pthread.h>
typedef enum { CPO_OK,CPO_PENDING,CPO_BUSY,CPO_STALE,CPO_NOMEM,CPO_RETIRED,CPO_ERROR } CPOStatus;
typedef enum { CPO_IDLE,CPO_ACTIVE,CPO_CANCELLING,CPO_REPORTING,CPO_DEAD } CPOState;
typedef void *(*CPOAllocate)(void *,size_t);
typedef void (*CPORelease)(void *,void *);
typedef int (*CPOReport)(void *,unsigned,const unsigned char *,size_t,int);
typedef struct {
    pthread_mutex_t mutex;unsigned char *bytes;size_t length;
    unsigned generation,limit;CPOState state;int report_failed;
    void *context;CPOAllocate allocate;CPORelease release;CPOReport report;
} CPOperation;
CPOStatus cpo_init(CPOperation *,unsigned,void *,CPOAllocate,CPORelease,CPOReport);
CPOStatus cpo_begin(CPOperation *,const unsigned char *,size_t,unsigned *);
CPOStatus cpo_cancel(CPOperation *,unsigned);
CPOStatus cpo_complete(CPOperation *,unsigned,int);
CPOStatus cpo_destroy(CPOperation *);
#endif
