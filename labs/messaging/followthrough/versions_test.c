#include "versions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "SC_CHECK: versions:%d:%s\n", __LINE__, #x); \
            exit(1);                                                     \
        }                                                                \
    } while (0)

struct msg_memory
{
    unsigned calls, allocations, frees, drops, fail;
};

struct msg_payload
{
    struct msg_memory *memory;
    size_t size;
    unsigned char *bytes;
};

struct msg_source
{
    const unsigned char *bytes;
    size_t size;
};

static void *allocate(void *ctx, size_t n)
{
    struct msg_memory *m = ctx;
    if (++m->calls == m->fail)
        return NULL;
    void *p = malloc(n);
    if (p)
        ++m->allocations;
    return p;
}

static void deallocate(void *ctx, void *p)
{
    struct msg_memory *m = ctx;
    ++m->frees;
    free(p);
}

static void drop(void *ptr)
{
    struct msg_payload *p = ptr;
    struct msg_memory *m = p->memory;
    ++m->drops;
    if (p->bytes)
        deallocate(m, p->bytes);
    deallocate(m, p);
}

static struct mfl_allocator allocator(struct msg_memory *m)
{
    return (struct mfl_allocator){m, allocate, deallocate};
}

static enum msg_status clone(void *ctx, const void *ptr, struct mfo_value *out)
{
    struct msg_memory *m = ctx;
    const struct msg_source *s = ptr;
    struct msg_payload *p = allocate(m, sizeof(*p));
    if (!p)
        return MSG_FAILED;
    *p = (struct msg_payload){.memory = m, .size = s->size};
    if (s->size)
    {
        p->bytes = allocate(m, s->size);
        if (!p->bytes)
        {
            deallocate(m, p);
            return MSG_FAILED;
        }
        for (size_t i = 0; i < s->size; i++)
            p->bytes[i] = s->bytes[i];
    }
    *out = (struct mfo_value){p, drop};
    return MSG_OK;
}

static struct mfo_value value(struct msg_memory *m, unsigned n)
{
    unsigned char b = (unsigned char)n;
    struct msg_source source = {&b, 1};
    struct mfo_value v = {0};
    CHECK(clone(m, &source, &v) == MSG_OK);
    return v;
}

static unsigned number(struct mfl_shared *p)
{
    const struct msg_payload *v = mfl_borrow(p)->data;
    CHECK(v->size == 1);
    return v->bytes[0];
}

static enum msg_status publish(struct mfv_store *s, struct msg_memory *m,
                               unsigned n)
{
    unsigned char b = (unsigned char)n;
    struct msg_source source = {&b, 1};
    return mfv_publish(s, clone, m, &source);
}

static unsigned read_pin(struct mfv_store *s, struct msg_snapshot pin)
{
    struct mfl_shared *lease = NULL;
    CHECK(mfv_read(s, pin, &lease) == MSG_OK);
    unsigned n = number(lease);
    mfl_drop(&lease);
    return n;
}

static void owned(void)
{
    for (unsigned cut = 1; cut <= 3; cut++)
    {
        struct msg_memory m = {0};
        struct mfv_store s;
        struct mfo_value initial = value(&m, 4);
        CHECK(mfv_init(&s, allocator(&m), &initial) == MSG_OK && !initial.data);
        struct msg_snapshot pin;
        CHECK(mfv_snapshot(&s, &pin) == MSG_OK);
        unsigned live = m.allocations - m.frees;
        uint64_t clock = s.core.clock;
        m.fail = m.calls + cut;
        CHECK(publish(&s, &m, 9) == MSG_FAILED);
        CHECK(s.core.clock == clock && read_pin(&s, pin) == 4 &&
              m.allocations - m.frees == live);
        m.fail = 0;
        CHECK(publish(&s, &m, 9) == MSG_OK && read_pin(&s, pin) == 4);
        CHECK(mfv_drop_snapshot(&s, pin) == MSG_OK);
        CHECK(mfv_destroy(&s) == MSG_OK && m.allocations == m.frees);
    }
    struct msg_memory m = {0};
    struct mfv_store s;
    struct mfo_value initial = value(&m, 7);
    m.fail = m.calls + 1;
    CHECK(mfv_init(&s, allocator(&m), &initial) == MSG_FAILED && initial.data);
    m.fail = 0;
    CHECK(mfv_init(&s, allocator(&m), &initial) == MSG_OK && !initial.data);
    for (unsigned i = 0; i < 30; i++)
        CHECK(publish(&s, &m, i) == MSG_OK);
    CHECK(mfv_destroy(&s) == MSG_OK && m.drops == 31 &&
          m.allocations == m.frees);
    m = (struct msg_memory){0};
    initial = value(&m, 5);
    CHECK(mfv_init(&s, allocator(&m), &initial) == MSG_OK);
    struct msg_snapshot pins[4];
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(mfv_snapshot(&s, &pins[i]) == MSG_OK);
        CHECK(publish(&s, &m, i + 6) == MSG_OK);
    }
    struct msg_snapshot unchanged = {99, 99};
    CHECK(mfv_snapshot(&s, &unchanged) == MSG_FULL && unchanged.slot == 99 &&
          unchanged.generation == 99);
    CHECK(mfv_destroy(&s) == MSG_BUSY);
    for (unsigned i = 0; i < 4; i++)
        CHECK(read_pin(&s, pins[i]) == i + 5);
    struct mfl_shared *lease = NULL;
    CHECK(mfv_read(&s, pins[0], &lease) == MSG_OK);
    CHECK(mfv_drop_snapshot(&s, pins[0]) == MSG_OK);
    /* Missing retain is rejected before evaluating the pointer after reclamation. */
    CHECK(m.drops == 0);
    struct mfl_shared *sentinel = NULL;
    CHECK(mfv_read(&s, pins[0], &sentinel) == MSG_STALE && !sentinel);
    struct msg_snapshot replacement;
    CHECK(mfv_snapshot(&s, &replacement) == MSG_OK &&
          replacement.generation != pins[0].generation);
    CHECK(mfv_drop_snapshot(&s, pins[0]) == MSG_STALE);
    CHECK(mfv_drop_snapshot(&s, replacement) == MSG_OK);
    for (unsigned i = 1; i < 4; i++)
        CHECK(mfv_drop_snapshot(&s, pins[i]) == MSG_OK);
    s.core.clock = UINT64_MAX;
    unsigned before = m.drops;
    CHECK(publish(&s, &m, 99) == MSG_OVERFLOW && m.drops == before + 1);
    CHECK(mfv_destroy(&s) == MSG_OK);
    CHECK(number(lease) == 5);
    mfl_drop(&lease);
    CHECK(m.drops == 6 && m.allocations == m.frees);
    m = (struct msg_memory){0};
    initial = value(&m, 1);
    CHECK(mfv_init(&s, allocator(&m), &initial) == MSG_OK);
    for (unsigned i = 0; i < 4; i++)
        s.core.pins[i].generation = UINT64_MAX;
    unchanged = (struct msg_snapshot){99, 99};
    CHECK(mfv_snapshot(&s, &unchanged) == MSG_FULL && unchanged.slot == 99);
    CHECK(mfv_destroy(&s) == MSG_OK && m.allocations == m.frees);
    m = (struct msg_memory){0};
    struct msg_source empty = {NULL, 0};
    struct mfo_value v = {0};
    CHECK(clone(&m, &empty, &v) == MSG_OK);
    CHECK(mfv_init(&s, allocator(&m), &v) == MSG_OK);
    CHECK(mfv_snapshot(&s, &pins[0]) == MSG_OK);
    CHECK(mfv_read(&s, pins[0], &lease) == MSG_OK);
    CHECK(((struct msg_payload *)mfl_borrow(lease)->data)->size == 0);
    CHECK(mfv_drop_snapshot(&s, pins[0]) == MSG_OK);
    CHECK(mfv_destroy(&s) == MSG_OK);
    mfl_drop(&lease);
    CHECK(m.allocations == m.frees);
}

struct msg_gate
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    unsigned ready;
    bool go, strict;
    struct mfk_store *store;
    enum msg_status results[2];
};

struct msg_worker
{
    struct msg_gate *gate;
    unsigned key;
};

static void *skew(void *ctx)
{
    struct msg_worker *w = ctx;
    struct msg_gate *g = w->gate;
    struct mfk_txn t;
    unsigned count;
    CHECK(mfk_begin(g->store, &t) == MSG_OK);
    CHECK(mfk_nonzero(&t, &count) == MSG_OK && count == 2);
    CHECK(mfk_write(&t, w->key, 0) == MSG_OK);
    pthread_mutex_lock(&g->mutex);
    ++g->ready;
    pthread_cond_broadcast(&g->condition);
    while (!g->go)
        pthread_cond_wait(&g->condition, &g->mutex);
    pthread_mutex_unlock(&g->mutex);
    g->results[w->key] = mfk_commit(&t, g->strict);
    return NULL;
}

static void predicate(void)
{
    for (unsigned strict = 0; strict < 2; strict++)
    {
        struct mfk_store s;
        CHECK(mfk_init(&s, 1, 1) == MSG_OK);
        struct msg_gate g = {.store = &s, .strict = strict != 0};
        CHECK(!pthread_mutex_init(&g.mutex, NULL));
        CHECK(!pthread_cond_init(&g.condition, NULL));
        struct msg_worker workers[2] = {{&g, 0}, {&g, 1}};
        pthread_t threads[2];
        for (unsigned i = 0; i < 2; i++)
            CHECK(!pthread_create(&threads[i], NULL, skew, &workers[i]));
        pthread_mutex_lock(&g.mutex);
        while (g.ready != 2)
            pthread_cond_wait(&g.condition, &g.mutex);
        g.go = true;
        pthread_cond_broadcast(&g.condition);
        pthread_mutex_unlock(&g.mutex);
        for (unsigned i = 0; i < 2; i++)
            CHECK(!pthread_join(threads[i], NULL));
        if (strict)
            CHECK((g.results[0] == MSG_OK && g.results[1] == MSG_CONFLICT) ||
                  (g.results[1] == MSG_OK && g.results[0] == MSG_CONFLICT));
        else
            CHECK(g.results[0] == MSG_OK && g.results[1] == MSG_OK);
        struct mfk_txn t;
        unsigned count;
        CHECK(mfk_begin(&s, &t) == MSG_OK);
        CHECK(mfk_nonzero(&t, &count) == MSG_OK && count == strict);
        CHECK(mfk_commit(&t, true) == MSG_OK);
        CHECK(!pthread_cond_destroy(&g.condition));
        CHECK(!pthread_mutex_destroy(&g.mutex));
        mfk_destroy(&s);
    }
    struct mfk_store s;
    CHECK(mfk_init(&s, 0, 0) == MSG_OK);
    struct mfk_txn a, b;
    unsigned n = 99;
    CHECK(mfk_begin(&s, &a) == MSG_OK);
    CHECK(mfk_begin(&s, &b) == MSG_OK);
    CHECK(mfk_nonzero(&a, &n) == MSG_OK && n == 0);
    CHECK(mfk_write(&a, 0, 1) == MSG_OK);
    CHECK(mfk_write(&b, 1, 1) == MSG_OK);
    CHECK(mfk_commit(&b, false) == MSG_OK);
    CHECK(mfk_commit(&a, true) == MSG_CONFLICT);
    CHECK(mfk_begin(&s, &a) == MSG_OK);
    CHECK(mfk_begin(&s, &b) == MSG_OK);
    CHECK(mfk_write(&a, 0, 4) == MSG_OK);
    CHECK(mfk_write(&b, 0, 8) == MSG_OK);
    CHECK(mfk_commit(&a, false) == MSG_OK);
    CHECK(mfk_commit(&b, false) == MSG_CONFLICT);
    CHECK(mfk_begin(&s, &a) == MSG_OK);
    CHECK(mfk_write(&a, 1, 7) == MSG_OK);
    CHECK(mfk_read(&a, 1, &n) == MSG_OK && n == 7);
    CHECK(mfk_commit(&a, true) == MSG_OK);
    CHECK(mfk_read(&a, 0, &n) == MSG_INVALID && n == 7);
    CHECK(mfk_commit(&a, true) == MSG_INVALID);
    CHECK(mfk_begin(&s, &a) == MSG_OK);
    CHECK(mfk_begin(&s, &b) == MSG_OK);
    CHECK(mfk_read(&a, 0, &n) == MSG_OK);
    CHECK(mfk_write(&b, 1, 3) == MSG_OK);
    CHECK(mfk_commit(&b, true) == MSG_OK);
    CHECK(mfk_commit(&a, true) == MSG_OK);
    atomic_store(&s.core.clock, UINT_MAX / 2);
    CHECK(mfk_begin(&s, &a) == MSG_OK);
    CHECK(mfk_write(&a, 0, 99) == MSG_OK);
    CHECK(mfk_commit(&a, true) == MSG_OVERFLOW);
    CHECK(atomic_load(&s.core.cells[0].value) == 4 &&
          !(atomic_load(&s.core.cells[0].stamp) & 1u));
    mfk_destroy(&s);
}

static void predicate_oracle(void)
{
    unsigned x, y, k0, k1, v0, v1, strict, order;
    while (scanf("%u %u %u %u %u %u %u %u", &x, &y, &k0, &k1, &v0, &v1, &strict,
                 &order) == 8)
    {
        struct mfk_store s;
        CHECK(mfk_init(&s, x, y) == MSG_OK);
        struct mfk_txn t[2];
        unsigned count;
        for (unsigned i = 0; i < 2; i++)
        {
            CHECK(mfk_begin(&s, &t[i]) == MSG_OK);
            CHECK(mfk_nonzero(&t[i], &count) == MSG_OK);
        }
        if (k0 < 2)
            CHECK(mfk_write(&t[0], k0, v0) == MSG_OK);
        if (k1 < 2)
            CHECK(mfk_write(&t[1], k1, v1) == MSG_OK);
        enum msg_status statuses[2];
        statuses[order] = mfk_commit(&t[order], strict != 0);
        statuses[1 - order] = mfk_commit(&t[1 - order], strict != 0);
        printf("%d %d %u %u\n", statuses[0], statuses[1],
               atomic_load(&s.core.cells[0].value),
               atomic_load(&s.core.cells[1].value));
        mfk_destroy(&s);
    }
}

static void owned_oracle(void)
{
    char line[128];
    while (fgets(line, sizeof(line), stdin))
    {
        struct msg_memory m = {0};
        struct mfv_store s;
        struct mfo_value v = value(&m, 0);
        CHECK(mfv_init(&s, allocator(&m), &v) == MSG_OK);
        struct msg_snapshot pins[4];
        unsigned count = 0, serial = 0;
        for (size_t i = 0; line[i] && line[i] != '\n'; i++)
        {
            enum msg_status status;
            unsigned n = 99;
            char op = line[i];
            if (op == 'p' || op == 'f')
            {
                ++serial;
                if (op == 'f')
                    m.fail = m.calls + 2;
                status = publish(&s, &m, serial);
                m.fail = 0;
            }
            else if (op == 's')
            {
                struct msg_snapshot pin;
                status = mfv_snapshot(&s, &pin);
                if (status == MSG_OK)
                    pins[count++] = pin;
            }
            else if (op == 'r')
            {
                struct mfl_shared *lease = NULL;
                status = mfv_read(
                    &s, count ? pins[0] : (struct msg_snapshot){9, 9}, &lease);
                if (status == MSG_OK)
                {
                    n = number(lease);
                    mfl_drop(&lease);
                }
            }
            else
            {
                CHECK(op == 'd');
                status = mfv_drop_snapshot(
                    &s, count ? pins[0] : (struct msg_snapshot){9, 9});
                if (status == MSG_OK)
                {
                    --count;
                    for (unsigned j = 0; j < count; j++)
                        pins[j] = pins[j + 1];
                }
            }
            printf("%d:%u:%llu,", status, n, (unsigned long long)s.core.clock);
        }
        for (unsigned i = 0; i < count; i++)
            CHECK(mfv_drop_snapshot(&s, pins[i]) == MSG_OK);
        CHECK(mfv_destroy(&s) == MSG_OK && m.allocations == m.frees);
        puts("");
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "owned"))
        owned();
    else if (!strcmp(argv[1], "predicate"))
        predicate();
    else if (!strcmp(argv[1], "predicate-oracle"))
    {
        predicate_oracle();
        return 0;
    }
    else if (!strcmp(argv[1], "owned-oracle"))
    {
        owned_oracle();
        return 0;
    }
    else
        CHECK(false);
    puts("versions passed");
    return 0;
}
