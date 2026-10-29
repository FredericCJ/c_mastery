#include "active_import.h"
#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <stdatomic.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PI_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static void enter(void *v)
{
    pthread_mutex_lock(v);
}

static void leave(void *v)
{
    pthread_mutex_unlock(v);
}

struct pf_work
{
    struct pa_task_active *active;
    unsigned id;
    atomic_uint *done, *bad;
};

static void *produce(void *v)
{
    struct pf_work *w = v;
    for (unsigned i = 0; i < 1000; i++)
    {
        int r;
        do
        {
            r = pa_active_post(w->active, w->id, PT_PING);
            if (r == PT_RANGE)
            {
                sched_yield();
            }
        } while (r == PT_RANGE);
        if (r)
        {
            atomic_store(w->bad, 1);
        }
    }
    atomic_fetch_add(w->done, 1);
    return NULL;
}

static void *consume(void *v)
{
    struct pf_work *w = v;
    for (;;)
    {
        size_t used = 91;
        int r = pa_active_step(w->active, w->id, NULL, 0, &used);
        if (r == PT_PENDING)
        {
            if (atomic_load(w->done) == 2)
            {
                break;
            }
            sched_yield();
        }
        else if (r || used)
        {
            atomic_store(w->bad, 1);
        }
    }
    return NULL;
}

int main(void)
{
    pthread_mutex_t mutex;
    C(pthread_mutex_init(&mutex, NULL) == 0);
    struct pa_task_active a;
    C(pa_active_init(&a, (struct pf_lock){&mutex, enter, leave}) == 0);
    C(pa_active_post(&a, 0, PT_START) == 0);
    unsigned char trace[16];
    size_t used = 91;
    C(pa_active_step(&a, 2, trace, 16, &used) == PT_STATE && a.held &&
      used == 91);
    C(pa_active_post(&a, 0, PT_CONNECT) == 0 &&
      pa_active_post(&a, 1, PT_START) == 0 &&
      pa_active_post(&a, 0, PT_FINISH) == 0);
    C(pa_active_post(&a, 1, PT_PING) == PT_RANGE && a.queue.count == 3 &&
      a.held);
    unsigned event = 91;
    C(pa_active_discard(&a, 3, &event) == 0 && event == PT_START &&
      a.discarded == 1);
    C(pa_active_step(&a, 2, NULL, 0, &used) == PT_RANGE && a.held &&
      a.machine.active == PT_OFFLINE);
    C(pa_active_step(&a, 3, trace, 16, &used) == 0 &&
      a.machine.active == PT_IDLE);
    C(pa_active_step(&a, 2, trace, 16, &used) == 0 &&
      a.machine.active == PT_BUSY);
    C(pa_active_step(&a, 3, trace, 16, &used) == 0 &&
      a.machine.active == PT_IDLE);
    C(pa_active_step(&a, 2, trace, 16, &used) == PT_PENDING);
    atomic_uint done = 0, bad = 0;
    struct pf_work w[4];
    pthread_t t[4];
    for (unsigned i = 0; i < 4; i++)
    {
        w[i] = (struct pf_work){&a, i, &done, &bad};
        C(pthread_create(t + i, NULL, i < 2 ? produce : consume, w + i) == 0);
    }
    for (unsigned i = 0; i < 4; i++)
    {
        C(pthread_join(t[i], NULL) == 0);
    }
    C(!atomic_load(&bad) && a.machine.pings == 2000 && !a.held &&
      !a.queue.count && cmx_invariant(&a.kernel));
    C(pthread_mutex_destroy(&mutex) == 0);
    puts(
        "PI_ACTIVE:actual execution.tasks/copied_queue/HSM;retained blocked head;2000events/2producers/2consumers");
    return 0;
}
