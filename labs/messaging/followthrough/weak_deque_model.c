#include "weak.h"
#include <pthread.h>
#include <assert.h>
#ifndef MFW_CASE
#define MFW_CASE 1
#endif
static struct mfw_deque queue;
static unsigned outputs[6];
static bool obtained[6];
static void *owner(void *unused) {
    (void)unused;
#if MFW_CASE==1
    assert(mfw_push(&queue,41)==MFW_OK);
#elif MFW_CASE==2
    obtained[0]=mfw_pop(&queue,&outputs[0])==MFW_OK;
#elif MFW_CASE==3
    assert(mfw_push(&queue,41)==MFW_OK);
    assert(mfw_push(&queue,42)==MFW_OK);
    obtained[0]=mfw_pop(&queue,&outputs[0])==MFW_OK;
#elif MFW_CASE==4
    obtained[0]=mfw_pop(&queue,&outputs[0])==MFW_OK;
    assert(mfw_push(&queue,43)==MFW_OK);
    obtained[1]=mfw_pop(&queue,&outputs[1])==MFW_OK;
#endif
    return NULL;
}
static void *thief1(void *unused) {
    (void)unused;obtained[2]=mfw_steal(&queue,&outputs[2])==MFW_OK;
#if MFW_CASE==4
    obtained[3]=mfw_steal(&queue,&outputs[3])==MFW_OK;
#endif
    return NULL;
}
static void *thief2(void *unused) {
    (void)unused;obtained[4]=mfw_steal(&queue,&outputs[4])==MFW_OK;return NULL;
}
int main(void) {
    pthread_t a,b,c;mfw_deque_init(&queue);
#if MFW_CASE==2 || MFW_CASE==4
    assert(mfw_push(&queue,41)==MFW_OK);assert(mfw_push(&queue,42)==MFW_OK);
#endif
    assert(!pthread_create(&a,NULL,owner,NULL));assert(!pthread_create(&b,NULL,thief1,NULL));
    assert(!pthread_create(&c,NULL,thief2,NULL));
    assert(!pthread_join(a,NULL));assert(!pthread_join(b,NULL));assert(!pthread_join(c,NULL));
    unsigned seen=0;
    for(unsigned i=0;i<6;i++)if(obtained[i]) {
        assert(outputs[i]>=41 && outputs[i]<=43);
        unsigned bit=1u<<(outputs[i]-41);assert(!(seen&bit));seen|=bit;
    }
    unsigned value;
    while(mfw_pop(&queue,&value)==MFW_OK) {
        assert(value>=41 && value<=43);unsigned bit=1u<<(value-41);
        assert(!(seen&bit));seen|=bit;
    }
#if MFW_CASE==1
    assert(seen==1);
#elif MFW_CASE==4
    assert(seen==7);
#else
    assert(seen==3);
#endif
    return 0;
}
