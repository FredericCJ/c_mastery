#define _POSIX_C_SOURCE 200809L
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <assert.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>
static StaticQueue_t control; static unsigned char storage[8*sizeof(unsigned)]; static QueueHandle_t q;
static StaticTask_t task_controls[2]; static StackType_t stacks[2][4096];
static atomic_uint received=0,sum=0;
static void producer(void *unused) {
    (void)unused;
    for(unsigned i=0;i<256;++i) configASSERT(xQueueSend(q,&i,portMAX_DELAY)==pdPASS);
    unsigned end=UINT_MAX; configASSERT(xQueueSend(q,&end,portMAX_DELAY)==pdPASS);
    vTaskSuspend(NULL); for(;;) {}
}
static void consumer(void *unused) {
    (void)unused; unsigned n=0,total=0;
    for(;;) {
        unsigned item; configASSERT(xQueueReceive(q,&item,portMAX_DELAY)==pdPASS);
        if(item==UINT_MAX) break;
        configASSERT(item==n); ++n; total+=item;
    }
    atomic_store(&received,n); atomic_store(&sum,total);
    configASSERT(n==256 && total==32640 && uxQueueMessagesWaiting(q)==0);
    vTaskEndScheduler(); for(;;) {}
}
static double seconds(struct timespec a,struct timespec b) { return (double)(b.tv_sec-a.tv_sec)+(double)(b.tv_nsec-a.tv_nsec)/1e9; }
int main(void) {
    struct timespec w0,w1,c0,c1;
    q=xQueueCreateStatic(8,sizeof(unsigned),storage,&control); configASSERT(q);
    configASSERT(xTaskCreateStatic(producer,"producer",4096,NULL,2,stacks[0],&task_controls[0]));
    configASSERT(xTaskCreateStatic(consumer,"consumer",4096,NULL,2,stacks[1],&task_controls[1]));
    configASSERT(!clock_gettime(CLOCK_MONOTONIC,&w0)); configASSERT(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&c0));
    vTaskStartScheduler();
    configASSERT(!clock_gettime(CLOCK_MONOTONIC,&w1)); configASSERT(!clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&c1));
    unsigned n=atomic_load(&received),s=atomic_load(&sum);
    printf("platform=FreeRTOS-11.2.0-POSIX count=%u checksum=%u capacity=8 wall_s=%.9f cpu_s=%.9f physical_timing=unmeasured\n",n,s,seconds(w0,w1),seconds(c0,c1));
    return n==256 && s==32640 ? 0 : 1;
}
