#ifndef CMASTERY_CONTRACT_MODES_H
#define CMASTERY_CONTRACT_MODES_H
#include "errors.h"
#include <threads.h>
#ifdef ECM_POSIX_THREADS
#include <pthread.h>
typedef pthread_mutex_t EcmMutex;
#else
typedef mtx_t EcmMutex;
#endif
typedef struct {unsigned char *data;size_t length,reports;} EcmBuffer;
typedef struct {EcmMutex mutex;EcmBuffer buffer;int counter;} EcmShared;
typedef struct {int counter;size_t reports,length;unsigned char bytes[8];} EcmSnapshot;
er_code ecm_init(EcmBuffer *,unsigned char *,size_t length,size_t reports);
er_code ecm_write(EcmBuffer *,er_mode,size_t index,unsigned char value);
/* Fresh noncopied control; borrowed data lives until quiescent joined destroy.
   Lock acquisition failure returns IO without effects. Unexpected unlock failure
   terminates the original local process: an already committed effect is not
   reported as an unchanged-output operational failure. No reentry/cancellation. */
er_code ecm_shared_init(EcmShared *,unsigned char *,size_t,int,size_t);
er_code ecm_shared_write(EcmShared *,er_mode,size_t,unsigned char);
er_code ecm_shared_counter(EcmShared *,er_mode,int);
er_code ecm_shared_snapshot(EcmShared *,EcmSnapshot *);
void ecm_shared_destroy(EcmShared *);
#endif
