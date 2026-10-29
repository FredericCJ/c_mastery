#include "stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "SC_CHECK: stream:%d:%s\n", __LINE__, #x); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

struct msg_sink
{
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    bool pause, entered, resume;
    atomic_uint count, ends, active, seen[2000];
    atomic_uint *drops;
    enum msg_status last;
    pthread_t callback_thread;
    bool thread_set;
    unsigned mode;
    struct mfq_handle callback_handle;
};

static void destroy_sink(void *ctx)
{
    struct msg_sink *s = ctx;
    CHECK(!atomic_load(&s->active));
    atomic_fetch_add(s->drops, 1);
    CHECK(!pthread_cond_destroy(&s->changed));
    CHECK(!pthread_mutex_destroy(&s->mutex));
    free(s);
}

static struct msg_sink *make_sink(atomic_uint *drops)
{
    struct msg_sink *s = calloc(1, sizeof(*s));
    CHECK(s);
    CHECK(!pthread_mutex_init(&s->mutex, NULL));
    CHECK(!pthread_cond_init(&s->changed, NULL));
    s->drops = drops;
    atomic_init(&s->count, 0);
    atomic_init(&s->ends, 0);
    atomic_init(&s->active, 0);
    for (unsigned i = 0; i < 2000; i++)
        atomic_init(&s->seen[i], 0);
    return s;
}

static void callback_enter(struct msg_sink *s)
{
    CHECK(atomic_fetch_add(&s->active, 1) == 0);
    if (!s->thread_set)
    {
        s->callback_thread = pthread_self();
        s->thread_set = true;
    }
    else
        CHECK(pthread_equal(s->callback_thread, pthread_self()));
}

static void next(void *ctx, unsigned value)
{
    struct msg_sink *s = ctx;
    callback_enter(s);
    CHECK(value < 2000);
    atomic_fetch_add(&s->seen[value], 1);
    atomic_fetch_add(&s->count, 1);
    pthread_mutex_lock(&s->mutex);
    if (s->pause && !s->entered)
    {
        s->entered = true;
        pthread_cond_broadcast(&s->changed);
        while (!s->resume)
            pthread_cond_wait(&s->changed, &s->mutex);
    }
    pthread_mutex_unlock(&s->mutex);
    if (s->mode == 1 && value == 0)
        CHECK(mfq_request(&s->callback_handle, 1) == MSG_OK);
    if (s->mode == 2)
    {
        CHECK(mfq_request(&s->callback_handle, 0) == MSG_OK);
        CHECK(mfq_cancel(&s->callback_handle) == MSG_OK);
    }
    if (s->mode == 3)
    {
        CHECK(mfq_submit(&s->callback_handle, 99, MFQ_WAIT) == MSG_BUSY);
        CHECK(mfq_cancel(&s->callback_handle) == MSG_OK);
    }
    CHECK(atomic_fetch_sub(&s->active, 1) == 1);
}

static void end(void *ctx, enum msg_status status)
{
    struct msg_sink *s = ctx;
    callback_enter(s);
    s->last = status;
    CHECK(atomic_fetch_add(&s->ends, 1) == 0);
    CHECK(atomic_fetch_sub(&s->active, 1) == 1);
}

static void wait_entered(struct msg_sink *s)
{
    pthread_mutex_lock(&s->mutex);
    while (!s->entered)
        pthread_cond_wait(&s->changed, &s->mutex);
    pthread_mutex_unlock(&s->mutex);
}

static void resume(struct msg_sink *s)
{
    pthread_mutex_lock(&s->mutex);
    s->resume = true;
    pthread_cond_broadcast(&s->changed);
    pthread_mutex_unlock(&s->mutex);
}

static void initialize(struct mfq_stream *s, struct msg_sink *sink,
                       unsigned cap, struct mfq_handle *h)
{
    struct mfo_value v = {sink, destroy_sink};
    CHECK(mfq_init(s, cap, &v, next, end, false) == MSG_OK && !v.data);
    CHECK(mfq_acquire(s, h) == MSG_OK);
}

static void stop(struct mfq_stream *s, struct mfq_handle *h,
                 struct msg_sink *sink)
{
    CHECK(mfq_close(s) == MSG_OK);
    CHECK(mfq_join(s) == MSG_OK);
    mfq_release(h);
    mfq_release(&sink->callback_handle);
    CHECK(mfq_destroy(s) == MSG_OK);
}

static void executor(void)
{
    atomic_uint drops;
    atomic_init(&drops, 0);
    struct msg_sink *sink = make_sink(&drops);
    struct mfq_stream s;
    struct mfq_handle h = {0};
    struct mfo_value initial = {sink, destroy_sink};
    CHECK(mfq_init(&s, 0, &initial, next, end, false) == MSG_INVALID &&
          initial.data);
    CHECK(mfq_init(&s, 2, &initial, next, end, true) == MSG_FAILED &&
          initial.data && atomic_load(&drops) == 0);
    initialize(&s, sink, 2, &h);
    pthread_mutex_lock(&s.mutex);
    s.stats.handles = UINT_MAX;
    pthread_mutex_unlock(&s.mutex);
    struct mfq_handle refused = {0};
    CHECK(mfq_acquire(&s, &refused) == MSG_OVERFLOW && !refused.owner);
    pthread_mutex_lock(&s.mutex);
    s.stats.handles = 1;
    s.stats.accepted = UINT64_MAX;
    pthread_mutex_unlock(&s.mutex);
    CHECK(mfq_submit(&h, 42, MFQ_DROP) == MSG_OVERFLOW);
    pthread_mutex_lock(&s.mutex);
    s.stats.accepted = 0;
    pthread_mutex_unlock(&s.mutex);
    CHECK(mfq_join(&s) == MSG_BUSY);
    CHECK(mfq_submit(&h, 0, MFQ_DROP) == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK);
    struct mfq_stats stats;
    CHECK(mfq_stats(&s, &stats) == MSG_OK && stats.queued == 1 &&
          stats.delivered == 0);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK);
    CHECK(atomic_load(&sink->count) == 1);
    CHECK(mfq_request(&h, UINT64_MAX) == MSG_OK);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_submit(&h, 1, MFQ_WAIT) == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK && atomic_load(&sink->count) == 2);
    CHECK(mfq_end(&h, MSG_OK) == MSG_OK);
    CHECK(mfq_join(&s) == MSG_OK);
    CHECK(atomic_load(&sink->ends) == 1 && sink->last == MSG_OK);
    CHECK(mfq_end(&h, MSG_FAILED) == MSG_CLOSED &&
          mfq_submit(&h, 2, MFQ_DROP) == MSG_CLOSED);
    CHECK(mfq_destroy(&s) == MSG_BUSY && atomic_load(&drops) == 0);
    mfq_release(&h);
    CHECK(mfq_destroy(&s) == MSG_OK && atomic_load(&drops) == 1);
    sink = make_sink(&drops);
    sink->pause = true;
    initialize(&s, sink, 2, &h);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_submit(&h, 4, MFQ_WAIT) == MSG_OK);
    wait_entered(sink);
    CHECK(mfq_cancel(&h) == MSG_OK);
    CHECK(mfq_end(&h, MSG_OK) == MSG_CLOSED);
    CHECK(mfq_destroy(&s) == MSG_BUSY && atomic_load(&drops) == 1);
    resume(sink);
    CHECK(mfq_join(&s) == MSG_OK && atomic_load(&sink->ends) == 0);
    CHECK(mfq_destroy(&s) == MSG_BUSY);
    mfq_release(&h);
    CHECK(mfq_destroy(&s) == MSG_OK && atomic_load(&drops) == 2);
    sink = make_sink(&drops);
    initialize(&s, sink, 2, &h);
    CHECK(mfq_request(&h, 0) == MSG_OK);
    CHECK(mfq_join(&s) == MSG_OK && atomic_load(&sink->ends) == 1 &&
          sink->last == MSG_INVALID);
    mfq_release(&h);
    CHECK(mfq_destroy(&s) == MSG_OK);
    sink = make_sink(&drops);
    sink->mode = 1;
    initialize(&s, sink, 2, &h);
    CHECK(mfq_acquire(&s, &sink->callback_handle) == MSG_OK);
    CHECK(mfq_submit(&h, 0, MFQ_WAIT) == MSG_OK);
    CHECK(mfq_submit(&h, 1, MFQ_WAIT) == MSG_OK);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK && atomic_load(&sink->count) == 2);
    stop(&s, &h, sink);
    sink = make_sink(&drops);
    sink->mode = 2;
    initialize(&s, sink, 2, &h);
    CHECK(mfq_acquire(&s, &sink->callback_handle) == MSG_OK);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_submit(&h, 2, MFQ_WAIT) == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK);
    CHECK(mfq_join(&s) == MSG_OK && atomic_load(&sink->ends) == 0);
    stop(&s, &h, sink);
    CHECK(atomic_load(&drops) == 5);
}

struct msg_producer
{
    struct mfq_handle handle;
    unsigned first, count;
    enum msg_status status;
};

static void *produce(void *ctx)
{
    struct msg_producer *p = ctx;
    p->status = MSG_OK;
    for (unsigned i = 0; i < p->count; i++)
    {
        enum msg_status r = mfq_submit(&p->handle, p->first + i, MFQ_WAIT);
        if (r != MSG_OK)
        {
            p->status = r;
            break;
        }
    }
    mfq_release(&p->handle);
    return NULL;
}

static void wait_blocked(struct mfq_stream *s)
{
    pthread_mutex_lock(&s->mutex);
    while (!s->stats.waiting)
        pthread_cond_wait(&s->changed, &s->mutex);
    pthread_mutex_unlock(&s->mutex);
}

static void overload(void)
{
    atomic_uint drops;
    atomic_init(&drops, 0);
    for (unsigned policy = MFQ_WAIT; policy <= MFQ_FAIL; policy++)
    {
        struct msg_sink *sink = make_sink(&drops);
        sink->pause = true;
        struct mfq_stream s;
        struct mfq_handle h = {0};
        initialize(&s, sink, 2, &h);
        CHECK(mfq_request(&h, UINT64_MAX) == MSG_OK);
        CHECK(mfq_submit(&h, 0, MFQ_WAIT) == MSG_OK);
        wait_entered(sink);
        CHECK(mfq_submit(&h, 1, MFQ_DROP) == MSG_OK);
        CHECK(mfq_submit(&h, 2, MFQ_DROP) == MSG_OK);
        struct msg_producer p = {.first = 3, .count = 1};
        pthread_t thread;
        if (policy == MFQ_WAIT)
        {
            CHECK(mfq_acquire(&s, &p.handle) == MSG_OK);
            CHECK(!pthread_create(&thread, NULL, produce, &p));
            wait_blocked(&s);
        }
        else
            CHECK(mfq_submit(&h, 3, (enum mfq_policy)policy) ==
                  (policy == MFQ_DROP ? MSG_FULL : MSG_FAILED));
        struct mfq_stats stats;
        CHECK(mfq_stats(&s, &stats) == MSG_OK && stats.accepted == 3 &&
              stats.inflight == 1 && stats.queued == 2 && stats.delivered == 0);
        CHECK(atomic_load(&drops) == policy);
        resume(sink);
        if (policy == MFQ_WAIT)
        {
            CHECK(!pthread_join(thread, NULL) && p.status == MSG_OK);
        }
        if (policy != MFQ_FAIL)
        {
            CHECK(mfq_idle(&s) == MSG_OK);
            CHECK(mfq_end(&h, MSG_OK) == MSG_OK);
        }
        CHECK(mfq_join(&s) == MSG_OK);
        CHECK(mfq_stats(&s, &stats) == MSG_OK && stats.queued == 0 &&
              stats.inflight == 0 &&
              stats.accepted == stats.delivered + stats.discarded);
        CHECK(stats.delivered == (policy == MFQ_WAIT   ? 4
                                  : policy == MFQ_DROP ? 3
                                                       : 1));
        CHECK(stats.discarded == (policy == MFQ_FAIL ? 2 : 0));
        CHECK(stats.rejected == (policy == MFQ_WAIT ? 0 : 1));
        CHECK(atomic_load(&sink->ends) == 1 &&
              sink->last == (policy == MFQ_FAIL ? MSG_FAILED : MSG_OK));
        mfq_release(&h);
        CHECK(mfq_destroy(&s) == MSG_OK);
    }
    struct msg_sink *sink = make_sink(&drops);
    sink->pause = true;
    sink->mode = 3;
    struct mfq_stream s;
    struct mfq_handle h = {0};
    initialize(&s, sink, 1, &h);
    CHECK(mfq_acquire(&s, &sink->callback_handle) == MSG_OK);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_submit(&h, 0, MFQ_WAIT) == MSG_OK);
    wait_entered(sink);
    CHECK(mfq_submit(&h, 1, MFQ_WAIT) == MSG_OK);
    resume(sink);
    CHECK(mfq_idle(&s) == MSG_OK);
    CHECK(mfq_join(&s) == MSG_OK && atomic_load(&sink->count) == 1 &&
          atomic_load(&sink->ends) == 0);
    stop(&s, &h, sink);
    sink = make_sink(&drops);
    sink->pause = true;
    initialize(&s, sink, 1, &h);
    CHECK(mfq_request(&h, 1) == MSG_OK);
    CHECK(mfq_submit(&h, 0, MFQ_WAIT) == MSG_OK);
    wait_entered(sink);
    CHECK(mfq_submit(&h, 1, MFQ_WAIT) == MSG_OK);
    struct msg_producer blocked = {.first = 2, .count = 1};
    pthread_t blocked_thread;
    CHECK(mfq_acquire(&s, &blocked.handle) == MSG_OK);
    CHECK(!pthread_create(&blocked_thread, NULL, produce, &blocked));
    wait_blocked(&s);
    CHECK(mfq_cancel(&h) == MSG_OK);
    CHECK(!pthread_join(blocked_thread, NULL) && blocked.status == MSG_CLOSED);
    CHECK(atomic_load(&drops) == 4);
    resume(sink);
    CHECK(mfq_join(&s) == MSG_OK);
    stop(&s, &h, sink);
    sink = make_sink(&drops);
    initialize(&s, sink, 4, &h);
    CHECK(mfq_request(&h, UINT64_MAX) == MSG_OK);
    struct msg_producer producers[2] = {{.first = 0, .count = 1000},
                                        {.first = 1000, .count = 1000}};
    pthread_t threads[2];
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(mfq_acquire(&s, &producers[i].handle) == MSG_OK);
        CHECK(!pthread_create(&threads[i], NULL, produce, &producers[i]));
    }
    for (unsigned i = 0; i < 2; i++)
        CHECK(!pthread_join(threads[i], NULL) && producers[i].status == MSG_OK);
    CHECK(mfq_idle(&s) == MSG_OK);
    CHECK(atomic_load(&sink->count) == 2000);
    for (unsigned i = 0; i < 2000; i++)
        CHECK(atomic_load(&sink->seen[i]) == 1);
    stop(&s, &h, sink);
    CHECK(atomic_load(&drops) == 6);
}

struct msg_plain
{
    unsigned count, ends;
    enum msg_status terminal;
};

static void plain_next(void *ctx, unsigned v)
{
    (void)v;
    ++((struct msg_plain *)ctx)->count;
}

static void plain_end(void *ctx, enum msg_status s)
{
    struct msg_plain *p = ctx;
    ++p->ends;
    p->terminal = s;
}

static void oracle(void)
{
    char line[128];
    while (fgets(line, sizeof(line), stdin))
    {
        struct msg_plain p = {0};
        struct msg_stream s;
        msg_stream_init(&s, plain_next, plain_end, &p);
        for (size_t i = 0; line[i] && line[i] != '\n'; i++)
        {
            enum msg_status status;
            switch (line[i])
            {
                case 'r':
                    status = msg_request(&s, 1);
                    break;
                case 'u':
                    status = msg_request(&s, UINT64_MAX);
                    break;
                case 'z':
                    status = msg_request(&s, 0);
                    break;
                case 'e':
                    status = msg_emit(&s, 42);
                    break;
                case 'c':
                    msg_stream_cancel(&s);
                    status = MSG_OK;
                    break;
                case 't':
                    status = msg_stream_end(&s, MSG_OK);
                    break;
                default:
                    CHECK(line[i] == 'f');
                    status = msg_stream_end(&s, MSG_FAILED);
                    break;
            }
            printf("%d:%llu:%u:%u:%d,", status, (unsigned long long)s.demand,
                   p.count, p.ends, p.terminal);
        }
        puts("");
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "executor"))
        executor();
    else if (!strcmp(argv[1], "overload"))
        overload();
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    else
        CHECK(false);
    puts("stream passed");
    return 0;
}
