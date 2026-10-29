#include "owned.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "SC_CHECK: owned_follow:%d:%s\n", __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct msg_payload
{
    atomic_uint *drops;
    unsigned serial;
};

static void release(void *arg)
{
    struct msg_payload *p = arg;
    atomic_fetch_add(p->drops, 1);
    free(p);
}

static struct mfo_value value(atomic_uint *d, unsigned serial)
{
    struct msg_payload *p = malloc(sizeof(*p));
    CHECK(p);
    *p = (struct msg_payload){d, serial};
    return (struct mfo_value){p, release};
}

struct msg_sender
{
    struct mfc_hub *h;
    struct mfo_value value;
    enum msg_status result;
    atomic_bool entered, go;
    bool hook;
};

static void after_submit(void *arg)
{
    struct msg_sender *s = arg;
    atomic_store(&s->entered, true);
    while (!atomic_load(&s->go))
        sched_yield();
}

static void *send_one(void *arg)
{
    struct msg_sender *s = arg;
    s->result =
        mfc_send(s->h, 0, &s->value, true, s->hook ? after_submit : NULL, s);
    return NULL;
}

static void buffered(void)
{
    struct mfc_hub h;
    atomic_uint drops;
    atomic_init(&drops, 0);
    unsigned channel = 77;
    struct mfo_value out = {0};
    CHECK(mfc_init(&h, 1, 1) == MSG_OK);
    struct mfo_value one = value(&drops, 1), two = value(&drops, 2);
    CHECK(mfc_send(&h, 0, &one, false, NULL, NULL) == MSG_OK && !one.data);
    void *previous = two.data;
    CHECK(mfc_send(&h, 0, &two, false, NULL, NULL) == MSG_FULL &&
          two.data == previous);
    CHECK(mfc_close(&h, 0) == MSG_OK);
    CHECK(mfc_send(&h, 0, &two, false, NULL, NULL) == MSG_CLOSED &&
          two.data == previous);
    CHECK(mfc_select(&h, 1, false, &channel, &out) == MSG_OK && channel == 0 &&
          ((struct msg_payload *)out.data)->serial == 1);
    mfo_drop(&out);
    CHECK(mfc_select(&h, 1, false, &channel, &out) == MSG_CLOSED && !out.data);
    CHECK(mfc_send(&h, 1, &two, false, NULL, NULL) == MSG_OK && !two.data);
    mfc_cancel(&h);
    CHECK(atomic_load(&drops) == 1);
    CHECK(mfc_select(&h, 2, false, &channel, &out) == MSG_CANCELLED &&
          !out.data);
    CHECK(mfc_destroy(&h) == MSG_OK && atomic_load(&drops) == 2);
}

static void rendezvous(bool accepted)
{
    struct mfc_hub h;
    atomic_uint drops;
    atomic_init(&drops, 0);
    CHECK(mfc_init(&h, 0, 1) == MSG_OK);
    struct msg_sender sender = {
        .h = &h, .value = value(&drops, 3), .hook = accepted};
    atomic_init(&sender.entered, false);
    atomic_init(&sender.go, false);
    pthread_t thread;
    CHECK(!pthread_create(&thread, NULL, send_one, &sender));
    if (accepted)
    {
        struct mfo_value out = {0};
        unsigned channel;
        CHECK(mfc_select(&h, 1, true, &channel, &out) == MSG_OK &&
              ((struct msg_payload *)out.data)->serial == 3);
        mfo_drop(&out);
        while (!atomic_load(&sender.entered))
            sched_yield();
        pthread_mutex_lock(&h.records_mutex);
        unsigned used = 0;
        for (unsigned i = 0; i < 8; i++)
            used += h.records[i].used;
        pthread_mutex_unlock(&h.records_mutex);
        CHECK(used ==
              1); /* Receiver finish cannot recycle a sender's live record. */
        struct mfo_value other = value(&drops, 4);
        CHECK(mfc_send(&h, 1, &other, false, NULL, NULL) == MSG_OK);
        CHECK(mfc_destroy(&h) == MSG_BUSY);
        mfc_cancel(&h);
        atomic_store(&sender.go, true);
        CHECK(!pthread_join(thread, NULL));
        CHECK(sender.result == MSG_OK && !sender.value.data &&
              atomic_load(&drops) == 1);
        CHECK(mfc_destroy(&h) == MSG_OK && atomic_load(&drops) == 2);
    }
    else
    {
        bool offered = false;
        while (!offered)
        {
            pthread_mutex_lock(&h.hub.mutex);
            offered = h.hub.channel[0].offered;
            pthread_cond_broadcast(&h.hub.changed);
            pthread_mutex_unlock(&h.hub.mutex);
            sched_yield();
        }
        mfc_cancel(&h);
        CHECK(!pthread_join(thread, NULL));
        CHECK(sender.result == MSG_CANCELLED && sender.value.data &&
              atomic_load(&drops) == 0);
        mfo_drop(&sender.value);
        CHECK(mfc_destroy(&h) == MSG_OK && atomic_load(&drops) == 1);
    }
}

struct msg_producer
{
    struct mfc_hub *h;
    atomic_uint *drops;
    unsigned channel;
};

static void *produce(void *arg)
{
    struct msg_producer *p = arg;
    for (unsigned i = 0; i < 500; i++)
    {
        struct mfo_value v = value(p->drops, 500 * p->channel + i);
        CHECK(mfc_send(p->h, p->channel, &v, true, NULL, NULL) == MSG_OK &&
              !v.data);
    }
    CHECK(mfc_close(p->h, p->channel) == MSG_OK);
    return NULL;
}

static void threaded(void)
{
    struct mfc_hub h;
    CHECK(mfc_init(&h, 0, 4) == MSG_OK);
    atomic_uint drops;
    atomic_init(&drops, 0);
    struct msg_producer p[2] = {{&h, &drops, 0}, {&h, &drops, 1}};
    pthread_t threads[2];
    CHECK(!pthread_create(&threads[0], NULL, produce, &p[0]));
    CHECK(!pthread_create(&threads[1], NULL, produce, &p[1]));
    bool seen[1000] = {false};
    unsigned count = 0;
    for (;;)
    {
        unsigned channel;
        struct mfo_value out = {0};
        enum msg_status s = mfc_select(&h, 3, true, &channel, &out);
        if (s == MSG_CLOSED)
            break;
        CHECK(s == MSG_OK && out.data);
        unsigned serial = ((struct msg_payload *)out.data)->serial;
        CHECK(serial < 1000 && serial / 500 == channel && !seen[serial]);
        seen[serial] = true;
        count++;
        mfo_drop(&out);
        pthread_mutex_lock(&h.hub.mutex);
        pthread_cond_broadcast(&h.hub.changed);
        pthread_mutex_unlock(&h.hub.mutex);
    }
    CHECK(!pthread_join(threads[0], NULL) && !pthread_join(threads[1], NULL));
    CHECK(count == 1000 && atomic_load(&drops) == 1000 &&
          mfc_destroy(&h) == MSG_OK);
}

struct actor_ctx
{
    struct mfa_actor *a;
    atomic_uint *drops;
    unsigned calls, dead;
    bool fail, self;
};

static bool handle(void *arg, const struct mfo_value *v)
{
    struct actor_ctx *c = arg;
    CHECK(v->data);
    c->calls++;
    CHECK(mfa_step(c->a) == MSG_BUSY && mfa_destroy(c->a) == MSG_BUSY);
    if (c->self)
    {
        c->self = false;
        struct mfo_value next = value(c->drops, 99);
        CHECK(mfa_send(c->a, c->a->actor.generation, &next) == MSG_OK &&
              !next.data);
    }
    return !c->fail;
}

static void dead(void *arg, uint64_t generation, struct mfo_value v)
{
    struct actor_ctx *c = arg;
    CHECK(generation == 1 && v.data);
    c->dead++;
    mfo_drop(&v);
}

static void actors(void)
{
    struct mfa_actor a;
    atomic_uint drops;
    atomic_init(&drops, 0);
    struct actor_ctx ctx = {&a, &drops, 0, 0, true, true};
    CHECK(mfa_init(&a, handle, &ctx, dead, &ctx) == MSG_OK);
    struct mfo_value first = value(&drops, 1), queued = value(&drops, 2);
    CHECK(mfa_send(&a, 1, &first) == MSG_OK &&
          mfa_send(&a, 1, &queued) == MSG_OK);
    CHECK(mfa_step(&a) == MSG_FAILED && ctx.calls == 1 &&
          atomic_load(&drops) == 3 && a.actor.count == 0);
    CHECK(mfa_restart(&a) == MSG_OK && a.actor.generation == 2);
    struct mfo_value stale = value(&drops, 3);
    CHECK(mfa_send(&a, 1, &stale) == MSG_STALE && !stale.data &&
          ctx.dead == 1 && atomic_load(&drops) == 4);
    ctx.fail = false;
    ctx.self = true;
    struct mfo_value current = value(&drops, 4);
    CHECK(mfa_send(&a, 2, &current) == MSG_OK && mfa_step(&a) == MSG_OK &&
          ctx.calls == 2 && a.actor.count == 1);
    CHECK(mfa_step(&a) == MSG_OK && ctx.calls == 3 && atomic_load(&drops) == 6);
    for (unsigned i = 0; i < 4; i++)
    {
        struct mfo_value v = value(&drops, i);
        CHECK(mfa_send(&a, 2, &v) == MSG_OK);
    }
    struct mfo_value refused = value(&drops, 7);
    void *ptr = refused.data;
    CHECK(mfa_send(&a, 2, &refused) == MSG_FULL && refused.data == ptr);
    mfo_drop(&refused);
    CHECK(mfa_destroy(&a) == MSG_OK && atomic_load(&drops) == 11);
    a.actor.generation = UINT64_MAX;
    CHECK(mfa_restart(&a) == MSG_OVERFLOW);
}

static void oracle(void)
{
    char word[64];
    while (fgets(word, sizeof(word), stdin))
    {
        struct mfc_hub h;
        atomic_uint drops;
        atomic_init(&drops, 0);
        unsigned serial = 0;
        CHECK(mfc_init(&h, 2, 1) == MSG_OK);
        for (size_t i = 0; word[i] && word[i] != '\n'; i++)
        {
            char c = word[i];
            enum msg_status status;
            unsigned selected = 77, observed = 0;
            struct mfo_value out = {0};
            if (c == 'a' || c == 'b')
            {
                struct mfo_value v = value(&drops, ++serial);
                status =
                    mfc_send(&h, (unsigned)(c - 'a'), &v, false, NULL, NULL);
                if (status == MSG_OK)
                    CHECK(!v.data);
                else
                {
                    CHECK(v.data);
                    mfo_drop(&v);
                }
            }
            else if (c == 'r')
            {
                status = mfc_select(&h, 3, false, &selected, &out);
                if (status == MSG_OK)
                {
                    observed = ((struct msg_payload *)out.data)->serial;
                    mfo_drop(&out);
                }
            }
            else if (c == 'c' || c == 'd')
                status = mfc_close(&h, (unsigned)(c - 'c'));
            else
            {
                CHECK(c == 'x');
                mfc_cancel(&h);
                status = MSG_OK;
            }
            printf("%u:%u:%u:%u,", (unsigned)status, selected, observed,
                   atomic_load(&drops));
        }
        CHECK(mfc_destroy(&h) == MSG_OK);
        printf("D%u\n", atomic_load(&drops));
    }
}

struct msg_lost
{
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    atomic_bool checked, published;
};

static void *notify_before_registration(void *arg)
{
    struct msg_lost *l = arg;
    while (!atomic_load(&l->checked))
        sched_yield();
    atomic_store(&l->published, true);
    CHECK(!pthread_cond_signal(&l->changed));
    return NULL;
}

static void lost_notification(void)
{
    /* Original local deliberately flawed split registration, not an OS defect. */
    struct msg_lost l;
    CHECK(!pthread_mutex_init(&l.mutex, NULL) &&
          !pthread_cond_init(&l.changed, NULL));
    atomic_init(&l.checked, false);
    atomic_init(&l.published, false);
    pthread_t producer;
    CHECK(!pthread_create(&producer, NULL, notify_before_registration, &l));
    CHECK(!atomic_load(&l.published));
    atomic_store(&l.checked, true);
    CHECK(!pthread_join(producer,
                        NULL)); /* Notification is definitely before wait. */
    struct timespec deadline;
    CHECK(timespec_get(&deadline, TIME_UTC) == TIME_UTC);
    deadline.tv_nsec += 1000000;
    if (deadline.tv_nsec >= 1000000000)
    {
        deadline.tv_sec++;
        deadline.tv_nsec -= 1000000000;
    }
    CHECK(!pthread_mutex_lock(&l.mutex));
    int result = pthread_cond_timedwait(&l.changed, &l.mutex, &deadline);
    /* Spurious wake is permitted: even a success cannot prove notification delivery. */
    CHECK((result == ETIMEDOUT || result == 0) && atomic_load(&l.published));
    printf(
        "split-registration observed timedwait=%d ready=1 (spurious wake permitted)\n",
        result);
    CHECK(!pthread_mutex_unlock(&l.mutex));
    CHECK(!pthread_cond_destroy(&l.changed) &&
          !pthread_mutex_destroy(&l.mutex));
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    if (!strcmp(argv[1], "channels"))
    {
        buffered();
        rendezvous(true);
        rendezvous(false);
        threaded();
        lost_notification();
    }
    else if (!strcmp(argv[1], "actors"))
        actors();
    else
        return 2;
    puts("owned messaging transfer and destruction checks passed");
    return 0;
}
