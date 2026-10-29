#include "control.h"
#include <stdio.h>
#ifdef LG_USE_C_THREADS
#include <threads.h>

static int consumer(void *ctx)
{
    struct lg_publication *p = ctx;
    int value = 0;
    for (unsigned attempts = 0; attempts < 10000000; ++attempts)
    {
        enum lg_status r = lg_receive(p, &value);
        if (r == LG_OK)
            return value == 37 ? 0 : 1;
        if (r != LG_PENDING)
            return 2;
        thrd_yield();
    }
    return 3;
}
#else
#include <pthread.h>
#include <sched.h>

struct context
{
    struct lg_publication *p;
    int result;
};

static void *consumer(void *ptr)
{
    struct context *c = ptr;
    int value = 0;
    c->result = 3;
    for (unsigned attempts = 0; attempts < 10000000; ++attempts)
    {
        enum lg_status r = lg_receive(c->p, &value);
        if (r == LG_OK)
        {
            c->result = value == 37 ? 0 : 1;
            break;
        }
        if (r != LG_PENDING)
        {
            c->result = 2;
            break;
        }
        sched_yield();
    }
    return NULL;
}
#endif
int main(void)
{
    for (unsigned iteration = 0; iteration < 32; ++iteration)
    {
        struct lg_publication p;
        lg_init(&p);
#ifdef LG_USE_C_THREADS
        thrd_t t;
        if (thrd_create(&t, consumer, &p) != thrd_success)
            return 1;
        lg_publish(&p, 37);
        int result;
        if (thrd_join(t, &result) != thrd_success || result)
            return 1;
#else
        struct context c = {&p, 0};
        pthread_t t;
        if (pthread_create(&t, NULL, consumer, &c))
            return 1;
        lg_publish(&p, 37);
        if (pthread_join(t, NULL) || c.result)
            return 1;
#endif
    }
    puts("CONTROL-PASS:32 actual thread publications before join;finite retry "
         "does not prove scheduler fairness");
    return 0;
}
