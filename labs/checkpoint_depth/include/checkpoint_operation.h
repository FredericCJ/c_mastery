#ifndef CHECKPOINT_OPERATION_H
#define CHECKPOINT_OPERATION_H
#include <stddef.h>
#include <pthread.h>
enum cpo_status { CPO_OK,CPO_PENDING,CPO_BUSY,CPO_STALE,CPO_NOMEM,CPO_RETIRED,CPO_ERROR };
enum cpo_state { CPO_IDLE,CPO_ACTIVE,CPO_CANCELLING,CPO_REPORTING,CPO_DEAD };
typedef void *(*cpo_allocate_fn)(void *,size_t);
typedef void (*cpo_release_fn)(void *,void *);
typedef int (*cpo_report_fn)(void *,unsigned,const unsigned char *,size_t,int);
struct cp_operation {
    pthread_mutex_t mutex;unsigned char *bytes;size_t length;
    unsigned generation,limit;enum cpo_state state;int report_failed;
    void *context;cpo_allocate_fn allocate;cpo_release_fn release;cpo_report_fn report;
};
enum cpo_status cpo_init(struct cp_operation *,unsigned,void *,cpo_allocate_fn,cpo_release_fn,cpo_report_fn);
enum cpo_status cpo_begin(struct cp_operation *,const unsigned char *,size_t,unsigned *);
enum cpo_status cpo_cancel(struct cp_operation *,unsigned);
enum cpo_status cpo_complete(struct cp_operation *,unsigned,int);
enum cpo_status cpo_destroy(struct cp_operation *);
#endif
