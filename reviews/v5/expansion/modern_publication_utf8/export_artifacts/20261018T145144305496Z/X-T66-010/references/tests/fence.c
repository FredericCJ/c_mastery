#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#ifndef ORC_ACQ_REL
#define ORC_ACQ_REL 0
#endif
static atomic_uint count;
static int values[2];
static void drop(void){if(atomic_fetch_sub_explicit(&count,1,ORC_ACQ_REL?memory_order_acq_rel:memory_order_release)==1){atomic_thread_fence(memory_order_acquire);assert(values[0]==11&&values[1]==12);}}
static void *first(void *unused){(void)unused;values[0]=11;drop();return NULL;}
static void *second(void *unused){(void)unused;values[1]=12;drop();return NULL;}
int main(void){atomic_init(&count,2);pthread_t a,b;assert(pthread_create(&a,NULL,first,NULL)==0);assert(pthread_create(&b,NULL,second,NULL)==0);assert(pthread_join(a,NULL)==0&&pthread_join(b,NULL)==0);puts(ORC_ACQ_REL?"ORC_FENCE stronger-acq-rel-rmw":"ORC_FENCE release-rmw-acquire-fence");return 0;}
