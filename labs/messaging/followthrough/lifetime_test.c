#include "lifetime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "SC_CHECK: lifetime:%d:%s\n", __LINE__, #x); \
            exit(1);                                                     \
        }                                                                \
    } while (0)

struct msg_memory
{
    atomic_uint calls, allocations, frees, drops;
    unsigned fail;
};

struct msg_payload
{
    struct msg_memory *memory;
    unsigned number;
};

static void *allocate(void *ctx, size_t n)
{
    struct msg_memory *m = ctx;
    unsigned call = atomic_fetch_add(&m->calls, 1) + 1;
    if (call == m->fail)
        return NULL;
    void *p = malloc(n);
    if (p)
        atomic_fetch_add(&m->allocations, 1);
    return p;
}

static void deallocate(void *ctx, void *p)
{
    struct msg_memory *m = ctx;
    atomic_fetch_add(&m->frees, 1);
    free(p);
}

static void drop(void *p)
{
    struct msg_payload *v = p;
    atomic_fetch_add(&v->memory->drops, 1);
    free(v);
}

static void memory_init(struct msg_memory *m)
{
    atomic_init(&m->calls, 0);
    atomic_init(&m->allocations, 0);
    atomic_init(&m->frees, 0);
    atomic_init(&m->drops, 0);
    m->fail = 0;
}

static struct mfl_allocator allocator(struct msg_memory *m)
{
    return (struct mfl_allocator){m, allocate, deallocate};
}

static struct mfo_value value(struct msg_memory *m, unsigned number)
{
    struct msg_payload *p = malloc(sizeof(*p));
    CHECK(p);
    *p = (struct msg_payload){m, number};
    return (struct mfo_value){p, drop};
}

static unsigned number(struct mfl_shared *p)
{
    return ((struct msg_payload *)mfl_borrow(p)->data)->number;
}

struct msg_pause
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    bool entered, resume;
    struct mfg_deque *queue;
    struct mfp_pointer *pointer;
    enum msg_status status;
    unsigned observed;
};

static void pause_init(struct msg_pause *p)
{
    *p = (struct msg_pause){0};
    CHECK(!pthread_mutex_init(&p->mutex, NULL));
    CHECK(!pthread_cond_init(&p->condition, NULL));
}

static void wait_entered(struct msg_pause *p)
{
    pthread_mutex_lock(&p->mutex);
    while (!p->entered)
        pthread_cond_wait(&p->condition, &p->mutex);
    pthread_mutex_unlock(&p->mutex);
}

static void resume(struct msg_pause *p)
{
    pthread_mutex_lock(&p->mutex);
    p->resume = true;
    pthread_cond_broadcast(&p->condition);
    pthread_mutex_unlock(&p->mutex);
}

static void pause_hook(void *ctx)
{
    struct msg_pause *p = ctx;
    pthread_mutex_lock(&p->mutex);
    p->entered = true;
    pthread_cond_broadcast(&p->condition);
    while (!p->resume)
        pthread_cond_wait(&p->condition, &p->mutex);
    pthread_mutex_unlock(&p->mutex);
}

static void pause_destroy(struct msg_pause *p)
{
    CHECK(!pthread_cond_destroy(&p->condition));
    CHECK(!pthread_mutex_destroy(&p->mutex));
}

static void *slow_thief(void *ctx)
{
    struct msg_pause *p = ctx;
    struct mfl_shared *lease = NULL;
    p->status = mfg_steal(p->queue, &lease, pause_hook, p);
    if (p->status == MSG_OK)
    {
        p->observed = number(lease);
        mfl_drop(&lease);
    }
    else
        CHECK(!lease);
    return NULL;
}

static void grow_pause(void)
{
    struct msg_memory m;
    memory_init(&m);
    struct mfg_deque q;
    CHECK(mfg_init(&q, allocator(&m), 8) == MSG_OK);
    struct mfo_value a = value(&m, 1), b = value(&m, 2), c = value(&m, 3);
    CHECK(mfg_push(&q, &a) == MSG_OK && !a.data);
    CHECK(mfg_push(&q, &b) == MSG_OK && !b.data);
    struct msg_pause pause;
    pause_init(&pause);
    pause.queue = &q;
    pthread_t thread;
    CHECK(!pthread_create(&thread, NULL, slow_thief, &pause));
    wait_entered(&pause);
    CHECK(mfg_destroy(&q) == MSG_BUSY);
    CHECK(mfg_push(&q, &c) == MSG_OK && q.capacity == 4 && !c.data);
    /* Omitted array retention is caught before the paused thread touches it again. */
    CHECK(atomic_load(&m.frees) == 0);
    struct mfl_shared *won = NULL;
    CHECK(mfg_steal(&q, &won, NULL, NULL) == MSG_OK && number(won) == 1);
    mfl_drop(&won);
    /* Omitted payload retention is caught before any stale pointer is dereferenced. */
    CHECK(atomic_load(&m.drops) == 0 && atomic_load(&m.frees) == 0);
    resume(&pause);
    CHECK(!pthread_join(thread, NULL));
    CHECK(pause.status == MSG_CONFLICT);
    CHECK(atomic_load(&m.drops) == 1 && atomic_load(&m.frees) == 2);
    pause_destroy(&pause);
    CHECK(mfg_destroy(&q) == MSG_OK);
    CHECK(atomic_load(&m.drops) == 3);
    CHECK(atomic_load(&m.allocations) == atomic_load(&m.frees));
}

static void allocation_cuts(void)
{
    for (unsigned cut = 1; cut <= 2; cut++)
    {
        struct msg_memory m;
        memory_init(&m);
        struct mfg_deque q;
        CHECK(mfg_init(&q, allocator(&m), 4) == MSG_OK);
        struct mfo_value a = value(&m, 1), b = value(&m, 2), c = value(&m, 3);
        CHECK(mfg_push(&q, &a) == MSG_OK);
        CHECK(mfg_push(&q, &b) == MSG_OK);
        m.fail = atomic_load(&m.calls) + cut;
        size_t top = q.top, bottom = q.bottom;
        CHECK(mfg_push(&q, &c) == MSG_FAILED && c.data);
        CHECK(q.capacity == 2 && q.top == top && q.bottom == bottom);
        m.fail = 0;
        CHECK(mfg_push(&q, &c) == MSG_OK);
        CHECK(mfg_destroy(&q) == MSG_OK && atomic_load(&m.drops) == 3);
        CHECK(atomic_load(&m.allocations) == atomic_load(&m.frees));
    }
    struct msg_memory m;
    memory_init(&m);
    struct mfg_deque q;
    CHECK(mfg_init(&q, allocator(&m), 3) == MSG_INVALID);
    m.fail = 1;
    CHECK(mfg_init(&q, allocator(&m), 2) == MSG_FAILED);
    CHECK(atomic_load(&m.allocations) == 0);
    m.fail = 0;
    CHECK(mfg_init(&q, allocator(&m), 2) == MSG_OK);
    struct mfo_value a = value(&m, 7), b = value(&m, 8), c = value(&m, 9);
    CHECK(mfg_push(&q, &a) == MSG_OK);
    CHECK(mfg_push(&q, &b) == MSG_OK);
    CHECK(mfg_push(&q, &c) == MSG_FULL && c.data);
    struct mfl_shared *lease = NULL;
    CHECK(mfg_pop(&q, &lease) == MSG_OK && number(lease) == 8);
    mfl_drop(&lease);
    CHECK(mfg_pop(&q, &lease) == MSG_OK && number(lease) == 7);
    mfl_drop(&lease);
    CHECK(q.top == q.bottom);
    CHECK(mfg_pop(&q, &lease) == MSG_EMPTY && !lease);
    q.top = q.bottom = SIZE_MAX - 1;
    CHECK(mfg_push(&q, &c) == MSG_OVERFLOW && c.data);
    mfo_drop(&c);
    CHECK(mfg_destroy(&q) == MSG_OK);
    CHECK(atomic_load(&m.allocations) == atomic_load(&m.frees) &&
          atomic_load(&m.drops) == 3);
}

struct msg_work
{
    struct mfg_deque *q;
    atomic_bool done;
    atomic_uint seen[1000];
};

static void *consume(void *ctx)
{
    struct msg_work *w = ctx;
    for (;;)
    {
        struct mfl_shared *p = NULL;
        enum msg_status s = mfg_steal(w->q, &p, NULL, NULL);
        if (s == MSG_OK)
        {
            unsigned n = number(p);
            CHECK(n < 1000 && atomic_fetch_add(&w->seen[n], 1) == 0);
            mfl_drop(&p);
        }
        else
            CHECK(s == MSG_EMPTY || s == MSG_CONFLICT);
        if (s == MSG_EMPTY && atomic_load(&w->done))
            break;
        sched_yield();
    }
    return NULL;
}

static void concurrent(void)
{
    struct msg_memory m;
    memory_init(&m);
    struct mfg_deque q;
    CHECK(mfg_init(&q, allocator(&m), 64) == MSG_OK);
    struct msg_work w = {.q = &q};
    atomic_init(&w.done, false);
    for (unsigned i = 0; i < 1000; i++)
        atomic_init(&w.seen[i], 0);
    pthread_t a, b;
    CHECK(!pthread_create(&a, NULL, consume, &w));
    CHECK(!pthread_create(&b, NULL, consume, &w));
    for (unsigned i = 0; i < 1000; i++)
    {
        struct mfo_value v = value(&m, i);
        enum msg_status s;
        while ((s = mfg_push(&q, &v)) == MSG_FULL)
            sched_yield();
        CHECK(s == MSG_OK && !v.data);
    }
    atomic_store(&w.done, true);
    CHECK(!pthread_join(a, NULL));
    CHECK(!pthread_join(b, NULL));
    for (unsigned i = 0; i < 1000; i++)
        CHECK(atomic_load(&w.seen[i]) == 1);
    CHECK(mfg_destroy(&q) == MSG_OK);
    CHECK(atomic_load(&m.drops) == 1000 &&
          atomic_load(&m.allocations) == atomic_load(&m.frees));
}

static void *paused_snapshot(void *ctx)
{
    struct msg_pause *p = ctx;
    struct mfl_shared *lease = NULL;
    p->status = mfp_snapshot(p->pointer, &lease);
    CHECK(p->status == MSG_OK);
    pause_hook(p);
    p->observed = number(lease);
    mfl_drop(&lease);
    return NULL;
}

static void snapshots(void)
{
    struct msg_memory m;
    memory_init(&m);
    struct mfo_value v = value(&m, 5);
    struct mfl_shared *lease = NULL;
    CHECK(mfl_make(allocator(&m), &v, &lease) == MSG_OK && !v.data);
    struct mfp_pointer pointer;
    CHECK(mfp_init(&pointer, &lease) == MSG_OK && !lease);
    struct msg_pause pause;
    pause_init(&pause);
    pause.pointer = &pointer;
    pthread_t thread;
    CHECK(!pthread_create(&thread, NULL, paused_snapshot, &pause));
    wait_entered(&pause);
    v = value(&m, 9);
    CHECK(mfl_make(allocator(&m), &v, &lease) == MSG_OK);
    CHECK(mfp_replace(&pointer, &lease) == MSG_OK && !lease);
    CHECK(atomic_load(&m.drops) == 0);
    resume(&pause);
    CHECK(!pthread_join(thread, NULL));
    CHECK(pause.observed == 5 && atomic_load(&m.drops) == 1);
    pause_destroy(&pause);
    v = value(&m, 11);
    CHECK(mfl_make(allocator(&m), &v, &lease) == MSG_OK);
    pointer.sequence = UINT64_MAX - 1;
    CHECK(mfp_replace(&pointer, &lease) == MSG_OVERFLOW && lease);
    CHECK(number(lease) == 11);
    mfl_drop(&lease);
    CHECK(mfp_snapshot(&pointer, &lease) == MSG_OK && number(lease) == 9);
    CHECK(mfp_destroy(&pointer) == MSG_OK);
    CHECK(number(lease) == 9);
    mfl_drop(&lease);
    CHECK(atomic_load(&m.drops) == 3 &&
          atomic_load(&m.allocations) == atomic_load(&m.frees));
    /* The rejected design: merely saving raw data would not prevent the next drop. */
    v = value(&m, 17);
    CHECK(mfl_make(allocator(&m), &v, &lease) == MSG_OK);
    CHECK(mfp_init(&pointer, &lease) == MSG_OK);
    unsigned before = atomic_load(&m.drops);
    CHECK(mfp_destroy(&pointer) == MSG_OK);
    CHECK(atomic_load(&m.drops) == before + 1);
    /* No stale raw pointer is evaluated or dereferenced to turn this into a value oracle. */
}

static void references(void)
{
#ifdef MFL_REFERENCE_MAX
    struct msg_memory m;
    memory_init(&m);
    struct mfo_value v = value(&m, 42);
    struct mfl_shared *owners[MFL_REFERENCE_MAX];
    CHECK(mfl_make(allocator(&m), &v, &owners[0]) == MSG_OK);
    for (unsigned i = 1; i < MFL_REFERENCE_MAX; i++)
    {
        CHECK(mfl_retain(owners[0]) == MSG_OK);
        owners[i] = owners[0];
    }
    CHECK(mfl_retain(owners[0]) == MSG_OVERFLOW);
    for (unsigned i = 0; i < MFL_REFERENCE_MAX; i++)
    {
        CHECK(number(owners[i]) == 42);
        mfl_drop(&owners[i]);
        CHECK(!owners[i]);
        CHECK(atomic_load(&m.drops) == (unsigned)(i + 1 == MFL_REFERENCE_MAX));
    }
    CHECK(atomic_load(&m.drops) == 1 &&
          atomic_load(&m.allocations) == atomic_load(&m.frees));
#else
    CHECK(false);
#endif
}

static void oracle(void)
{
    char line[128];
    while (fgets(line, sizeof(line), stdin))
    {
        struct msg_memory m;
        memory_init(&m);
        struct mfg_deque q;
        CHECK(mfg_init(&q, allocator(&m), 8) == MSG_OK);
        unsigned serial = 0;
        for (size_t i = 0; line[i] && line[i] != '\n'; i++)
        {
            unsigned observed = 0;
            enum msg_status status;
            if (line[i] == 'a' || line[i] == 'f')
            {
                struct mfo_value v = value(&m, ++serial);
                if (line[i] == 'f')
                    m.fail = atomic_load(&m.calls) + 1;
                status = mfg_push(&q, &v);
                m.fail = 0;
                if (status != MSG_OK)
                    mfo_drop(&v);
                else
                    CHECK(!v.data);
            }
            else
            {
                struct mfl_shared *p = NULL;
                CHECK(line[i] == 'p' || line[i] == 's');
                status = line[i] == 'p' ? mfg_pop(&q, &p)
                                        : mfg_steal(&q, &p, NULL, NULL);
                if (status == MSG_OK)
                {
                    observed = number(p);
                    mfl_drop(&p);
                }
                else
                    CHECK(!p);
            }
            printf("%d:%u:%zu:%u,", status, observed, q.capacity,
                   atomic_load(&m.drops));
        }
        CHECK(mfg_destroy(&q) == MSG_OK);
        printf("D%u\n", atomic_load(&m.drops));
        CHECK(atomic_load(&m.allocations) == atomic_load(&m.frees));
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "growth"))
    {
        grow_pause();
        allocation_cuts();
        concurrent();
    }
    else if (!strcmp(argv[1], "snapshot"))
        snapshots();
    else if (!strcmp(argv[1], "references"))
        references();
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    else
        CHECK(false);
    puts("lifetime passed");
    return 0;
}
