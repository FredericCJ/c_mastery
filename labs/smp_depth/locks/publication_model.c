#include <pthread.h>
#include <assert.h>
#include <locks.c>
#ifndef SL_KIND
#define SL_KIND 0
#endif
static struct sl_lock lock;
static struct sl_node nodes[2];
static unsigned counter;
static void *worker(void *context) {
 unsigned index=(unsigned)(uintptr_t)context;struct sl_guard guard={0};
 assert(sl_enter(&lock,&nodes[index],&guard,NULL)==SL_OK);
 counter++;
 assert(sl_leave(&guard,NULL)==SL_OK);return NULL;
}
int main(void) {
 pthread_t a,b;assert(sl_init(&lock,SL_KIND,1,0)==SL_OK);
 sl_node_init(&nodes[0]);sl_node_init(&nodes[1]);
 pthread_create(&a,NULL,worker,(void *)(uintptr_t)0);pthread_create(&b,NULL,worker,(void *)(uintptr_t)1);
 pthread_join(a,NULL);pthread_join(b,NULL);assert(counter==2);return 0;
}
