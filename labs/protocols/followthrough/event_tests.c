#include "event_follow.h"
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "PE_CHECK:%s:%d\n", #x, __LINE__); \
            exit(1);                                           \
        }                                                      \
    } while (0)

struct pf_entry
{
    struct pf_hsm *h;
    unsigned entries, reentries;
};

static int entry(void *p, unsigned state)
{
    struct pf_entry *e = p;
    ++e->entries;
    unsigned char t[6];
    size_t n = 0;
    C(pf_dispatch(e->h, PT_PING, 0, 0, 0, 0, t, 6, &n) == PT_STATE);
    ++e->reentries;
    if (state == PT_IDLE)
    {
        C(pf_post(e->h, PT_START) == PT_OK);
    }
    return 0;
}

static int hsm(void)
{
    struct pf_hsm h = {.machine = {PT_OFFLINE, 0, 0}};
    struct pf_entry e = {&h, 0, 0};
    unsigned char t[6] = {99};
    size_t n = 99;
    C(pf_dispatch(&h, PT_CONNECT, 0, 0, entry, &e, t, 2, &n) == PT_RANGE &&
      h.machine.active == PT_OFFLINE && !e.entries && n == 99 && t[0] == 99);
    C(pf_dispatch(&h, PT_CONNECT, 0, 0, entry, &e, t, 6, &n) == PT_OK &&
      h.machine.active == PT_IDLE && h.count == 1 && e.entries == 2 &&
      e.reentries == 2);
    C(h.events[0] == PT_START);
    C(pf_dispatch(&h, h.events[0], 0, 0, 0, 0, t, 6, &n) == PT_OK &&
      h.machine.active == PT_BUSY);
    C(pf_dispatch(&h, PF_GUARDED, 1, 0, 0, 0, t, 6, &n) == PT_OK && n == 0 &&
      h.machine.active == PT_BUSY);
    C(pf_dispatch(&h, PF_GUARDED, 0, 0, 0, 0, t, 6, &n) == PT_OK &&
      h.machine.active == PT_FAULT && n == 3 && t[0] == (0x80 | PT_BUSY) &&
      t[1] == (0x80 | PT_ONLINE));
    for (unsigned local = 0; local < 2; ++local)
    {
        h.machine.active = PT_BUSY;
        C(pf_dispatch(&h, PF_LOCAL, 0, local, 0, 0, t, 6, &n) == PT_OK &&
          n == (local ? 2 : 4) && t[0] == (0x80 | PT_BUSY) &&
          t[n - 1] == PT_IDLE);
        if (!local)
        {
            C(t[1] == (0x80 | PT_ONLINE) && t[2] == PT_ONLINE);
        }
    }
    return 0;
}

struct pf_concurrent
{
    struct pf_active a;
    pthread_mutex_t mutex;
    atomic_uint accepted, dropped, processed, done;
};

static void enter(void *p)
{
    C(!pthread_mutex_lock(p));
}

static void leave(void *p)
{
    C(!pthread_mutex_unlock(p));
}

static void *producer(void *p)
{
    struct pf_concurrent *c = p;
    for (unsigned i = 0; i < 4000; ++i)
    {
        int r = pf_active_post(&c->a, PT_PING);
        C(r == PT_OK || r == PT_RANGE);
        atomic_fetch_add(r == PT_OK ? &c->accepted : &c->dropped, 1);
    }
    return 0;
}

static void *consumer(void *p)
{
    struct pf_concurrent *c = p;
    for (;;)
    {
        unsigned char t[6];
        size_t n = 9;
        int r = pf_active_step(&c->a, t, 6, &n);
        C(r == PT_OK || r == PT_PENDING);
        if (r == PT_OK)
        {
            C(!n);
            atomic_fetch_add(&c->processed, 1);
        }
        else if (atomic_load(&c->done))
        {
            return 0;
        }
        else
        {
            sched_yield();
        }
    }
}

static int active(void)
{
    struct pf_concurrent c = {.a = {.hsm = {.machine = {PT_OFFLINE, 0, 0}}}};
    C(!pthread_mutex_init(&c.mutex, 0));
    c.a.lock = (struct pf_lock){&c.mutex, enter, leave};
    C(pf_active_post(&c.a, PT_START) == PT_OK);
    unsigned char t[6];
    size_t n = 19;
    C(pf_active_step(&c.a, t, 6, &n) == PT_STATE && c.a.hsm.count == 1 &&
      n == 19);
    unsigned event = 99;
    C(pf_active_discard(&c.a, &event) == PT_OK && event == PT_START &&
      c.a.discarded == 1);
    C(pf_active_discard(&c.a, &event) == PT_PENDING && event == PT_START);
    pthread_t p[2], q[2];
    for (unsigned i = 0; i < 2; ++i)
    {
        C(!pthread_create(&p[i], 0, producer, &c));
        C(!pthread_create(&q[i], 0, consumer, &c));
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        C(!pthread_join(p[i], 0));
    }
    atomic_store(&c.done, 1);
    for (unsigned i = 0; i < 2; ++i)
    {
        C(!pthread_join(q[i], 0));
    }
    C(atomic_load(&c.accepted) + atomic_load(&c.dropped) == 8000);
    C(atomic_load(&c.accepted) == atomic_load(&c.processed) &&
      c.a.hsm.machine.pings == atomic_load(&c.accepted) && !c.a.hsm.count);
    C(!pthread_mutex_destroy(&c.mutex));
    printf("PE_COUNTS:accepted=%u dropped=%u\n", atomic_load(&c.accepted),
           atomic_load(&c.dropped));
    return 0;
}

struct pf_tree
{
    unsigned calls[16], aborts[16], first_occurrence;
    struct pf_reactive *frame;
    const struct pf_node *nodes;
};

static unsigned leaf(void *p, unsigned occurrence, unsigned node)
{
    struct pf_tree *t = p;
    (void)node;
    ++t->calls[occurrence];
    if (!t->first_occurrence)
    {
        t->first_occurrence = occurrence;
    }
    return occurrence == t->first_occurrence ? PT_BT_SUCCESS : PT_BT_RUNNING;
}

static void abort_leaf(void *p, unsigned occurrence, unsigned node)
{
    struct pf_tree *t = p;
    (void)node;
    C(occurrence < 16);
    ++t->aborts[occurrence];
}

static int reactive(void)
{
    /* Both edges name the same immutable leaf; their mutable occurrences differ. */
    struct pf_node nodes[] = {{PT_BT_SEQUENCE, 1, 1}, {PT_BT_LEAF, 0, 0}};
    struct pf_reactive b;
    C(pf_reactive_init(&b, nodes, 2) == PT_OK && b.count == 3);
    struct pf_tree ctx = {.frame = &b, .nodes = nodes};
    unsigned out = 99;
    C(pf_reactive_tick(&b, nodes, 2, 1, leaf, abort_leaf, &ctx, &out) ==
          PT_OK &&
      out == PT_BT_RUNNING && ctx.calls[1] == 1 && ctx.calls[2] == 1);
    C(pf_reactive_tick(&b, nodes, 2, 1, leaf, abort_leaf, &ctx, &out) ==
          PT_OK &&
      ctx.calls[1] == 1 && ctx.calls[2] == 2);
    C(pf_reactive_tick(&b, nodes, 2, 0, leaf, abort_leaf, &ctx, &out) ==
          PT_OK &&
      out == PT_BT_FAILURE && ctx.aborts[2] == 1 && !ctx.aborts[1]);
    C(pf_reactive_cancel(&b, abort_leaf, &ctx) == PT_OK && ctx.aborts[2] == 1);
    C(pf_reactive_tick(&b, nodes, 2, 1, leaf, abort_leaf, &ctx, &out) ==
          PT_OK &&
      ctx.calls[1] == 2);
    C(pf_reactive_cancel(&b, abort_leaf, &ctx) == PT_OK && ctx.aborts[2] == 2);
    struct pf_reactive old = b;
    nodes[0].left = 0;
    C(pf_reactive_init(&b, nodes, 2) == PT_RANGE &&
      !memcmp(&b, &old, sizeof b));
    struct pf_node exploding[8];
    for (unsigned i = 0; i < 7; ++i)
    {
        exploding[i] = (struct pf_node){PT_BT_SEQUENCE, i + 1, i + 1};
    }
    exploding[7] = (struct pf_node){PT_BT_LEAF, 0, 0};
    C(pf_reactive_init(&b, exploding, 8) == PT_RANGE &&
      !memcmp(&b, &old, sizeof b));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
#define RUN(name)                   \
    if (!strcmp(argv[1], #name))    \
    {                               \
        int r = name();             \
        if (!r)                     \
            puts("PE_PASS:" #name); \
        return r;                   \
    }
    RUN(hsm);
    RUN(active);
    RUN(reactive);
    return 2;
}
